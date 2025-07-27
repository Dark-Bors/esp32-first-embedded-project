/**
 * @file cli_handler_usb.c
 * @brief USB CLI implementation for developer/debug commands over UART.
 *
 * This module registers and handles CLI commands available over USB serial,
 * including FSM state transitions, LED control, memory access, and more.
 */

#include "esp_log.h"
#include "esp_console.h"
#include "state_machine.h"
#include "cli_handler.h"
#include "led_handler.h"
#include "security.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "argtable3/argtable3.h"  // Needed if you use arguments

static const char* TAG = "CLI_USB";

// =============================
// command pause mode 
// When the user types cmd, the firmware suppresses normal log output for a few seconds.
// This is useful for debugging without flooding the console with logs.
// The timer will automatically restore normal logging after a timeout.
// =============================
static bool cli_quiet_mode = false;
static TimerHandle_t quiet_timer = NULL;

void quiet_mode_timeout_cb(TimerHandle_t xTimer) {
    cli_quiet_mode = false;
    esp_log_level_set("*", ESP_LOG_INFO);  // Restore normal logging
    printf("\n[CLI] Quiet mode ended — logging resumed\n> ");
}


// =============================
// Command Handlers (Used)
// =============================
static int cmd_fsm_state(int argc, char **argv);   ///< Show current FSM state
static int cmd_fsm_set(int argc, char **argv);     ///< Force FSM state
static int cmd_led_on(int argc, char **argv);      ///< Force LED ON
static int cmd_led_off(int argc, char **argv);     ///< Force LED OFF

#if 0
// =============================
// Command Handlers (TODO Stubs)
// =============================

// --- FSM Commands ---
static int cmd_fsm_trigger(int argc, char **argv); // TODO

// --- LED Commands ---
static int cmd_led_pattern(int argc, char **argv); // TODO
static int cmd_led_burst(int argc, char **argv);   // TODO
static int cmd_led_debug(int argc, char **argv);   // TODO

// --- Security Commands ---
static int cmd_sec_level(int argc, char **argv);   // TODO
static int cmd_sec_unlock(int argc, char **argv);  // TODO

// --- Memory Access Commands ---
static int cmd_mem_map(int argc, char **argv);     // TODO
static int cmd_mem_read(int argc, char **argv);    // TODO
static int cmd_mem_write(int argc, char **argv);   // TODO
static int cmd_mem_dump(int argc, char **argv);    // TODO

// --- RTV Commands ---
static int cmd_rtv_start_usb(int argc, char **argv);   // TODO
static int cmd_rtv_stop(int argc, char **argv);        // TODO
static int cmd_rtv_status(int argc, char **argv);      // TODO

// --- System/Meta CLI Commands ---
static int cmd_cli_version(int argc, char **argv);      // TODO
static int cmd_cli_help(int argc, char **argv);         // TODO
static int cmd_cli_bdds_start(int argc, char **argv);   // TODO
static int cmd_cli_reset(int argc, char **argv);        // TODO
static int cmd_cli_tick(int argc, char **argv);         // TODO

// --- Developer Debug Commands ---
static int cmd_dbg_state(int argc, char **argv);        // TODO
static int cmd_dbg_event(int argc, char **argv);        // TODO
#endif


/**
 * @brief CLI command: Set FSM to a new state by number
 *
 * Usage:
 *     fsm.set <N>
 *
 * State numbers:
 *     1 = DEV
 *     2 = OPERATIONAL
 *     3 = TETHERED
 *     4 = UNTETHERED
 *     5 = RTV
 *     6 = HALTED
 *
 * @note This command forces state transition directly — available only when
 *       CONFIG_DEV_SHORTCUTS_ENABLED is enabled in menuconfig.
 *
 * @param argc Number of arguments
 * @param argv Command arguments
 * @return int 0 on success, -1 on invalid input, -2 if blocked
 */
static int cmd_fsm_set(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: fsm.set <STATE_NUM>\n");
        printf("  1 = DEV, 2 = OPERATIONAL, 3 = TETHERED, 4 = UNTETHERED, 5 = RTV, 6 = HALTED\n");
        return -1;
    }

    int value = atoi(argv[1]);
    if (value < 1 || value > 6) {
        printf("Invalid state number: %d\n", value);
        return -1;
    }

    SystemState target_state = (SystemState)(value - 1);

#ifdef CONFIG_DEV_SHORTCUTS_ENABLED
    ESP_LOGW(TAG, "[DEV] Forcing FSM transition: %s (%d) --> %s (%d)",
             state_to_string(get_current_state()), get_current_state(),
             state_to_string(target_state), target_state);
    transition_to_state(target_state);
    return 0;
#else
    printf("[SECURITY] Direct FSM transitions disabled in production build.\n");
    return -2;
#endif
}

/**
 * @brief CLI command: Print current FSM state
 *
 * Usage:
 *     fsm.state
 *
 * @param argc Number of arguments
 * @param argv Command arguments
 * @return int Always returns 0
 */
static int cmd_fsm_state(int argc, char **argv)
{
    SystemState state = get_current_state();
    printf("[FSM] Current State: %s (%d)\n", state_to_string(state), state);
    return 0;
}

/**
 * @brief CLI command: Turn LED ON (static)
 *
 * Usage:
 *     led.on
 *
 * @return int Always returns 0
 */
static int cmd_led_on(int argc, char **argv)
{
    led_set_static(true);
    ESP_LOGI(TAG, "[LED] Forced ON");
    return 0;
}

/**
 * @brief CLI command: Turn LED OFF (static)
 *
 * Usage:
 *     led.off
 *
 * @return int Always returns 0
 */
static int cmd_led_off(int argc, char **argv)
{
    led_set_static(false);
    ESP_LOGI(TAG, "[LED] Forced OFF");
    return 0;
}

/** 
* @brief CLI command: Enable quiet mode for 5 seconds
* This suppresses all log output except critical errors.
* Usage:
*     quiet
* @return int 0 on success, -1 if security level is insufficient
*/
static int cmd_quiet(int argc, char **argv) {
    if (security_get_level() < 2) {
        printf("Unauthorized: Only high security level can use quiet mode.\n");
        return -1;
    }

    cli_quiet_mode = true;
    esp_log_level_set("*", ESP_LOG_NONE);  // Mute all logs
    printf("[CLI] Quiet mode enabled — logs suppressed for 5 sec\n");

    if (quiet_timer) {
        xTimerStop(quiet_timer, 0);
        xTimerStart(quiet_timer, 0);
    } else {
        quiet_timer = xTimerCreate("quiet_timer", pdMS_TO_TICKS(5000), pdFALSE, NULL, quiet_mode_timeout_cb);
        xTimerStart(quiet_timer, 0);
    }

    return 0;
}

/**
 * @brief Registers all CLI commands available over USB console
 *
 * This function must be called during system boot.
 */
void cli_register_usb_commands(void) {
    const esp_console_cmd_t cmd_state = {
        .command = "fsm.state",
        .help = "Print current FSM state",
        .hint = NULL,
        .func = cmd_fsm_state,
    };

    const esp_console_cmd_t cmd_set = {
        .command = "fsm.set",
        .help = "Force FSM state (DEV, OP, etc)",
        .hint = "<state_id>",
        .func = cmd_fsm_set,
    };

    const esp_console_cmd_t cmd_led_on_cmd = {
        .command = "led.on",
        .help = "Force LED ON",
        .hint = NULL,
        .func = cmd_led_on,
    };

    const esp_console_cmd_t cmd_led_off_cmd = {
        .command = "led.off",
        .help = "Force LED OFF",
        .hint = NULL,
        .func = cmd_led_off,
    };

    const esp_console_cmd_t cmd_quiet_cmd = {
    .command = "cmd",
    .help = "Enable 5s quiet mode (suppress logs)",
    .hint = NULL,
    .func = cmd_quiet,
    };
    
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_quiet_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_state));
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_set));
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_led_on_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_led_off_cmd));
}
