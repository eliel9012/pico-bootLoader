/*
 * app_launch.h - Hand control from the bootloader to the application that lives
 *                at APP_BASE_ADDR.
 *
 * Two mechanisms are provided and selected at compile time:
 *
 *   BOOT_USE_ROM_CHAIN == 0  (default)  Classic Cortex-M vector-table jump:
 *        relocate VTOR, load the app's initial SP, branch to its reset vector.
 *        Simple, well understood, no partition table required. This is the
 *        tested default.
 *
 *   BOOT_USE_ROM_CHAIN == 1             Use the RP2350 bootrom rom_chain_image()
 *        function, which re-runs the ROM's image loader on the partition. This
 *        is the "blessed" RP2350 path (handles signed/encrypted images, flash
 *        re-init, etc.) but generally expects a proper partition table and can
 *        return BOOTROM_ERROR_NOT_PERMITTED otherwise. See app_launch.c.
 */
#ifndef APP_LAUNCH_H
#define APP_LAUNCH_H

#include <stdbool.h>
#include <stdint.h>

/* Does APP_BASE_ADDR look like a valid, programmed image (sane SP + reset
 * vector, not just erased 0xFF)? */
bool app_launch_present(void);

/* Tear down the bootloader's runtime and transfer control to the application.
 * Does not return on success. Returns (with the bootloader still running) only
 * if no valid image is present or the chosen launch method refused. */
void app_launch_run(void);

/*
 * Region-parametrised versions of the two functions above. app_launch_present()
 * and app_launch_run() are thin wrappers around these for
 * (APP_BASE_ADDR, APP_END_ADDR); the Fruit Jam retro-TV hand-off uses them
 * directly for (TV_BASE_ADDR, TV_END_ADDR) -- see boot_config.h. `base` and
 * `end` are absolute XIP addresses framing the candidate region.
 *
 * app_launch_run_at() only supports the classic vector-table jump (the
 * BOOT_USE_ROM_CHAIN=1 path in app_launch.c is unmodified and still only
 * targets APP_BASE_ADDR/APP_PARTITION_SIZE; that combination is not used by
 * the Fruit Jam build).
 */
bool app_launch_present_at(uint32_t base, uint32_t end);
void app_launch_run_at(uint32_t base, uint32_t end);

#endif /* APP_LAUNCH_H */
