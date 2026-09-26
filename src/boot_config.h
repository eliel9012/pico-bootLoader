/*
 * boot_config.h - Flash memory map for the bootLoader.
 *
 * The same numbers live in pico_shared/BootPartition.cmake (which links the
 * bootloader into the boot region and the emulators into the app partition).
 * CMake passes BOOTLOADER_SIZE / FLASH_TOTAL_SIZE / TV_RESERVED_SIZE here as
 * compile definitions so the C code below agrees with the linker. The
 * #ifndef fallbacks keep this header usable from host unit tests too.
 *
 * Flash layout (Adafruit Fruit Jam, 16 MB, TV_RESERVED_SIZE=4 MB):
 *
 *   0x10000000  +-----------------------------+  <- bootrom always boots this
 *               |   Bootloader (bootLoader)   |     image (the menu/flasher).
 *               |        512 KB               |
 *   0x10080000  +-----------------------------+  <- APP_BASE_ADDR
 *               |   Application partition      |     emulator UF2s land here.
 *               |        11.5 MB              |     the bootloader jumps here.
 *   0x10C00000  +-----------------------------+  <- TV_BASE_ADDR
 *               |   Fruit Jam retro-TV        |     the fruitjam-retro-tv
 *               |        4 MB                 |     firmware; never erased by
 *               |                             |     the emulator flash path.
 *   0x11000000  +-----------------------------+  <- TV_END_ADDR
 *
 * Because the bootloader sits at the very start of flash, the RP2350 bootrom
 * always runs it first. Every hardware reset / power cycle lands here, and
 * (see main.cpp's resume check) a cold boot then jumps straight into the TV
 * region -- the emulator picker is one options-menu entry away, not the
 * default. An emulator or the TV only runs when the bootloader jumps to it.
 *
 * TV_RESERVED_SIZE defaults to 0 (no change from upstream: the whole app
 * partition, all the way to end of flash) so every non-Fruit-Jam board keeps
 * its original partition size unless it opts in. The Fruit Jam build (this
 * fork's CMakeLists.txt, HW_CONFIG==8) sets it to 4 MB.
 */
#ifndef BOOT_CONFIG_H
#define BOOT_CONFIG_H

#include <stdint.h>

#ifndef XIP_BASE
#define XIP_BASE 0x10000000u
#endif
#ifndef SRAM_BASE
#define SRAM_BASE 0x20000000u
#endif

/* Size reserved for the bootloader at the start of flash. Must be a multiple of
 * the 4096-byte flash sector. The bootloader pulls in the full pico_shared
 * framework (HSTX + USB host + FatFs + fonts), so 512 KB is comfortable. */
#ifndef BOOTLOADER_SIZE
#define BOOTLOADER_SIZE (512u * 1024u)
#endif

/* Total external flash on the board. Fruit Jam = 16 MB. Override for others. */
#ifndef FLASH_TOTAL_SIZE
#define FLASH_TOTAL_SIZE (16u * 1024u * 1024u)
#endif

/* Bytes carved out of the TOP of flash for the always-resident Fruit Jam
 * retro-TV firmware. 0 (the default) reproduces the original, TV-less
 * partition map exactly -- see BootPartition.cmake in this repo's
 * CMakeLists.txt for where the Fruit Jam build sets this to 4 MB. Must be a
 * multiple of the 4096-byte flash sector, same as BOOTLOADER_SIZE. */
#ifndef TV_RESERVED_SIZE
#define TV_RESERVED_SIZE (0u)
#endif

/* Application partition: everything after the bootloader and before the TV
 * reserve (which is 0 on every board except Fruit Jam, so this is a no-op
 * everywhere else). */
#define APP_PARTITION_OFFSET (BOOTLOADER_SIZE)                 /* XIP-relative   */
#define APP_PARTITION_SIZE   (FLASH_TOTAL_SIZE - BOOTLOADER_SIZE - TV_RESERVED_SIZE)
#define APP_BASE_ADDR        (XIP_BASE + APP_PARTITION_OFFSET) /* absolute (0x10080000) */
#define APP_END_ADDR         (APP_BASE_ADDR + APP_PARTITION_SIZE)

/* Fruit Jam retro-TV region: the last TV_RESERVED_SIZE bytes of flash. When
 * TV_RESERVED_SIZE is 0, TV_BASE_ADDR == TV_END_ADDR == APP_END_ADDR and
 * every TV_RESERVED_SIZE>0-guarded code path in main.cpp is compiled out, so
 * nothing ever reads at that (degenerate) address range. */
#define TV_BASE_ADDR (XIP_BASE + FLASH_TOTAL_SIZE - TV_RESERVED_SIZE)
#define TV_END_ADDR  (XIP_BASE + FLASH_TOTAL_SIZE)

/* RP2350 has 520 KB of SRAM (0x20000000 .. 0x20082000); used to sanity-check a
 * loaded image's initial stack pointer. */
#define SRAM_END_ADDR (SRAM_BASE + (520u * 1024u))

#endif /* BOOT_CONFIG_H */
