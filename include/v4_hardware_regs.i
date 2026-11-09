; ============================================================================
; Apollo V4 Standalone (Atari Falcon Core) Hardware Register Definitions
; Reconstructed from ETOS512.IMG (1.4.0.1-V4) & V4SA Drivers
; Assembler: vasm (vasmm68k_mot)
; ============================================================================

    IFND V4_HARDWARE_REGS_I
V4_HARDWARE_REGS_I SET 1

; ----------------------------------------------------------------------------
; 1. V4 ROM Shadow RAM Write-Protection Register ($00E00000 - $00E7FFFF)
; ----------------------------------------------------------------------------
V4_ROM_LOCK_REG         EQU $00FFFEFE   ; Write $B00B to unlock $E00000-$E7FFFF for writing
V4_ROM_UNLOCK_KEY       EQU $B00B       ; Magic key to enable RAM writes to ROM space

; ----------------------------------------------------------------------------
; 2. Vampire MicroSD SPI Controller (Atari I/O Window $00FFFE00 - $00FFFE0F)
; ----------------------------------------------------------------------------
V4_SPI_DATA             EQU $00FFFE00   ; (B) Read/Write SPI byte transfer
V4_SPI_CS               EQU $00FFFE04   ; (W) Chip Select: $0000 = Active (Low), $0001 = Inactive (High)
V4_SPI_DIVIDER          EQU $00FFFE0C   ; (W) SPI Clock Divider ($0001=Fast, $0002=Med, $00FF=Slow init)

; ----------------------------------------------------------------------------
; 3. SAGA Video Controller (Atari I/O Mirror $00FFFED0 - $00FFFEF4)
;    Mirrors Amiga SAGA registers $00DFF1D0 - $00DFF1F4
; ----------------------------------------------------------------------------
V4_SAGA_DAC_CTRL0       EQU $00FFFED0   ; (W) SAGA DAC / Border control 0 (init to $3FFF)
V4_SAGA_DAC_CTRL1       EQU $00FFFED2   ; (W) SAGA DAC / Border control 1 (init to $3FFF)
V4_SAGA_MODULO          EQU $00FFFEE6   ; (W) SAGA Screen Modulo / Pitch in bytes (mirror of $DFF1E6)
V4_SAGA_PLANE0          EQU $00FFFEEC   ; (L) SAGA 32-bit Chunky Framebuffer Address (mirror of $DFF1EC)
V4_SAGA_VIDMODE         EQU $00FFFEF4   ; (W) SAGA Video Mode Register (mirror of $DFF1F4)
                                        ;     High byte = Resolution ID ($01..$14)
                                        ;     Low byte  = Pixel Format ($01=CLUT8, $02=RGB16, $04=RGB24,
                                        ;                 $05=RGB32, $08=1bpp, $09=2bpp, $0A=4bpp, $0B=8bpp)

; Direct SAGA mode flag in Atari VsetMode(mode):
V4_VSETMODE_SAGA_FLAG   EQU $4000       ; Bit 14: pass (mode & $3FFF) directly to V4_SAGA_VIDMODE

; Standard Atari Falcon / ST Video Registers still updated for compatibility:
ATARI_VID_BASE_HI       EQU $FFFF8201   ; (B) Video base high byte (when addr <= $00FFFFFF)
ATARI_VID_BASE_MID      EQU $FFFF8203   ; (B) Video base mid byte
ATARI_VID_SYNCMODE      EQU $FFFF820A   ; (B) Video sync mode
ATARI_VID_BASE_LO       EQU $FFFF820D   ; (B) Video base low byte
ATARI_VID_LINEWID       EQU $FFFF820F   ; (B) Video extra line width
ATARI_ST_PALETTE        EQU $FFFF8240   ; (W) 16-word ST/STE color palette ($FFFF8240-$FFFF825E)
ATARI_ST_SHIFTMODE      EQU $FFFF8260   ; (B) ST shift mode (0=4bpp, 1=2bpp, 2=1bpp)
ATARI_FALCON_PALETTE    EQU $FFFF9800   ; (L) 256-long Falcon Videl palette

; ----------------------------------------------------------------------------
; 4. SAGA Picture-in-Picture (PiP) Hardware Overlay ($00DFF38C, $00DFF3D0..E2)
;    Controlled via "V4PP" ($56345050) descriptor API at ROM $E0CCA6
; ----------------------------------------------------------------------------
V4_PIP_CLUT_PORT        EQU $00DFF38C   ; (L) Sequential write port for 256-entry PiP CLUT (1024 bytes)
V4_PIP_X_START          EQU $00DFF3D0   ; (W) PiP X Start (x + 17)
V4_PIP_Y_START          EQU $00DFF3D2   ; (W) PiP Y Start (y - 1)
V4_PIP_X_STOP           EQU $00DFF3D4   ; (W) PiP X Stop  (x + 17 + width)
V4_PIP_Y_STOP           EQU $00DFF3D6   ; (W) PiP Y Stop  (y - 1 + height)
V4_PIP_BPLPTR           EQU $00DFF3D8   ; (L) PiP Framebuffer Pointer (0 = disable)
V4_PIP_FORMAT           EQU $00DFF3DC   ; (W) PiP Pixel Format
V4_PIP_OFFSET           EQU $00DFF3DE   ; (W) PiP Horizontal bit/byte offset
V4_PIP_COLORKEY         EQU $00DFF3E0   ; (W) PiP Color-Key control
V4_PIP_MODULO           EQU $00DFF3E2   ; (W) PiP Row Modulo

V4_PIP_MAGIC_HI         EQU $5634       ; 'V4'
V4_PIP_MAGIC_LO         EQU $5050       ; 'PP'

; ----------------------------------------------------------------------------
; 5. SAGA Hardware Mouse Sprite ($00DFF1D0..D8, $00DFF3A0..BE, $00DFF800)
; ----------------------------------------------------------------------------
V4_SPRITE_X             EQU $00DFF1D0   ; (W) Hardware Mouse Cursor X Position
V4_SPRITE_Y             EQU $00DFF1D2   ; (W) Hardware Mouse Cursor Y Position
V4_SPRITE_COL1          EQU $00DFF3A2   ; (W) Sprite Color 1 (RGB16)
V4_SPRITE_COL2          EQU $00DFF3A4   ; (W) Sprite Color 2 (RGB16)
V4_SPRITE_COL3          EQU $00DFF3A6   ; (W) Sprite Color 3 (RGB16)
V4_SPRITE_BITMAP        EQU $00DFF800   ; (L) 16x16 Planar Sprite Bitmap Buffer

; ----------------------------------------------------------------------------
; 6. Fast IDE Interface
; ----------------------------------------------------------------------------
V4_IDE_DATA             EQU $00F00000   ; Base of Atari Falcon IDE registers
V4_IDE_FAST_WAITSTATES  EQU $00DD1020   ; Vampire Fast IDE timing/waitstate register

    ENDC ; V4_HARDWARE_REGS_I
