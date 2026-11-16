/*
 * spi_vamp.c - SPI interface for SD/MMC card driver on SAGA core
 *
 * Copyright (C) 2019 The EmuTOS development team
 *
 * Authors:
 *  Christian Zietz
 *  RFB   Roger Burrows
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "emutos.h"
#include "spi.h"

#if CONF_WITH_VAMPIRE_SPI

#if CONF_WITH_APOLLO_V4
#define SAGA_SDCARD_BASE 0x00FFFE00UL
#else
#define SAGA_SDCARD_BASE 0x00DE0000UL
#endif

#define SAGA_SDCARD_DATA (*(volatile UBYTE*)(SAGA_SDCARD_BASE + 0x00UL))
#define SAGA_SDCARD_READ (*(volatile UBYTE*)(SAGA_SDCARD_BASE + 0x02UL))
#define SAGA_SDCARD_CTL  (*(volatile UWORD*)(SAGA_SDCARD_BASE + 0x04UL))
#define SAGA_SDCARD_STAT (*(volatile UWORD*)(SAGA_SDCARD_BASE + 0x06UL))
#define SAGA_SDCARD_CLK  (*(volatile UWORD*)(SAGA_SDCARD_BASE + 0x0CUL))

#define SAGA_SDCARD_CLKDIV_IDENT ((UWORD)0xffU) /* 195 kHz */
#define SAGA_SDCARD_CLKDIV_MMC   ((UWORD)0x02U) /* 16.7 MHz */
#define SAGA_SDCARD_CLKDIV_SD    ((UWORD)0x01U) /* 25 MHz */

/*
 * Set when SAGA_SDCARD_READ (reg 02, $00FFFE02 / $00DE0002) is wired in the
 * FPGA core.  Reading reg 02 is pipelined (1-byte lag): it stalls if a shift
 * is busy, returns the previously shifted byte from rx_reg, and triggers the
 * next 0xFF 8-bit shift in the background without requiring a write cycle.
 * Older V4 Atari cores return 0x00 on reg 02 and fall back to reg 00.
 */
static BOOL has_saga_read_reg;

static void spi_probe_read_reg(void)
{
    SAGA_SDCARD_DATA = 0xFF;
    if (SAGA_SDCARD_DATA == 0xFF) {
        UBYTE probe = SAGA_SDCARD_READ;
        FORCE_READ(SAGA_SDCARD_DATA);
        has_saga_read_reg = (probe == 0xFF);
    } else {
        has_saga_read_reg = FALSE;
    }
}

/*
 *  initialise spi for memory card
 */
void spi_initialise(void)
{
    has_saga_read_reg = FALSE;
    spi_cs_unassert();
}

void spi_clock_sd(void)
{
    SAGA_SDCARD_CLK = SAGA_SDCARD_CLKDIV_SD;
    spi_probe_read_reg();
}

void spi_clock_mmc(void)
{
    SAGA_SDCARD_CLK = SAGA_SDCARD_CLKDIV_MMC;
    spi_probe_read_reg();
}

void spi_clock_ident(void)
{
    SAGA_SDCARD_CLK = SAGA_SDCARD_CLKDIV_IDENT;
}

/* when we assert or unassert, we send a dummy byte to
 * force a write to the register
 */
void spi_cs_assert(void)
{
    SAGA_SDCARD_CTL = 0;
    spi_send_byte(0xff);
}

void spi_cs_unassert(void)
{
    SAGA_SDCARD_CTL = 1;
    spi_send_byte(0xff);
}

void spi_send_byte(UBYTE c)
{
    SAGA_SDCARD_DATA = c;
    /* reading will stall until transmission is complete */
    FORCE_READ(SAGA_SDCARD_DATA);
}

UBYTE spi_recv_byte(void)
{
    /* send a byte to get one */
    SAGA_SDCARD_DATA = 0xFF;
    /* reading will stall until transmission is complete */
    return SAGA_SDCARD_DATA;
}

void spi_recv_block(UBYTE *buf, UWORD len)
{
    volatile UBYTE * const data_reg = &SAGA_SDCARD_DATA;
    UWORD n, rem;

    if (len == 0)
        return;

    if (has_saga_read_reg) {
        volatile UBYTE * const read_reg = &SAGA_SDCARD_READ;

        /* Prime pipelined reg 02: discards previous byte, clocks data[0] */
        FORCE_READ(*read_reg);

        rem = len - 1;
        n = rem >> 3;
        rem &= 7;

        if (n > 0) {
            UWORD cnt = n - 1;
            __asm__ volatile (
                "1:\n\t"
                "move.b (%2),(%0)+\n\t"
                "move.b (%2),(%0)+\n\t"
                "move.b (%2),(%0)+\n\t"
                "move.b (%2),(%0)+\n\t"
                "move.b (%2),(%0)+\n\t"
                "move.b (%2),(%0)+\n\t"
                "move.b (%2),(%0)+\n\t"
                "move.b (%2),(%0)+\n\t"
                "dbra %1,1b"
                : "+a"(buf), "+d"(cnt)
                : "a"(read_reg)
                : "cc", "memory"
            );
        }
        while (rem--)
            *buf++ = *read_reg;

        /* Drain final byte data[len-1] via reg 00 without clocking an extra byte */
        *buf = *data_reg;
        return;
    }

    /* Fallback for older cores without reg 02: tight reg 00 write+read loop */
    n = len >> 3;
    rem = len & 7;

    if (n > 0) {
        UWORD cnt = n - 1;
        UBYTE ff = 0xFF;
        __asm__ volatile (
            "1:\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "move.b %3,(%2)\n\t" "move.b (%2),(%0)+\n\t"
            "dbra %1,1b"
            : "+a"(buf), "+d"(cnt)
            : "a"(data_reg), "d"(ff)
            : "cc", "memory"
        );
    }
    while (rem--) {
        *data_reg = 0xFF;
        *buf++ = *data_reg;
    }
}

void spi_send_block(const UBYTE *buf, UWORD len)
{
    volatile UBYTE * const data_reg = &SAGA_SDCARD_DATA;
    UWORD n = len >> 3;
    UWORD rem = len & 7;

    if (n > 0) {
        UWORD cnt = n - 1;
        UBYTE dummy;
        __asm__ volatile (
            "1:\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "move.b (%0)+,(%3)\n\t" "move.b (%3),%2\n\t"
            "dbra %1,1b"
            : "+a"(buf), "+d"(cnt), "=&d"(dummy)
            : "a"(data_reg)
            : "cc", "memory"
        );
    }
    while (rem--) {
        *data_reg = *buf++;
        FORCE_READ(*data_reg);
    }
}
#endif /* CONF_WITH_VAMPIRE_SPI */
