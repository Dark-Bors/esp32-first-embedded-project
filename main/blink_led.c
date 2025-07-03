// File: main/blink_led.c

#include <stdio.h>

void show_banner(void) {
    printf("\n");
    printf("###################################################################################\n");
    printf("##                                 BLINKY DEMO                                   ##\n");
    printf("##                                                                               ##\n");
    printf("##  ESP32-S3 FreeRTOS LED Blinker + Logger                                       ##\n");
    printf("##                                                                               ##\n");
    printf("##  Author     : Dark Bors                                                       ##\n");
    printf("##  Version    : v1.0.1                                                          ##\n");
    printf("##  Date       : July 1, 2025                                                    ##\n");
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

    // vTaskDelete(NULL); // Optional
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

