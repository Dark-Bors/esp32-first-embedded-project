// File: main/led_handler.c
// Description: Implements LED control logic for various system modes and patterns.

#include "led_handler.h" // Header file for LED handling
#include "driver/gpio.h" // ESP-IDF GPIO driver for LED control
#include "esp_timer.h" // ESP-IDF timer library for LED timing
#include "esp_log.h" // ESP-IDF logging library


static const char *TAG = "LED_HANDLER"; // Log tag for this module
static esp_timer_handle_t led_timer = NULL;
static bool led_state = false;
static uint32_t blink_on_us = 500000;   // 500ms default ON
static uint32_t blink_off_us = 500000;  // 500ms default OFF

// Timer callback function to toggle the LED state
// This function is called by the ESP timer when the timer expires
// It toggles the LED state and restarts the timer with the next interval
// The 'arg' parameter can be used to pass additional data, but we don't need it
// in this case, so we leave it as void*.
static void led_timer_callback(void* arg) {
    led_state = !led_state;
    led_set_static(led_state);
    ESP_LOGD(TAG, "Timer callback triggered. New state: %s", led_state ? "ON" : "OFF");

    uint64_t next_interval = led_state ? blink_on_us : blink_off_us;
    ESP_LOGD(TAG, "Scheduling next timer: %llu us", next_interval);

    // ⚠️ Removed unnecessary stop
    esp_timer_start_once(led_timer, next_interval);  // Just reschedule
}



// initialize the led GPIO 
void led_handler_init(void) {
    ESP_LOGI(TAG, "Initializing LED handler..."); // DEBUG log for initialization
    // Configure the LED GPIO pin
    gpio_config_t io_conf = {
        // Set the pin bit mask for GPIO2 (the LED pin)
        // This assumes GPIO2 is used for the LED
        // The pin_bit_mask is a bitmask where each bit corresponds to a GPIO pin
        // In this case, we are using GPIO2, so we set the second bit (bit 2) to 1.
        // 1ULL is used to ensure it's treated as a 64-bit unsigned long long
        // This is necessary for the pin_bit_mask field in gpio_config_t
        // The pin_bit_mask is a 64-bit value where each bit represents a GPIO pin
        // For example, if we want to use GPIO2, we set the second bit (bit 2) to 1.
        // The 1ULL ensures that the value is treated as a 64-bit unsigned
        // long integer, which is required for the pin_bit_mask field in gpio_config_t.
        // This allows us to configure multiple GPIOs if needed in the future.
        .pin_bit_mask = (1ULL << GPIO_NUM_2), // Use GPIO2 for the LED
        .mode = GPIO_MODE_OUTPUT, // Set as output
        // Set the GPIO mode to output
        // This means the GPIO will be used to drive the LED
        // GPIO_MODE_OUTPUT is used to configure the GPIO pin as an output
        // This allows us to control the LED by setting the GPIO high or low
        .pull_up_en = GPIO_PULLUP_DISABLE, // No pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // No pull-down
        // pull-up and pull-down resistors are not needed for a simple LED
        // We are not using pull-up or pull-down resistors for the LED GPIO
        // This is because the LED will be driven directly by the GPIO output
        // and does not require any external resistors for pull-up or pull-down.
        .intr_type = GPIO_INTR_DISABLE // No interrupts 
        // Disable interrupts for this GPIO
        // Since this is a simple LED control, we don't need to handle interrupts
        // This means the GPIO will not generate any interrupt events
        // This is suitable for a simple LED that we control directly
        // We are not using interrupts for the LED, so we set this to GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf); // Apply the GPIO configuration
    // Apply the GPIO configuration using the gpio_config function
    // This function initializes the GPIO with the specified configuration
    // The gpio_config function takes a pointer to a gpio_config_t structure
    // which contains the configuration settings for the GPIO pin.
    ESP_LOGD(TAG, "GPIO configuration applied for GPIO %d", GPIO_NUM_2); // DEBUG log for GPIO configuration
    esp_timer_create_args_t timer_args = {
        .callback = &led_timer_callback,
        .name = "led_blink_timer"
    };
    esp_timer_create(&timer_args, &led_timer); // Create a new timer with the specified callback and name

    ESP_LOGI(TAG, "LED handler initialized on GPIO %d", GPIO_NUM_2);
}

// For better system management (especially on shutdown or reset),
// we should ensure the LED is turned off and the timer is stopped/deleted
// This function deinitializes the LED handler
// It stops and deletes the LED timer, ensuring no resources are left floating
void led_handler_deinit(void) {
    ESP_LOGI(TAG, "Deinitializing LED handler..."); // DEBUG log for deinitialization
    if (led_timer) {
        ESP_LOGD(TAG, "Stopping and deleting timer"); // DEBUG log for timer cleanup
        esp_timer_stop(led_timer);
        esp_timer_delete(led_timer);
        led_timer = NULL;
    }
    led_off(); // Ensure LED is off
}


// apply asic functions like led_on, led_off, led_set_static
// make sure that each write to the GPIO directly
void led_on(void) {
    // Set the GPIO pin high to turn on the LED
    gpio_set_level(GPIO_NUM_2, 1); // 1 means ON
    ESP_LOGI(TAG, "LED turned ON");
    // Log the action of turning on the LED
}
void led_off(void) {
    // Set the GPIO pin low to turn off the LED
    gpio_set_level(GPIO_NUM_2, 0); // 0 means OFF
    ESP_LOGI(TAG, "LED turned OFF");
    // Log the action of turning off the LED
}
void led_set_static(bool on) {
    ESP_LOGD(TAG, "led_set_static(%s)", on ? "true" : "false"); // DEBUG log for static state setting

    // Set the LED to a static state based on the 'on' parameter
    if (on) {
        led_on(); // Call led_on if 'on' is true
    } else {
        led_off(); // Call led_off if 'on' is false
    }
    ESP_LOGI(TAG, "LED set to static state: %s", on ? "ON" : "OFF");
}

// This function sets the LED to a static state (ON or OFF)
// It takes a boolean parameter 'on' to determine the state
void led_blink(float frequency_hz, float duty_cycle_percent) {
    ESP_LOGI(TAG, "Requested LED blink → Frequency: %.2f Hz, Duty: %.2f%%", frequency_hz, duty_cycle_percent); // DEBUG log for blink request

    uint32_t period_us = (uint32_t)(1000000 / frequency_hz); // Calculate the period in microseconds
    // Convert frequency from Hz to microseconds
    blink_on_us = (uint32_t)(period_us * (duty_cycle_percent / 100.0)); // Calculate ON time in microseconds
    // Calculate the ON time based on the duty cycle percentage
    blink_off_us = period_us - blink_on_us; // Calculate OFF time in microseconds
    // Calculate the OFF time by subtracting ON time from the total period
    ESP_LOGD(TAG, "Calculated timing — ON: %u us, OFF: %u us", blink_on_us, blink_off_us); // DEBUG log for calculated timing

    // If the timer already exists, stop and delete it before creating a new one
    // This ensures we don't have multiple timers running simultaneously
    if (led_timer) {
        esp_timer_stop(led_timer);
    } else {
        esp_timer_create_args_t timer_args = {
            .callback = &led_timer_callback,
            .name = "led_blink_timer"
        };
        esp_timer_create(&timer_args, &led_timer); // Only create once
    }

    esp_timer_start_once(led_timer, blink_on_us); // Start the first ON pulse
    ESP_LOGI(TAG, "LED blinking started with frequency %.2f Hz and duty cycle %.2f%%", frequency_hz, duty_cycle_percent);
    // Log the start of the LED blinking with the specified frequency and duty cycle    
}

// Apply a specific LED pattern
void led_apply_pattern(led_pattern_t pattern) {
    ESP_LOGI(TAG, "Applying LED pattern: %d", pattern);

    switch (pattern) {
        case LED_PATTERN_DEV_MODE:
            ESP_LOGI(TAG, "[Pattern] DEV_MODE → 1Hz, 50%%");
            led_blink(1.0f, 50.0f);
            break;

        case LED_PATTERN_OPERATIONAL:
            ESP_LOGI(TAG, "[Pattern] OPERATIONAL → from YAML (placeholder: 2Hz, 50%%)");
            // TODO: [YAML] Replace hardcoded values with config from parsed YAML file
            led_blink(2.0f, 50.0f);
            break;

        case LED_PATTERN_RTV_ACTIVE:
            // TODO: [RTV] Implement 5x short burst blinking
            ESP_LOGW(TAG, "[Pattern] RTV_ACTIVE is not yet implemented.");
            break;

        case LED_PATTERN_HALTED_ENTRY:
            // TODO: [HALTED] Blink at 5Hz forever
            ESP_LOGW(TAG, "[Pattern] HALTED_ENTRY is not yet implemented.");
            break;

        case LED_PATTERN_TRANSFER_COMPLETE:
            // TODO: [TRANSFER] Fade out slowly
            ESP_LOGW(TAG, "[Pattern] TRANSFER_COMPLETE is not yet implemented.");
            break;

        default:
            ESP_LOGW(TAG, "Unknown LED pattern: %d", pattern);
            break;
    }
}
