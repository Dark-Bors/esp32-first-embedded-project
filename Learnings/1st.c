// FreeRTOS headers for delays and task handling
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// GPIO driver for configuring and controlling GPIOs
#include "driver/gpio.h"

// ESP-IDF logging facility
#include "esp_log.h"

// Define a tag for log messages
static const char *TAG = "BLINK";

// Define the GPIO number for the onboard blue LED
#define LED_GPIO GPIO_NUM_2

/**
 * @brief Configure the specified GPIO as an output
 */
void configure_led_gpio() {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO,     // Bit mask to select GPIO2
        .mode = GPIO_MODE_OUTPUT,             // Set as output mode
        .pull_up_en = GPIO_PULLUP_DISABLE,    // Disable internal pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,// Disable internal pull-down
        .intr_type = GPIO_INTR_DISABLE        // Disable interrupts
    };
    gpio_config(&io_conf);
}

/**
 * @brief Main application entry point
 *
 * Continuously toggles the onboard blue LED every 500ms
 */
void app_main(void) {
    // Configure GPIO2 as output
    configure_led_gpio();

    // Start blinking loop
    while (1) {
        ESP_LOGI(TAG, "LED ON (GPIO %d)", LED_GPIO);
        gpio_set_level(LED_GPIO, 1);  // Turn LED ON
        vTaskDelay(500 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "LED OFF (GPIO %d)", LED_GPIO);
        gpio_set_level(LED_GPIO, 0);  // Turn LED OFF
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

/*
======== Summary ========

- This code blinks the onboard blue LED on an ESP32-S3 board.
- GPIO2 is used (confirmed to control the onboard LED).
- LED turns ON for 500 ms, OFF for 500 ms, in a loop.
- Uses FreeRTOS `vTaskDelay()` for non-blocking timing.
- All GPIO setup follows ESP-IDF best practices.

To build and flash:
1. Run: `idf.py build`
2. Run: `idf.py -p COMx flash monitor`
   (replace COMx with your actual COM port)

You can also monitor with TeraTerm once the flash completes and the port is available.

*/
