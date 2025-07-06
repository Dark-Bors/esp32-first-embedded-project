// File: main/led_handler.c
// ==========================================================================================
// This module implements LED control logic for the OptiPulse™ project.
// It supports various blinking patterns, burst modes, and static ON/OFF control.
// Scheduling is done using the ESP-IDF timer API and GPIO control.
// ==========================================================================================

#include "led_handler.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

// === GPIO Configuration ===
#define GPIO_LED                GPIO_NUM_2       // LED connected to GPIO2

// === Pattern Timings (in microseconds) ===

// --- OPERATIONAL pattern ---
#define BURST_ON_US            10000             // 10ms ON
#define BURST_OFF_US           10000             // 10ms OFF
#define BURST_PAUSE_US         50000             // 50ms pause after burst
#define BURST_MAX_CYCLES       5                 // Number of ON/OFF cycles

// --- HALTED_ENTRY pattern ---
#define HALTED_BLINK_ON_US     100000            // 100ms ON (5Hz)
#define HALTED_BLINK_OFF_US    100000            // 100ms OFF
#define HALTED_MAX_CYCLES      10                // Total 2s duration

// --- TRANSFER_COMPLETE pattern ---
#define TRANSFER_BLINK_ON_US   500000            // 500ms ON (1Hz)
#define TRANSFER_BLINK_OFF_US  500000            // 500ms OFF

// --- TETHERED pattern ---
#define TETHERED_BLINK_ON_US   1000000           // 1s ON (0.5Hz)
#define TETHERED_BLINK_OFF_US  1000000           // 1s OFF

// --- UNTETHERED pattern ---
#define UNTETHERED_BURST_ON_US   250000            // 250ms ON (2Hz)
#define UNTETHERED_BURST_OFF_US  250000            // 250ms OFF
#define UNTETHERED_BURST_CYCLES  10              // Total 40ms burst
#define UNTETHERED_PAUSE_US      500000          // 500ms pause between bursts

// --- RTV pattern ---
#define RTV_BLINK_ON_US          50000           // 50ms ON (20Hz)
#define RTV_BLINK_OFF_US         50000           // 50ms OFF
#define RTV_BURST_CYCLES         5               // 5 bursts
#define RTV_PAUSE_US             500000          // 500ms pause after burst

// === Logging Tag ===
static const char *TAG = "LED_HANDLER";

// === Static Internal State ===

static led_timing_t current_timing;              ///< Current pattern's timing
static esp_timer_handle_t led_timer = NULL;      ///< Timer for scheduling LED toggles
static bool led_state = false;                   ///< Current LED level (true = ON)

static burst_state_t burst = { .active = false, .count = 0 }; ///< Burst tracking
static led_pattern_t current_pattern = LED_PATTERN_DEV_MODE;  ///< Default state

// --- Special Mode Flags ---
static bool halted_mode_active = false;          ///< Indicates if HALTED_ENTRY pattern is running
static int halted_blink_count = 0;               ///< Counter for HALTED blinks

static bool untethered_mode_active = false;      ///< Indicates if UNTETHERED burst is active
static int untethered_blink_count = 0;           ///< Counter for UNTETHERED bursts

// === GPIO LED Control ===

/**
 * @brief Turn ON the LED.
 */
void led_on(void) {
    gpio_set_level(GPIO_LED, 1);
    ESP_LOGI(TAG, "LED turned ON");
}

/**
 * @brief Turn OFF the LED.
 */
void led_off(void) {
    gpio_set_level(GPIO_LED, 0);
    ESP_LOGI(TAG, "LED turned OFF");
}

/**
 * @brief Set the LED to a static state (ON or OFF).
 * 
 * @param on True to turn ON, false to turn OFF.
 */
void led_set_static(bool on) {
    on ? led_on() : led_off();
    ESP_LOGI(TAG, "LED set to static state: %s", on ? "ON" : "OFF");
}

// === Timer Callback ===

/**
 * @brief Callback function for the LED timer
 * 
 * This function is called automatically by the ESP-IDF timer system
 * after a specified ON or OFF interval expires.
 * It toggles the LED state and schedules the next toggle.
 * 
 * Special behavior is handled here for burst, halted, and untethered patterns.
 * Each special mode has its own condition, counter, and stop logic.
 * 
 * @param arg Unused, but required by esp_timer callback signature
 */
static void led_timer_callback(void* arg) {
    // === Toggle LED State ===
    led_state = !led_state;
    led_set_static(led_state);  // Turn LED ON or OFF

    // === Decide next timer interval based on LED state ===
    uint64_t next_interval = led_state ? current_timing.on_us : current_timing.off_us;

    // === BURST MODE: OPERATIONAL or RTV_ACTIVE ===
    if (burst.active && !led_state) {
        burst.count++;  // Only count OFF phases

    int max_cycles = (current_pattern == LED_PATTERN_RTV_ACTIVE)
                    ? RTV_BURST_CYCLES : BURST_MAX_CYCLES;
    uint32_t pause_us = (current_pattern == LED_PATTERN_RTV_ACTIVE)
                        ? RTV_PAUSE_US : BURST_PAUSE_US;

//

//
//
        if (burst.count >= max_cycles) {
            ESP_LOGI(TAG, "[%s] Burst complete → Pausing %d us",
                     current_pattern == LED_PATTERN_RTV_ACTIVE ? "RTV" : "OPERATIONAL",
                     pause_us);

            burst.active = false;
            burst.count = 0;
            esp_timer_start_once(led_timer, pause_us);
            return;
        }
    }

    // === HALTED_ENTRY: 10 total toggles (5s at 2Hz) ===
    if (halted_mode_active && !led_state) {
        halted_blink_count++;
        if (halted_blink_count >= HALTED_MAX_CYCLES) {
            halted_mode_active = false;
            led_set_static(false);  // Final OFF
            ESP_LOGI(TAG, "[HALTED] Done → Holding LED OFF.");
            return;
        }
    }

    // === UNTETHERED: 10x 250ms toggles → pause ===
    if (untethered_mode_active && !led_state) {
        untethered_blink_count++;
        if (untethered_blink_count >= UNTETHERED_BURST_CYCLES) {
            untethered_blink_count = 0;
            ESP_LOGI(TAG, "[UNTETHERED] Burst complete → Pausing %d us", UNTETHERED_PAUSE_US);
            esp_timer_start_once(led_timer, UNTETHERED_PAUSE_US);
            return;
        }
    }

    // === Default: Continue blinking ===
    esp_timer_start_once(led_timer, next_interval);
}


// === Initialization ===

/**
 * @brief Initializes the LED GPIO and timer system
 * 
 * - Configures the LED pin as output
 * - Creates the timer that will toggle the LED at the configured rate
 * - Does NOT start any blinking until a pattern is applied
 */
void led_handler_init(void) {
    ESP_LOGI(TAG, "Initializing LED handler...");

    // === GPIO Configuration ===
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_LED),         // Configure GPIO_LED pin
        .mode = GPIO_MODE_OUTPUT,                   // Set as OUTPUT
        .pull_up_en = GPIO_PULLUP_DISABLE,          // No pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,      // No pull-down
        .intr_type = GPIO_INTR_DISABLE              // Disable interrupts
    };
    gpio_config(&io_conf);  // Apply configuration

    // === Timer Configuration ===
    // Set up a software timer to call `led_timer_callback` at ON/OFF intervals
    esp_timer_create_args_t timer_args = {
        .callback = &led_timer_callback,            // Timer ISR function
        .name = "led_blink_timer"                   // Name for debugging
    };
    esp_timer_create(&timer_args, &led_timer);      // Create the timer object
}

/**
 * @brief Deinitializes the LED handler
 * 
 * - Stops and deletes the timer
 * - Turns off the LED to leave the system in a clean state
 */
void led_handler_deinit(void) {
    ESP_LOGI(TAG, "Deinitializing LED handler...");
    if (led_timer) {
        esp_timer_stop(led_timer);     // Stop timer if active
        esp_timer_delete(led_timer);   // Delete the timer object
        led_timer = NULL;              // Avoid dangling pointer
    }
    led_off();                         // Turn LED off physically
}

// === Apply Pattern ===

/**
 * @brief Applies a predefined LED blinking pattern
 *
 * Based on the selected pattern, this function:
 * - Resets all internal state and counters
 * - Updates the `current_timing` struct with ON/OFF durations
 * - Starts the LED timer to begin toggling
 *
 * The function handles different logic branches depending on whether the pattern:
 * - Is continuous (DEV_MODE, TRANSFER_COMPLETE, TETHERED)
 * - Has a stopping condition (HALTED_ENTRY, UNTETHERED)
 * - Uses burst logic (OPERATIONAL, RTV_ACTIVE)
 *
 * @param pattern The LED pattern to apply (defined in `led_pattern_t` enum)
 */
void led_apply_pattern(led_pattern_t pattern) {
    ESP_LOGI(TAG, "Applying LED pattern: %d", pattern);

    // Stop any existing timer activity
    if (led_timer) esp_timer_stop(led_timer);

    // Reset all LED handler state
    led_state = false;
    burst = (burst_state_t){0};
    halted_mode_active = false;
    halted_blink_count = 0;
    untethered_mode_active = false;
    untethered_blink_count = 0;
    current_pattern = pattern;  // Track current pattern globally

    // === Pattern selection ===
    switch (pattern) {

        case LED_PATTERN_DEV_MODE:
            ESP_LOGI(TAG, "[Pattern] DEV_MODE → static LED - constant ON");
            led_set_static(true);  // No timer used
            break;

        case LED_PATTERN_OPERATIONAL:
            ESP_LOGI(TAG, "[Pattern] OPERATIONAL → 1Hz blinking (500ms ON/OFF)");
            current_timing = (led_timing_t){ 500000, 500000 };
            esp_timer_start_once(led_timer, current_timing.off_us);
            break;

        case LED_PATTERN_RTV_ACTIVE:
            ESP_LOGI(TAG, "[Pattern] RTV_ACTIVE → 5x blinks @ 100Hz, then 500ms pause");
            current_timing = (led_timing_t){ RTV_BLINK_ON_US, RTV_BLINK_OFF_US };
            burst.active = true;
            burst.count = 0;
            esp_timer_start_once(led_timer, current_timing.off_us);
            break;

        case LED_PATTERN_HALTED_ENTRY:
            ESP_LOGI(TAG, "[Pattern] HALTED_ENTRY → 2Hz (250ms ON/OFF) for 5s, then OFF");
            current_timing = (led_timing_t){ HALTED_BLINK_ON_US, HALTED_BLINK_OFF_US };
            halted_mode_active = true;
            halted_blink_count = 0;
            esp_timer_start_once(led_timer, current_timing.off_us);
            break;

        case LED_PATTERN_TRANSFER_COMPLETE:
            ESP_LOGI(TAG, "[Pattern] TRANSFER_COMPLETE → 1Hz (500ms ON, 1s OFF)");
            current_timing = (led_timing_t){ TRANSFER_BLINK_ON_US, TRANSFER_BLINK_OFF_US };
            esp_timer_start_once(led_timer, current_timing.off_us);
            break;

        case LED_PATTERN_TETHERED:
            ESP_LOGI(TAG, "[Pattern] TETHERED → 0.5Hz (1s ON, 1s OFF)");
            current_timing = (led_timing_t){ TETHERED_BLINK_ON_US, TETHERED_BLINK_OFF_US };
            esp_timer_start_once(led_timer, current_timing.off_us);
            break;

        case LED_PATTERN_UNTETHERED:
            ESP_LOGI(TAG, "[Pattern] UNTETHERED → 10x 250ms blinks, then 500ms pause");
            current_timing = (led_timing_t){ UNTETHERED_BURST_ON_US, UNTETHERED_BURST_OFF_US };
            untethered_mode_active = true;
            untethered_blink_count = 0;
            esp_timer_start_once(led_timer, current_timing.off_us);
            break;

        default:
            ESP_LOGW(TAG, "[Pattern] Unknown pattern! Turning LED OFF.");
            led_off();
            break;
    }
}


// === Utility API: Custom Blink Pattern ===

/**
 * @brief Configure a custom LED blinking pattern at runtime
 *
 * This function lets you define any blink frequency and duty cycle without relying on
 * predefined `led_pattern_t` enums. It updates the internal `current_timing` struct
 * and restarts the LED timer with your chosen parameters.
 *
 * @param frequency_hz        Frequency in Hertz (e.g., 2.0 → 2 toggles per second)
 * @param duty_cycle_percent  Duty cycle in % (e.g., 50.0 = 50% ON, 50% OFF)
 *
 * Example:
 *     led_blink(2.0f, 25.0f);  // 2Hz blinking with 25% ON time (125ms ON, 375ms OFF)
 */
void led_blink(float frequency_hz, float duty_cycle_percent) {
    // Convert Hz → period in microseconds (us)
    uint32_t period_us = (uint32_t)(1000000.0f / frequency_hz);  

    // Calculate ON time based on duty cycle
    uint32_t on_us = (uint32_t)(period_us * (duty_cycle_percent / 100.0f));

    // Remaining time becomes OFF period
    uint32_t off_us = period_us - on_us;

    // Save timing to internal struct
    current_timing.on_us = on_us;
    current_timing.off_us = off_us;

    // Reset LED timer with new OFF delay (OFF comes first in this framework)
    if (led_timer) esp_timer_stop(led_timer);
    esp_timer_start_once(led_timer, current_timing.off_us);
}


// === Pulse and Fade (Unimplemented Placeholders) ===

/**
 * @brief Create a LED pulsing effect (TO BE IMPLEMENTED)
 *
 * This function is a placeholder for future enhancement.
 * You would use the LEDC (LED Controller) module or a PWM driver
 * to gradually increase and decrease LED brightness.
 *
 * @param frequency_hz Frequency of pulse cycles (e.g., breathing rate)
 */
void led_pulse(float frequency_hz) {
    // [TODO] Implement pulse using LEDC fade or sine wave modulation
    ESP_LOGW(TAG, "Pulse pattern not implemented.");
}

/**
 * @brief Fade the LED brightness over a specific time (TO BE IMPLEMENTED)
 *
 * This function is another placeholder for future fade effects.
 * It will likely use `ledc_set_fade_with_time()` and `ledc_fade_start()` from ESP-IDF.
 *
 * @param duration_ms Time in milliseconds to complete the fade
 */
void led_fade(uint32_t duration_ms) {
    // [TODO] Implement gradual brightness fade over time
    ESP_LOGW(TAG, "Fade pattern not implemented.");
}

// === Periodic Tick (for future use) ===

/**
 * @brief Optional periodic tick for LED logic (currently unused)
 *
 * Can be hooked into a system tick or FreeRTOS timer in the future
 * to track animations, advanced patterns, or timeouts.
 */
void led_handler_tick(void) {
    // No-op for now
}



void led_debug_status(void) {
    ESP_LOGI(TAG, "=== LED DEBUG STATUS ===");

    if (current_pattern != LED_PATTERN_DEV_MODE) {
        ESP_LOGW(TAG, "Debug status is only available in DEV_MODE.");
        return;
    }

    // Basic LED state info
    ESP_LOGI(TAG, "LED physical state: %s", gpio_get_level(GPIO_LED) ? "ON" : "OFF");
    ESP_LOGI(TAG, "Current timing → ON: %u us | OFF: %u us", current_timing.on_us, current_timing.off_us);

    // Burst Mode
    if (burst.active) {
        const char* label = (current_pattern == LED_PATTERN_RTV_ACTIVE) ? "RTV" : "OPERATIONAL";
        int max_cycles = (current_pattern == LED_PATTERN_RTV_ACTIVE) ? RTV_BURST_CYCLES : BURST_MAX_CYCLES;
        ESP_LOGI(TAG, "[Burst - %s] ACTIVE → Cycles: %d / %d", label, burst.count, max_cycles);
    } else {
        ESP_LOGI(TAG, "[Burst] INACTIVE");
    }

    // HALTED Mode
    if (halted_mode_active) {
        ESP_LOGI(TAG, "[HALTED Entry] ACTIVE → Blinks: %d / %d", halted_blink_count, HALTED_MAX_CYCLES);
    } else {
        ESP_LOGI(TAG, "[HALTED Entry] INACTIVE");
    }

    // UNTETHERED Mode
    if (untethered_mode_active) {
        ESP_LOGI(TAG, "[UNTETHERED] ACTIVE → Bursts: %d / %d", untethered_blink_count, UNTETHERED_BURST_CYCLES);
    } else {
        ESP_LOGI(TAG, "[UNTETHERED] INACTIVE");
    }

    // Timer validity
    if (led_timer) {
        ESP_LOGI(TAG, "Timer: VALID (callback state not tracked here)");
    } else {
        ESP_LOGW(TAG, "Timer: NULL (not initialized?)");
    }

    ESP_LOGI(TAG, "=======================================");
}
