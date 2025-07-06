// File: main/main.c

#include <stdio.h>                      // Standard I/O for printing messages
#include "freertos/FreeRTOS.h"         // FreeRTOS header for task management
#include "freertos/task.h"             // FreeRTOS task functions (vTaskDelay)
#include "led_handler.h"               // Custom header for LED control
#include "state_machine.h"             // Custom header for system state management

void show_banner(void) {
    printf("\n");
    printf("###################################################################################\n");
    printf("##                               OptiPulse™ State                                ##\n");
    printf("##                         Developer Training Project                            ##\n");
    printf("##                                                                               ##\n");
    printf("##  This embedded project demonstrates CLI control, LED feedback,                ##\n");
    printf("##  camera RTV sessions, and persistent state logic using the ESP32-S3.          ##\n");
    printf("###################################################################################\n\n");
}

void app_main(void) {
    show_banner();  // Show startup banner

    // === Init LED Control ===
    led_handler_init();

    // === Pattern: DEV_MODE ===
    printf("[MAIN] Applying DEV_MODE pattern (1Hz)\n");
    led_apply_pattern(LED_PATTERN_DEV_MODE);  // Blinks at 1Hz
    vTaskDelay(pdMS_TO_TICKS(5000));          // Wait 5 seconds

    // === Pattern: OPERATIONAL ===
    printf("[MAIN] Applying OPERATIONAL pattern (2Hz)\n");
    led_apply_pattern(LED_PATTERN_OPERATIONAL);  // Blinks at 2Hz
    vTaskDelay(pdMS_TO_TICKS(5000));             // Wait 5 seconds

    // === Clean Shutdown ===
    printf("[MAIN] Deinitializing LED handler\n");
    led_handler_deinit();

    // === TODO: State Machine Integration ===
    // state_machine_start();
}
