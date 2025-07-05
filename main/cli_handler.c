#include "cli_handler.h"
#include "esp_console.h"      // ESP-IDF console system (CLI core)
#include "esp_log.h"          // For logging
#include "argtable3/argtable3.h"  // For argument parsing in commands

#include "state_machine.h"    // Access to get/transition state

static const char *TAG = "CLI_HANDLER";

// ====================================================
// Register all CLI commands on startup
// This gets called once from app_main()
// ====================================================
void cli_register_commands(void)
{
    ESP_LOGI(TAG, "Registering CLI commands...");

    // You’ll later register each command here using:
    // esp_console_cmd_register(&cmd);
    //
    // Example:
    // esp_console_cmd_register(&set_state_cmd);
    // esp_console_cmd_register(&get_state_cmd);
    // esp_console_cmd_register(&rtv_on_cmd);
    //
    // Each will be declared below this function.
}

// ====================
// EXAMPLE STRUCTURE:
// You’ll later define CLI commands using:
// 1. A struct of arguments (optional)
// 2. A handler function
// 3. A command descriptor
// ====================

/*
 * Future Example Command:
 * Command: set_state OPERATIONAL
 *
 * Step 1: Define handler function
int cmd_set_state(int argc, char **argv)
{
    // Parse argument from argv[1], validate and call transition_to_state()
    return 0;
}

 * Step 2: Register the command
static const esp_console_cmd_t set_state_cmd = {
    .command = "set_state",
    .help = "Change the current system state",
    .hint = NULL,
    .func = &cmd_set_state,
    .argtable = NULL  // For now, we skip advanced argument parsing
};
*/

