/*
 * nvram.c - Non-Volatile RAM access
 *
 * Copyright (C) 2001-2020 The EmuTOS development team
 *
 * Authors:
 *  LVL     Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

/* #define ENABLE_KDEBUG */

#include "emutos.h"
#include "cookie.h"
#include "machine.h"
#include "vectors.h"
#include "nvram.h"
#include "biosmem.h"
#include "bdosbind.h"
#include "string.h"

#if CONF_WITH_NVRAM

#define NVRAM_ADDR_REG  0xffff8961L
#define NVRAM_DATA_REG  0xffff8963L

#define NVRAM_RTC_SIZE  14          /* first 14 registers are RTC */
#define NVRAM_START     NVRAM_RTC_SIZE
#define NVRAM_SIZE      50          /* remaining 50 are RAM */
#define NVRAM_USER_SIZE 48          /* of which the user may access 48 */
#define NVRAM_CKSUM     NVRAM_USER_SIZE /* and the last 2 are checksum */

/*
 * on the TT, resetting NVRAM causes TOS3 to zero it.
 * on the Falcon and FireBee, it is set to zeroes except for a small
 * portion (see below) starting at NVRAM_INIT_START.
 *
 * we do the same to avoid problems booting Atari TOS and EmuTOS on the
 * same system.
 */
#define NVRAM_INIT_START    8
#if CONF_WITH_APOLLO_V4
const UBYTE nvram_init[] = { 0x00, 0x2f, 0x20, 0xff, 0xff, 0xff, 0x01, 0x0a };
#else
const UBYTE nvram_init[] = { 0x00, 0x2f, 0x20, 0xff, 0xff, 0xff };
#endif

int has_nvram;

#if CONF_WITH_APOLLO_V4

static UBYTE nvram_buf[NVRAM_SIZE];
static BOOL nvram_loaded;

static const char nvram_path_1[] = "C:\\GEMSYS\\NVRAM.BIN";
static const char nvram_path_2[] = "C:\\NVRAM.BIN";

void detect_nvram(void)
{
    has_nvram = 1;
}

UBYTE get_nvram_rtc(int index)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;
    int ret_value = 0;

    if (has_nvram)
    {
        if ((index >= 0) && (index < NVRAM_RTC_SIZE))
        {
            *addr_reg = index;
            ret_value = *data_reg;
        }
    }

    return ret_value;
}

void set_nvram_rtc(int index, int data)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;

    if (has_nvram)
    {
        if ((index >= 0) && (index < NVRAM_RTC_SIZE))
        {
            *addr_reg = index;
            *data_reg = data;
        }
    }
}

static UWORD compute_sum(void)
{
    UBYTE sum;
    int i;

    for (i = 0, sum = 0; i < NVRAM_USER_SIZE; i++)
    {
        sum += nvram_buf[i];
    }

    return MAKE_UWORD(~sum, sum);
}

static UWORD get_sum(void)
{
    return MAKE_UWORD(nvram_buf[NVRAM_CKSUM], nvram_buf[NVRAM_CKSUM + 1]);
}

static void set_sum(UWORD sum)
{
    nvram_buf[NVRAM_CKSUM] = HIBYTE(sum);
    nvram_buf[NVRAM_CKSUM + 1] = LOBYTE(sum);
}

static void write_nvram_file(void)
{
    LONG fh;

    fh = Fopen(nvram_path_1, 1);
    if (fh < 0)
        fh = Fopen(nvram_path_2, 1);
    if (fh < 0)
        fh = Fcreate(nvram_path_1, 0);
    if (fh < 0)
        fh = Fcreate(nvram_path_2, 0);

    if (fh >= 0)
    {
        Fwrite((WORD)fh, NVRAM_SIZE, nvram_buf);
        Fclose((WORD)fh);
    }
}

static void load_or_init_nvram(void)
{
    LONG fh;
    LONG nread;

    if (nvram_loaded)
        return;

    fh = Fopen(nvram_path_1, 0);
    if (fh < 0)
        fh = Fopen(nvram_path_2, 0);

    if (fh >= 0)
    {
        nread = Fread((WORD)fh, NVRAM_SIZE, nvram_buf);
        Fclose((WORD)fh);

        if ((nread == NVRAM_SIZE) && (compute_sum() == get_sum()))
        {
            nvram_loaded = TRUE;
            return;
        }
    }

    memset(nvram_buf, 0, NVRAM_USER_SIZE);
    memcpy(&nvram_buf[NVRAM_INIT_START], nvram_init, ARRAY_SIZE(nvram_init));
    set_sum(compute_sum());
    nvram_loaded = TRUE;
    write_nvram_file();
}

WORD nvmaccess(WORD type, WORD start, WORD count, UBYTE *buffer)
{
    int i;

    if (!has_nvram)
        return 0x2E;

    load_or_init_nvram();

    if (type == 2)      /* reset all */
    {
        memset(nvram_buf, 0, NVRAM_USER_SIZE);
        if (cookie_mch == MCH_TT)
        {
            set_sum(compute_sum());
            write_nvram_file();
        }
        else
        {
            nvmaccess(1, NVRAM_INIT_START, ARRAY_SIZE(nvram_init), CONST_CAST(UBYTE *, nvram_init));
        }
        return 0;
    }

    if ((buffer == NULL) || (start < 0) || (count < 1) || ((start + count) > NVRAM_USER_SIZE))
        return -5;

    switch(type) {
    case 0:         /* read */
        {
            UWORD expected = compute_sum();
            UWORD actual = get_sum();

            if (expected != actual)
            {
                KDEBUG(("wrong nvram: expected=0x%04x actual=0x%04x\n", expected, actual));
                return -12;
            }
            for (i = start; i < start + count; i++)
            {
                *buffer++ = nvram_buf[i];
            }
        }
        break;
    case 1:         /* write */
        for (i = start; i < start + count; i++)
        {
            nvram_buf[i] = *buffer++;
        }
        set_sum(compute_sum());
        write_nvram_file();
        break;
    default:
        return -5;
    }

    return 0;
}

#else /* !CONF_WITH_APOLLO_V4 */

/*
 * detect_nvram - detect the NVRAM
 */
void detect_nvram(void)
{
    if (check_read_byte(NVRAM_ADDR_REG))
        has_nvram = 1;
    else has_nvram = 0;
}

/*
 * get_nvram_rtc - read the realtime clock from NVRAM
 */
UBYTE get_nvram_rtc(int index)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;
    int ret_value = 0;

    if (has_nvram)
    {
        if ((index >= 0) && (index < NVRAM_RTC_SIZE))
        {
            *addr_reg = index;
            ret_value = *data_reg;
        }
    }

    return ret_value;
}

/*
 * set_nvram_rtc - set the realtime clock in NVRAM
 */
void set_nvram_rtc(int index, int data)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;

    if (has_nvram)
    {
        if ((index >= 0) && (index < NVRAM_RTC_SIZE))
        {
            *addr_reg = index;
            *data_reg = data;
        }
    }
}

/*
 * compute_sum - internal checksum handling
 */
static UWORD compute_sum(void)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;
    UBYTE sum;
    int i;

    for (i = 0, sum = 0; i < NVRAM_USER_SIZE; i++)
    {
        *addr_reg = i + NVRAM_START;
        sum += *data_reg;
    }

    return MAKE_UWORD(~sum, sum);
}

/*
 * get_sum - read checksum from NVRAM
 */
static UWORD get_sum(void)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;
    UWORD sum;

    *addr_reg = NVRAM_START + NVRAM_CKSUM;
    sum = *data_reg << 8;
    *addr_reg = NVRAM_START + NVRAM_CKSUM + 1;
    sum |= *data_reg;

    return sum;
}

/*
 * set_sum - write checksum to NVRAM
 */
static void set_sum(UWORD sum)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;

    *addr_reg = NVRAM_START + NVRAM_CKSUM;
    *data_reg = HIBYTE(sum);
    *addr_reg = NVRAM_START + NVRAM_CKSUM + 1;
    *data_reg = LOBYTE(sum);
}

/*
 * nvmaccess - XBIOS read/write/reset NVRAM
 *
 * Arguments:
 *
 *   type   - 0:read, 1:write, 2:reset
 *   start  - start address for operation
 *   count  - count of bytes
 *   buffer - buffer for operations
 */
WORD nvmaccess(WORD type, WORD start, WORD count, UBYTE *buffer)
{
    volatile UBYTE *addr_reg = (volatile UBYTE *)NVRAM_ADDR_REG;
    volatile UBYTE *data_reg = (volatile UBYTE *)NVRAM_DATA_REG;
    int i;

    if (!has_nvram)
        return 0x2E;

    if (type == 2)      /* reset all */
    {
        for (i = 0; i < NVRAM_USER_SIZE; i++)
        {
            *addr_reg = i + NVRAM_START;
            *data_reg = 0;
        }
        if (cookie_mch == MCH_TT)
            set_sum(compute_sum());
        else
            nvmaccess(1,NVRAM_INIT_START,ARRAY_SIZE(nvram_init),CONST_CAST(UBYTE *,nvram_init));
        return 0;
    }

    if ((buffer == NULL) || (start < 0) || (count < 1) || ((start + count) > NVRAM_USER_SIZE))
        return -5;

    switch(type) {
    case 0:         /* read */
        {
            UWORD expected = compute_sum();
            UWORD actual = get_sum();

            if (expected != actual)
            {
                KDEBUG(("wrong nvram: expected=0x%04x actual=0x%04x\n", expected, actual));
                /* wrong checksum, return error code */
                return -12;
            }
            for (i = start; i < start + count; i++)
            {
                *addr_reg = i + NVRAM_START;
                *buffer++ = *data_reg;
            }
        }
        break;
    case 1:         /* write */
        for (i = start; i < start + count; i++)
        {
            *addr_reg = i + NVRAM_START;
            *data_reg = *buffer++;
        }
        set_sum(compute_sum());
        break;
    default:
        /* wrong operation code! */
        return -5;
    }

    return 0;
}

#endif /* CONF_WITH_APOLLO_V4 */

#endif  /* CONF_WITH_NVRAM */
