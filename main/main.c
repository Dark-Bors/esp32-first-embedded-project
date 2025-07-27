// File: main/main.c
// ==========================================================================================
// Main application for OptiPulse™ Developer Training Project
// This stripped-down build demonstrates CLI usage over UART on ESP32-S3
// ==========================================================================================

#include <stdio.h>                      // Standard I/O
#include "freertos/FreeRTOS.h"         // FreeRTOS core
#include "freertos/task.h"             // Delay / task APIs
#include "led_handler.h"               // LED control interface
#include "state_machine.h"             // Placeholder
#include "esp_system.h"                // ESP-IDF system info
#include "driver/uart.h"               // For serial input
#include "version_config.h"            // Firmware version and debug config
#include "cli_handler.h"              // CLI command registration
#include "esp_vfs_dev.h"
#include "esp_console.h"
// #include "linenoise.h"  // For line editing + history
#include "driver/uart_vfs.h"
#include "linenoise/linenoise.h"    // For line editing + history
#include "security.h"   ///< Provides get_security_level()


void show_banner(void) {
    printf("\n");
    printf("###################################################################################\n");
    printf("##                               OptiPulse™ State                                ##\n");
    printf("##                         Developer Training Project                            ##\n");
    printf("##                                %s                                   ##\n", FW_VERSION_STRING);
    printf("##  This embedded project demonstrates CLI control, LED feedback,                ##\n");
    printf("##  camera RTV sessions, and persistent state logic using the ESP32-S3.          ##\n");
    printf("###################################################################################\n\n");
    // Display firmware version for debugging purposes!
    printf("Firmware Version: %s (Major=%d, Minor=%d, Build=%d)\n",
       FW_VERSION_STRING,
       FW_VERSION_MAJOR,
       FW_VERSION_MINOR,
       FW_VERSION_BUILD);

    if (security_get_level() <= 2) {
        printf("[DEBUG] Developer mode active: shortcuts allowed\n");
    }

}


// Wait for user to press 'c' before starting logic
void wait_for_user_to_continue(void) {
    char c = 0;

    // UART0 config
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);

    while (1) {
        int len = uart_read_bytes(UART_NUM_0, (uint8_t*)&c, 1, pdMS_TO_TICKS(100));
        if (len > 0 && (c == 'c' || c == 'C')) {
            printf("[CONTINUE] Starting main functionality...\n\n");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// === CLI Task to process serial input ===
void cli_task(void *arg)
{
    setvbuf(stdin, NULL, _IONBF, 0);
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    esp_console_config_t console_config = {
        .max_cmdline_length = 256,
        .max_cmdline_args = 8
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));
    linenoiseSetDumbMode(1);

    while (true) {
        char *line = linenoise("> ");
        if (line == NULL) continue;
        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
            int ret;
            esp_err_t err = esp_console_run(line, &ret);
            if (err == ESP_ERR_NOT_FOUND) {
                printf("Unrecognized command\n");
            } else if (err == ESP_ERR_INVALID_ARG) {
                printf("Invalid arguments\n");
            } else if (err != ESP_OK) {
                printf("Error: %s\n", esp_err_to_name(err));
            }
        }
        linenoiseFree(line);
    }
}


void app_main(void) {
    show_banner();
    printf("\n[FIRMWARE HALT] Type 'c' and press ENTER to continue...\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    wait_for_user_to_continue();

    uart_vfs_dev_use_driver(UART_NUM_0);
    
    // Initialize LED handler
    printf("[LED_HANDLER] LED handler init\n"); 
    led_handler_init(); // Initialize LED GPIO and timer

    // register USB commands
    cli_register_usb_commands();  // Must be called during boot
    xTaskCreate(cli_task, "cli_task", 4096, NULL, 5, NULL);

}
 

//     // === Pattern 1: DEV_MODE ===
//     printf("[MAIN] Applying DEV_MODE pattern (constant ON)\n");
//     led_apply_pattern(LED_PATTERN_DEV_MODE);
//     vTaskDelay(pdMS_TO_TICKS(5000));

//     printf("[MAIN] LED debug status (should show info only in DEV_MODE)\n");
//     led_debug_status();
//     vTaskDelay(pdMS_TO_TICKS(2000));

//     // === Pattern 2: OPERATIONAL ===
//     printf("[MAIN] Applying OPERATIONAL pattern (1Hz blinking)\n");
//     led_apply_pattern(LED_PATTERN_OPERATIONAL);
//     vTaskDelay(pdMS_TO_TICKS(5000));

//     // === Pattern 3: RTV_ACTIVE ===
//     printf("[MAIN] Applying RTV_ACTIVE pattern (5x 10Hz blinks → pause)\n");
//     led_apply_pattern(LED_PATTERN_RTV_ACTIVE);
//     vTaskDelay(pdMS_TO_TICKS(5000));

//     // === Pattern 4: HALTED_ENTRY ===
//     printf("[MAIN] Applying HALTED_ENTRY pattern (2Hz for 5s, then OFF)\n");
//     led_apply_pattern(LED_PATTERN_HALTED_ENTRY);
//     vTaskDelay(pdMS_TO_TICKS(3000));  // Enough to confirm it stops

//     // === Pattern 5: TRANSFER_COMPLETE ===
//     printf("[MAIN] Applying TRANSFER_COMPLETE pattern (500ms ON / 1s OFF)\n");
//     led_apply_pattern(LED_PATTERN_TRANSFER_COMPLETE);
//     vTaskDelay(pdMS_TO_TICKS(5000));

//     // === Pattern 6: TETHERED ===
//     printf("[MAIN] Applying TETHERED pattern (0.5Hz slow blink)\n");
//     led_apply_pattern(LED_PATTERN_TETHERED);
//     vTaskDelay(pdMS_TO_TICKS(5000));

//     // === Pattern 7: UNTETHERED ===
//     printf("[MAIN] Applying UNTETHERED pattern (10x 2Hz blinks → 500ms pause)\n");
//     led_apply_pattern(LED_PATTERN_UNTETHERED);
//     vTaskDelay(pdMS_TO_TICKS(5000));

//     // === Debug status in non-DEV_MODE ===
//     printf("[MAIN] LED debug status (should warn: not in DEV_MODE)\n");
//     led_debug_status();
//     vTaskDelay(pdMS_TO_TICKS(2000));

//     // === Deinitialize and exit ===
//     printf("[MAIN] Deinitializing LED handler\n");
//     led_handler_deinit();

//     // Reserved for CLI/RTV/Storage logic
//     // state_machine_start();
// }
