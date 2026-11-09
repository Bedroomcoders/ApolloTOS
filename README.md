# ApolloTOS

**ApolloTOS** is a high-performance TOS/GEM-compatible operating system for the **Apollo V4 Standalone (V4SA)** running the Atari Falcon FPGA core.

It is forked from [EmuTOS](https://github.com/emutos/emutos) to provide first-class, dedicated hardware support for the **Apollo AC68080** processor, **AMMX** vector extensions, **SAGA** video controller, high-speed **Vampire SPI MicroSD**, and **Fast IDE** interfaces.

---

## Key Features & Hardware Support

- **Processor Support**:
  - Full support for the **Apollo AC68080** (68040/68060 instruction set with 64-bit AMMX vector extensions).
  - Integrated `vasm` (`vasmm68k_mot`) in the build system for writing native 68080 and AMMX assembly routines (`.vasm`).
- **SAGA Video Controller**:
  - Direct SAGA video registers (`$00FFFEE6` modulo, `$00FFFEF4` video mode, `$00FFFEEC` 32-bit chunky framebuffer pointer).
  - True 32-bit framebuffer routing to high FastRAM without bus errors.
  - SAGA video modes: 320x200, 320x240, 320x256, 640x400, 640x480, 800x600, 1024x768, 1280x720, 1920x1080, in 1/2/4/8-bit planar and 16/24/32-bit chunky formats.
  - Full STe Shifter and Falcon 256-color hardware palette compatibility.
  - SAGA hardware sprite cursor initialization.
- **Memory Subsystem**:
  - Hardware mirror-probe detection of physical FastRAM (512 MB, 1024 MB, or 2048 MB).
  - Reserved 19.3 MB SAGA VRAM aperture at the top of physical RAM.
- **Storage Subsystem**:
  - Native **Falcon/Gayle Fast IDE** controller (`$00F00000`) for CompactFlash storage.
  - Built-in **Vampire SPI MicroSD** driver (`$00FFFE00` - `$00FFFE0F`) supporting direct boot from SD-card and hot partition access without external drivers.
- **File-Backed NVRAM**:
  - File-backed NVRAM system saving boot configuration, keyboard layout, and display settings directly to `C:\NVRAM.BIN` / `C:\GEMSYS\NVRAM.BIN`.

---

## Building ApolloTOS

### Prerequisites

1. **m68k-atari-mint Toolchain**:
   - `m68k-atari-mint-gcc` (GCC 13.x or 4.6.4 cross-compiler for m68k)
   - `m68k-atari-mint-binutils` (`as`, `ld`, `objdump`)
2. **vasm Assembler**:
   - `vasmm68k_mot` (Motorola-syntax m68k/68080 assembler) in your system `PATH`.
3. **Build Environment**:
   - MSYS2 (Windows) or any modern Linux/Unix environment with GNU `make`, `bash`, and a host C compiler (`gcc`).

### Build Instructions

To build the 512 KB ROM image:

```bash
# Build ApolloTOS for Apollo V4 (US English)
make apollotos

# Or alternatively:
make v4atari
```

To build for a specific keyboard/country (e.g. French `fr`, German `de`, Swedish `se`):

```bash
make apollotos COUNTRY=de
```

### Build Artifact

The build produces:
- **`etos512us.img`** (or `etos512<country>.img`): 512 KB binary ROM image ready to be softloaded or flashed.

---

## Running / Testing on Apollo V4

### Method 1: Soft-loading with MapTOS
Place `etos512us.img` onto your CompactFlash or MicroSD card:
```text
C:\V4SA\ROM\ETOS512.IMG
```
Execute `MAPTOS.TOS` from your `AUTO` folder or desktop to soft-load the ROM into the V4 shadow RAM at `$00E00000`.

### Method 2: Booting from MicroSD or CompactFlash
- If only the MicroSD card is inserted, ApolloTOS automatically assigns it as drive **`C:`** and boots directly from the SD card.
- If both CompactFlash and MicroSD cards are inserted:
  - CompactFlash is assigned to drive **`C:`** (boot).
  - MicroSD card is assigned to drive **`D:`** (data).
  - To boot from the SD card instead, hold **`Shift`** at the ApolloTOS welcome screen and press **`D`**.

---

## License

ApolloTOS is licensed under the **GNU General Public License, version 2** (GPLv2). See `doc/license.txt` for full details.

### Acknowledgements
ApolloTOS is built upon the incredible work of the **EmuTOS Development Team**, Digital Research, and the **Apollo Computer / Vampire Team**.
