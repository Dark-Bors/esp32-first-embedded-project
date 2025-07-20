// file: main/state_machine.c
// This file implements the state machine for the firmware, handling transitions
// between different operational states based on events and system conditions.
// It includes initialization, event handling, and state transitions.

#include "state_machine.h"
#include "esp_log.h"            // For ESP_LOG macros
#include "nvs_flash.h"          // TODO: Support persistent state
#include "driver/gpio.h"        // Optional: for HALTED LED pattern recovery
#include "led_handler.h"        // LED pattern driver
#include "security.h"           // Magic key, access levels
#include "config_parser.h"      // Config + magic key validation (DEV → OP)
#include "version_config.h"  // For version numbers and dev shortcut macros


// ================================
// Local Tag and State Variables
// ================================
static const char *TAG = "STATE_MACHINE";         ///< Logging tag
static SystemState current_state = STATE_DEV;     ///< Default startup state
static uint8_t security_level = 0;                ///< Placeholder (future: GPIO/role auth)

// ================================
// @brief Initialize the FSM
// ================================
void state_machine_init(void)
{
    // Future: Load previous state from NVS here

    current_state = STATE_DEV;  // Default safe state
    ESP_LOGI(TAG, "State machine initialized in DEV mode");
}

/**
 * @brief Core state machine event dispatcher.
 *
 * This function interprets incoming system events and, based on the current
 * state of the system, determines whether a transition is required.
 * Each event may be triggered by:
 *  - CLI input
 *  - Peripheral interrupt (e.g., button, timer)
 *  - System signal (e.g., error, timeout)
 *
 * Purpose:
 *  - Encapsulates state logic in one place (central FSM logic)
 *  - Makes transition rules explicit and testable
 *  - Allows branching based on internal config or security level if needed
 *
 * @param event The event to process. See `event_t` enum for available types.
 *
 * @note Future Hook: You may want to override event logic based on
 *       `FWVersion.major > 1` or a developer debug mode (e.g., "force transition").
 */
void handle_event(event_t event)
{
    ESP_LOGI(TAG, "Handling event: %d in state: %d", event, current_state);

    switch (current_state) {

        case STATE_DEV:
            // Only allow transition to OPERATIONAL if YAML + magic key are valid
            if (event == EVENT_CLI_SET_OP) {
                if (config_validate_and_unlock()) {
                    transition_to_state(STATE_OPERATIONAL);
                } else {
                    ESP_LOGW(TAG, "Validation failed: YAML or security mismatch.");
                }
            }
            // Magic key event has no effect here (unlike HALTED)
            break;

        case STATE_OPERATIONAL:
            // User or system wants to start RTV mode
            if (event == EVENT_RTV_ON) {
                transition_to_state(STATE_RTV);
            }
            // Scheduled upload or storage trigger reached
            else if (event == EVENT_TRANSFER_COMPLETE) {
                transition_to_state(STATE_UNTETHERED);
            }
            // Hardware or system failure
            else if (event == EVENT_ERROR) {
                transition_to_state(STATE_HALTED);
            }
            break;

        case STATE_RTV:
            // RTV finished due to timeout or user stopped it
            if (event == EVENT_TIMEOUT || event == EVENT_RTV_OFF) {
                transition_to_state(STATE_OPERATIONAL);
            }
            else if (event == EVENT_ERROR) {
                transition_to_state(STATE_HALTED);
            }
            break;

        case STATE_UNTETHERED:
            // Upload completed successfully
            if (event == EVENT_TRANSFER_COMPLETE) {
                transition_to_state(STATE_OPERATIONAL);
            }
            // Failed Wi-Fi, disconnection, or retry limit exceeded
            else if (event == EVENT_ERROR) {
                transition_to_state(STATE_HALTED);
            }
            break;

        case STATE_TETHERED:
            // File transfer via USB completed
            if (event == EVENT_TRANSFER_COMPLETE) {
                transition_to_state(STATE_OPERATIONAL);
            }
            // USB disconnection or serial protocol fault
            else if (event == EVENT_ERROR) {
                transition_to_state(STATE_HALTED);
            }
            break;

        case STATE_HALTED:
            // Magic key (CLI) unlocks halted mode into DEV (manual recovery)
            if (event == EVENT_CLI_MAGIC_KEY) {
                transition_to_state(STATE_DEV);
            } else {
                ESP_LOGW(TAG, "Event %d ignored in HALTED mode (locked state)", event);
            }
            break;
    }

    // NOTE:
    // Future feature toggle: If build version includes `DEV_FAST_MODE`,
    // allow override shortcuts in any state (e.g., CLI numeric triggers).
    // Controlled via `version_config.h` or NVS field.
    
    #if DEV_SHORTCUTS_ENABLED
    // Developer-only shortcut: numeric CLI triggers event/state directly
    // For example, `state 2` in CLI sends event that maps to STATE_RTV
    if (event >= 100 && event <= 105) {
        ESP_LOGW(TAG, "[DEV] Forcing transition via developer shortcut: %d", event - 100);
        transition_to_state((SystemState)(event - 100));  // Only safe if enum matches exactly
    }
#endif

}


/**
 * @brief Transition to a new system state.
 *
 * This function applies the necessary side effects for each state:
 *  - Logs the transition
 *  - Updates internal FSM state
 *  - Triggers LED feedback pattern
 *  - Prepares the system for next event cycle
 *
 * @param new_state The target state to enter.
 *
 * @note This function does NOT validate whether the transition is allowed.
 *       It assumes that decision has already been made (by `handle_event()`).
 *
 * @note Developer Feature:
 *       You may embed version-aware behavior here. For example, if
 *       `FW_VERSION.build > 100`, enable test hooks like:
 *         - auto-fake transfer completion
 *         - ignore config key check
 *         - shorter RTV sessions
 */
void transition_to_state(SystemState new_state)
{
    ESP_LOGI(TAG, "Transition: %d → %d", current_state, new_state);
    current_state = new_state;

    // Optional: In future, add hooks like `on_state_entry(new_state)` here

    switch (new_state) {
        case STATE_DEV:
            led_apply_pattern(LED_PATTERN_DEV_MODE);  // Constant ON or CLI-blink
            break;

        case STATE_OPERATIONAL:
            led_apply_pattern(LED_PATTERN_OPERATIONAL);  // Periodic burst pattern
            break;

        case STATE_TETHERED:
            led_apply_pattern(LED_PATTERN_TETHERED);  // Slow blink (USB connected)
            break;

        case STATE_UNTETHERED:
            led_apply_pattern(LED_PATTERN_UNTETHERED);  // Burst-pause Wi-Fi transfer
            break;

        case STATE_RTV:
            led_apply_pattern(LED_PATTERN_RTV_ACTIVE);  // Fast burst while capturing
            break;

        case STATE_HALTED:
            led_apply_pattern(LED_PATTERN_HALTED_ENTRY);  // 5Hz blinking, then OFF
            break;

        default:
            ESP_LOGW(TAG, "Unknown state %d — LED forced OFF", new_state);
            led_off();
            break;
    }

    // NOTE:
    // Consider broadcasting state via MQTT or logging to SD card here.
    // You could also call:
    //   state_hooks_notify(new_state); // for external modules to respond
}


// ================================
// @brief Get current active state
// ================================
SystemState get_current_state(void)
{
    return current_state;
}
