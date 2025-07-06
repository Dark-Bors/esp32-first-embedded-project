// File: main/led_handler.h
// ==========================================================================================
// @file led_handler.h
// @brief LED handler interface for pattern control and GPIO management on ESP32
//
// This file declares the public API for controlling LED behavior, including static control,
// burst patterns, and special modes like Tethered, Untethered, and HALTED.
// It uses the ESP-IDF GPIO and timer framework and is designed for integration into
// embedded systems with real-time feedback requirements.
// ==========================================================================================

#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    uint32_t on_us;
    uint32_t off_us;
} led_timing_t;

typedef struct {
    bool active;
    uint16_t count;
} burst_state_t;


/**
 * @enum led_pattern_t
 * @brief LED blinking patterns for system state indication.
 */
typedef enum {
    LED_PATTERN_DEV_MODE,           /**< Development mode: 1Hz continuous blinking */
    LED_PATTERN_OPERATIONAL,        /**< Operational state: 2Hz bursts with pauses */
    LED_PATTERN_RTV_ACTIVE,         /**< Real-Time Video active: burst mode pattern */
    LED_PATTERN_TETHERED,           /**< Tethered transfer: 0.5Hz blinking */
    LED_PATTERN_UNTETHERED,         /**< Untethered transfer: 10x short bursts + pause */
    LED_PATTERN_HALTED_ENTRY,       /**< HALTED state: 5Hz blink for 2 seconds */
    LED_PATTERN_TRANSFER_COMPLETE   /**< Transfer done: 1Hz blinking until change */
} led_pattern_t;

/**
 * @brief Initialize the LED handler and configure the GPIO and timer.
 */
void led_handler_init(void);

/**
 * @brief Deinitialize the LED handler and release GPIO/timer resources.
 */
void led_handler_deinit(void);

/**
 * @brief Optional function to advance internal LED handling (e.g., fading).
 * Can be used in future tick-based logic.
 */
void led_handler_tick(void);

/**
 * @brief Apply a predefined LED blinking pattern based on system state.
 * 
 * @param pattern The desired pattern to apply from led_pattern_t.
 */
void led_apply_pattern(led_pattern_t pattern);

/**
 * @brief Immediately turn ON the LED (no timer logic).
 */
void led_on(void);

/**
 * @brief Immediately turn OFF the LED (no timer logic).
 */
void led_off(void);

/**
 * @brief Set the LED to a static state (ON or OFF), bypassing pattern logic.
 * 
 * @param on Pass true to turn ON, false to turn OFF.
 */
void led_set_static(bool on);

/**
 * @brief Blink LED with custom frequency and duty cycle.
 * 
 * @param frequency_hz        Frequency in Hz (e.g., 1.0 for 1Hz)
 * @param duty_cycle_percent  Duty cycle in percent (e.g., 50.0 for 50%)
 */
void led_blink(float frequency_hz, float duty_cycle_percent);

/**
 * @brief Placeholder for pulse mode (uses PWM fading or toggling).
 * 
 * @param frequency_hz Pulse frequency in Hz.
 */
void led_pulse(float frequency_hz);

/**
 * @brief Placeholder for fading effect (e.g., fade in/out using LEDC).
 * 
 * @param duration_ms Duration of fade in milliseconds.
 */
void led_fade(uint32_t duration_ms);

/**
 * @brief Print debug status of the LED handler.
 * This function provides detailed information about the current LED state,
 * including active patterns, burst states, and timer validity.
 * It is only available in DEV_MODE.
 */
void led_debug_status(void);


#ifdef __cplusplus
}
#endif

#endif // LED_HANDLER_H
