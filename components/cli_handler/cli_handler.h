/** 
 * @file cli_handler.h
 * @brief Header file for CLI command registration functions.
 * This file declares functions to register CLI commands for different transports.
 */

#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register CLI commands available over USB transport (UART).
 */
void cli_register_usb_commands(void);

/**
 * @brief (Future) Register BLE commands
 */
void cli_register_ble_commands(void);

/**
 * @brief (Optional) Register Wi-Fi CLI commands
 */
void cli_register_wifi_commands(void);

#ifdef __cplusplus
}
#endif

#endif // CLI_HANDLER_H
