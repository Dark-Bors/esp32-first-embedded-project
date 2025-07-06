<<<<<<< HEAD

#include <stdio.h>
=======
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
>>>>>>> 3d6a7df (🔧 v1.4.0-dev: Finalize LED behavior, timing, debug status)

void show_banner(void) {
    printf("\n");
    printf("###################################################################################\n");
<<<<<<< HEAD
    printf("##                                 BLINKY DEMO                                   ##\n");
    printf("##                                                                               ##\n");
    printf("##  ESP32-S3 FreeRTOS LED Blinker + Logger                                       ##\n");
    printf("##                                                                               ##\n");
    printf("##  Author     : Dark Bors                                                       ##\n");
    printf("##  Version    : v1.1.0                                                          ##\n");
    printf("##  Date       : July 4, 2025                                                    ##\n");
    printf("##                                                                               ##\n");
    printf("##  This demo shows multitasking and GPIO control using FreeRTOS and ESP-IDF.    ##\n");
    printf("###################################################################################\n\n");
}

#include "freertos/FreeRTOS.h"      // Core FreeRTOS definitions and types
#include "freertos/task.h"          // Functions for creating and managing tasks (like xTaskCreate)
#include "driver/gpio.h"            // Functions and types for working with GPIO pins (input/output)
#include "esp_log.h"                // ESP logging utility to print to the serial monitor

// ==============================
// Define Project-Level Constants
// ==============================
#define LED_GPIO GPIO_NUM_2         // GPIO number for onboard LED. GPIO2 is often tied to the onboard blue LED.
#define LOGGING_TIMEOUT_SEC 120  // Stop task after 120 seconds 

static const char *TAG = "FREERTOS_DEMO"; // Tag for logging messages, used to filter logs in the console
const int log_interval = 5; // Log every 5 seconds
const int toggle_interval = 15; // Toggle silent mode every 15 seconds

TickType_t seconds_elapsed = 0; // Counter for seconds elapsed since start
TickType_t seconds_since_log = 0; // Counter for seconds since last log
TickType_t seconds_since_toggle = 0; // Counter for seconds since last toggle of silent mode
bool silentMode = false; // Flag to control logging output

////////////////////////////////////////////////////////////////////////////////
// Task 1: Blink the Onboard LED
////////////////////////////////////////////////////////////////////////////////
void led_blink_task(void *pvParameter)
{
    // Configure GPIO2 as an output
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO,     // Use 1ULL (64-bit) to shift to the bit of GPIO2
        // This sets the pin_bit_mask to only affect GPIO2
        // 1ULL is used to ensure the bitmask is treated as a 64-bit unsigned long long
        // This is important for ESP32-S3 which has a 64-bit GPIO numbering scheme
        // The pin_bit_mask is a bitmask where each bit corresponds to a GPIO pin
        // For example, if LED_GPIO is 2, then pin_bit_mask will be 0b000...000100 (64 bits)
        // This means only GPIO2 will be configured, all other bits are 0
        // This is how we specify which GPIO pin we want to configure

        .mode = GPIO_MODE_OUTPUT,             // Set pin as digital output
        .pull_up_en = 0,                      // Disable internal pull-up resistor
        .pull_down_en = 0,                    // Disable internal pull-down resistor
        .intr_type = GPIO_INTR_DISABLE        // No interrupts for this pin
    };
    gpio_config(&io_conf); // Apply configuration to the selected GPIO

    // Infinite loop to blink LED
    while (seconds_elapsed <= LOGGING_TIMEOUT_SEC)
    {
        gpio_set_level(LED_GPIO, 1);          // Turn ON the LED by setting GPIO high
        vTaskDelay(pdMS_TO_TICKS(1000));       // Delay 1000 ms (non-blocking)

        gpio_set_level(LED_GPIO, 0);          // Turn OFF the LED by setting GPIO low
        vTaskDelay(pdMS_TO_TICKS(500));       // Delay another 500 ms
    }

    // Clean up (optional, to satisfy FreeRTOS expectations)
    vTaskDelete(NULL);
=======
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
>>>>>>> 3d6a7df (🔧 v1.4.0-dev: Finalize LED behavior, timing, debug status)
}


////////////////////////////////////////////////////////////////////////////////
// Task 2: Logger Task — shows uptime, toggles silent mode every 15s,
// and prints log every 5s (if not silent). Tracks stack usage too.
////////////////////////////////////////////////////////////////////////////////
void logging_task(void *pvParameter)
{
    while (seconds_elapsed <= LOGGING_TIMEOUT_SEC)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 sec tick

        seconds_elapsed++;
        seconds_since_log++;
        seconds_since_toggle++;

        // Toggle silent mode every 15s
        if (seconds_since_toggle >= toggle_interval) 
        {
            // Toggle silent mode
            // If silentMode is true, set it to false and vice versa
            // This allows us to switch between logging and not logging
            // Useful for debugging or when we want to suppress logs temporarily

            silentMode = !silentMode;
            seconds_since_toggle = 0;

            ESP_LOGI(TAG, "Silent mode %s. %s",
                     silentMode ? "ON" : "OFF",
                     silentMode ? "No further logs will be printed." : "Resuming periodic logs.");
            // Log the change in silent mode status
            // This will print to the console whether we are in silent mode or not
            // If silentMode is true, it will print "Silent mode ON. No further logs will be printed."
            // If silentMode is false, it will print "Silent mode OFF. Resuming periodic logs."
        }

        // Log every 5s only if not in silent mode
        if (!silentMode && seconds_since_log >= log_interval)
        {
            // Log the current status
            // This will print the uptime, LED state, and stack watermark every 5 seconds
            // We use seconds_elapsed to track how long the system has been running
            // seconds_since_log is reset after logging to start the next interval

            seconds_since_log = 0;

            int led_state = gpio_get_level(LED_GPIO);
            UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
            // Get the current state of the LED (1 for ON, 0 for OFF)
            // Get the stack watermark for this task to see how much stack is left
            // uxTaskGetStackHighWaterMark(NULL) returns the minimum amount of stack space that has remained for this task since it started
            // NULL means we are checking the current task (this logging task)
            // This is useful to monitor stack usage and ensure we are not running out of stack space

            ESP_LOGI(TAG, "[INFO] 5s Update - Uptime: %lus | LED State: %s | Stack watermark: %lu bytes",
                     (unsigned long)seconds_elapsed,
                     led_state ? "ON" : "OFF",
                     (unsigned long)watermark);
            // Log the information to the console
            // %lus formats the seconds_elapsed as an unsigned long integer
            // %s formats the LED state as a string (ON or OFF)
            // %lu formats the stack watermark as an unsigned long integer
            // This log will show how long the system has been running, the current state of the LED, and how much stack space is left
            // This is useful for debugging and monitoring the system's health
        
        }

        // Optional: ESP_LOGI(TAG, "Tick: %lus", seconds_elapsed);
    }

    // EOT: Task ran long enough. Start shutdown sequence.

    // 1. Print EOT message
    ESP_LOGI(TAG, "End of Task (EOT) reached at %lu seconds", (unsigned long)seconds_elapsed);

    // 2. Blink LED at 5Hz (10 blinks = 2 seconds)
    ESP_LOGI(TAG, "Start of EOT Notifications");
    for (int i = 0; i < 10; i++) {
        gpio_set_level(LED_GPIO, i % 2); // Toggle LED ON/OFF
        vTaskDelay(pdMS_TO_TICKS(100));  // 100ms delay default
        //for debugging purposes, we can log the blink count
        ESP_LOGI(TAG, "Blink %d", i + 1);
        // This will blink the LED ON for 100ms and OFF for 100ms

        // This will blink the LED 10 times, creating a 5Hz blink rate
        // The LED will be ON for 100ms and OFF for 100ms, resulting
        // in a total of 2 seconds for the entire blink sequence
        // The loop runs 10 times, so it will blink the LED 10 times
    }

    // 3. Ensure LED is off
    gpio_set_level(LED_GPIO, 0);
    ESP_LOGI(TAG, "End of EOT notifications");

    // 4. Log halt
    ESP_LOGW(TAG, "System is now halted. Awaiting manual reset or power cycle.");

    // 5. Sleep forever
    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}


////////////////////////////////////////////////////////////////////////////////
// app_main: Main entry point
// This is called automatically when ESP boots. We create our tasks here.
////////////////////////////////////////////////////////////////////////////////
void app_main(void)
{
    // Show banner at the start of application
    // print_banner();
    show_banner();  // Show immediately after app starts

    // Create task to blink the LED
    // Parameters: (function ptr, task name, stack size, params, priority, handle)
    xTaskCreate(led_blink_task,      // Task function
                "LED_Blink_Task",    // Task name (for debugging)
                4096,                // Stack size in bytes
                NULL,                // Parameters to pass (none here)
                5,                   // Priority (higher = more important)
                NULL);               // No need to store handle

    // Create task to log system status
    xTaskCreate(logging_task,        // Task function
                "Logging_Task",      // Task name
                4096,                // Stack size in bytes
                NULL,                // No parameters
                4,                   // Lower priority than blink
                NULL);               // No handle needed

}

