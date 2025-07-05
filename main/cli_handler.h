#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

// ==============================
// Purpose:
// This header defines the CLI system for interacting with your ESP32.
// You’ll be able to type commands like "set_state OPERATIONAL" or "rtv_on"
// via UART (USB CLI console).
// ==============================

/**
 * @brief Initialize the CLI command system.
 * This will register all CLI commands (e.g., set_state, get_state, help).
 * Must be called from app_main() or equivalent at startup.
 */
void cli_register_commands(void);

#endif // CLI_HANDLER_H
