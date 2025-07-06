// File: main/led_handler.h

#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include <stdint.h>
#include <stdbool.h>  // Enables use of `bool`, `true`, `false`

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Available LED patterns for system modes and events
 */
typedef enum {
    LED_PATTERN_DEV_MODE,         ///< Blink 1Hz, 50% DC
    LED_PATTERN_OPERATIONAL,      ///< Blink per YAML (e.g. 2Hz)
    LED_PATTERN_RTV_ACTIVE,       ///< 5x Blink @ 100Hz + 50ms pause
    LED_PATTERN_TETHERED,         ///< Slow blink or pulse (≤1Hz)
    LED_PATTERN_UNTETHERED,       ///< Pulse / fade pattern
    LED_PATTERN_HALTED_ENTRY,     ///< Blink 5Hz for 2s, then OFF
    LED_PATTERN_TRANSFER_COMPLETE ///< Blink 1Hz until next transition
} led_pattern_t;

/**
 * @brief Initialize LED subsystem (GPIO + timers)
 */
void led_handler_init(void);
/**
 * @brief Deinitialize LED subsystem (stop timers, reset GPIO)
 */ 
void led_handler_deinit(void);

/**
 * @brief Must be called periodically for internal pattern timing
 */
void led_handler_tick(void);

/**
 * @brief Apply high-level pattern based on system mode or event
 * 
 * @param pattern One of the LED_PATTERN_* values
 */
void led_apply_pattern(led_pattern_t pattern);

/**
 * @brief Turn the LED ON (static)
 */
void led_on(void);

/**
 * @brief Turn the LED OFF (static)
 */
void led_off(void);

/**
 * @brief Set LED to a static state (ON or OFF)
 * 
 * @param on true for ON, false for OFF
 */
void led_set_static(bool on);

/**
 * @brief Make the LED blink
 * 
 * @param frequency_hz Blink frequency in Hz
 * @param duty_cycle_percent Percentage of ON time (0–100)
 */
void led_blink(float frequency_hz, float duty_cycle_percent);

/**
 * @brief Pulse the LED at a given frequency (on-off-on pattern)
 * 
 * @param frequency_hz Pulse rate in Hz
 */
void led_pulse(float frequency_hz);

/**
 * @brief Run a one-shot fade in/out animation
 * 
 * @param duration_ms Total duration in milliseconds
 */
void led_fade(uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif // LED_HANDLER_H
