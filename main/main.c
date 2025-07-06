// File: main/main.c
// ==========================================================================================
// Main application for OptiPulse™ Developer Training Project
// This program tests all LED patterns implemented in led_handler.c
// ==========================================================================================

#include <stdio.h>                      // Standard I/O
#include "freertos/FreeRTOS.h"         // FreeRTOS core
#include "freertos/task.h"             // Delay / task APIs
#include "led_handler.h"               // LED control interface
#include "state_machine.h"             // Placeholder
#include "esp_system.h"                // ESP-IDF system info
#include "driver/uart.h"               // For serial input

void show_banner(void) {
    printf("\n");
    printf("###################################################################################\n");
    printf("##                               OptiPulse™ State                                ##\n");
    printf("##                         Developer Training Project                            ##\n");
    printf("##                                 v1.4.0-dev                                    ##\n");
    printf("##  This embedded project demonstrates CLI control, LED feedback,                ##\n");
    printf("##  camera RTV sessions, and persistent state logic using the ESP32-S3.          ##\n");
    printf("###################################################################################\n\n");
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

void app_main(void) {
    show_banner();
    printf("\n[FIRMWARE HALT] Type 'c' and press ENTER to continue...\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    wait_for_user_to_continue();

    // === Initialize LED control ===
    led_handler_init();

    // === Pattern 1: DEV_MODE ===
    printf("[MAIN] Applying DEV_MODE pattern (constant ON)\n");
    led_apply_pattern(LED_PATTERN_DEV_MODE);
    vTaskDelay(pdMS_TO_TICKS(5000));

    printf("[MAIN] LED debug status (should show info only in DEV_MODE)\n");
    led_debug_status();
    vTaskDelay(pdMS_TO_TICKS(2000));

    // === Pattern 2: OPERATIONAL ===
    printf("[MAIN] Applying OPERATIONAL pattern (1Hz blinking)\n");
    led_apply_pattern(LED_PATTERN_OPERATIONAL);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // === Pattern 3: RTV_ACTIVE ===
    printf("[MAIN] Applying RTV_ACTIVE pattern (5x 10Hz blinks → pause)\n");
    led_apply_pattern(LED_PATTERN_RTV_ACTIVE);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // === Pattern 4: HALTED_ENTRY ===
    printf("[MAIN] Applying HALTED_ENTRY pattern (2Hz for 5s, then OFF)\n");
    led_apply_pattern(LED_PATTERN_HALTED_ENTRY);
    vTaskDelay(pdMS_TO_TICKS(3000));  // Enough to confirm it stops

    // === Pattern 5: TRANSFER_COMPLETE ===
    printf("[MAIN] Applying TRANSFER_COMPLETE pattern (500ms ON / 1s OFF)\n");
    led_apply_pattern(LED_PATTERN_TRANSFER_COMPLETE);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // === Pattern 6: TETHERED ===
    printf("[MAIN] Applying TETHERED pattern (0.5Hz slow blink)\n");
    led_apply_pattern(LED_PATTERN_TETHERED);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // === Pattern 7: UNTETHERED ===
    printf("[MAIN] Applying UNTETHERED pattern (10x 2Hz blinks → 500ms pause)\n");
    led_apply_pattern(LED_PATTERN_UNTETHERED);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // === Debug status in non-DEV_MODE ===
    printf("[MAIN] LED debug status (should warn: not in DEV_MODE)\n");
    led_debug_status();
    vTaskDelay(pdMS_TO_TICKS(2000));

    // === Deinitialize and exit ===
    printf("[MAIN] Deinitializing LED handler\n");
    led_handler_deinit();

    // Reserved for CLI/RTV/Storage logic
    // state_machine_start();
}
