// File: main/state_machine.c


#include "state_machine.h"  // Header for the state machine
#include "esp_log.h"        // For logging
#include "nvs_flash.h"      // Future: to persist state
#include "driver/gpio.h"    // Optional: for LED indication
#include "led_handler.h"    // For LED patterns
#include "security.h"   // For security level management (future use)

// =================================
// Static variable to track the state
// =================================
static SystemState current_state;  // Global only to this .c file

// =================================
// Logging tag for ESP_LOG macros
// =================================
static const char *TAG = "STATE_MACHINE";

// =====================================
// Initialize the state machine on boot
// =====================================
void state_machine_init(void)
{
    // TODO: Load from NVS in future
    // Placeholder logic:
    current_state = STATE_DEV;

    // Optional: Log this event
    ESP_LOGI(TAG, "State machine initialized in DEV mode");
}
// ============================================
// Handle events and transition states
// ============================================

void handle_event(event_t event) {
    ESP_LOGI(TAG, "Handling event: %d in state: %d", event, current_state);

    switch (current_state) {

        case STATE_DEV:
            if (event == EVENT_CLI_SET_OP) {
                transition_to_state(STATE_OPERATIONAL);
            } else if (event == EVENT_CLI_MAGIC_KEY) {
                ESP_LOGI(TAG, "Magic key received, staying in DEV");
            }
            break;

        case STATE_OPERATIONAL:
            if (event == EVENT_RTV_ON) {
                transition_to_state(STATE_RTV);
            } else if (event == EVENT_TRANSFER_COMPLETE) {
                transition_to_state(STATE_UNTETHERED);
            }
            break;

        case STATE_RTV:
            if (event == EVENT_RTV_OFF || event == EVENT_TIMEOUT) {
                transition_to_state(STATE_OPERATIONAL);
            }
            break;

        case STATE_UNTETHERED:
            if (event == EVENT_TRANSFER_COMPLETE) {
                transition_to_state(STATE_OPERATIONAL);
            }
            break;

        case STATE_TETHERED:
            if (event == EVENT_TRANSFER_COMPLETE) {
                transition_to_state(STATE_OPERATIONAL);
            }
            break;

        case STATE_HALTED:
            if (event == EVENT_CLI_MAGIC_KEY) {
                transition_to_state(STATE_DEV);
            }
            break;
    }
}


// ============================================
// Transition to a new state with side effects
// ============================================
void transition_to_state(SystemState new_state)
{
    ESP_LOGI(TAG, "State change: %d -> %d", current_state, new_state);
    current_state = new_state;

    switch (new_state) {
        case STATE_DEV:
            led_apply_pattern(LED_PATTERN_DEV_MODE);
            break;
        case STATE_OPERATIONAL:
            led_apply_pattern(LED_PATTERN_OPERATIONAL);
            break;
        case STATE_TETHERED:
            led_apply_pattern(LED_PATTERN_TETHERED);
            break;
        case STATE_UNTETHERED:
            led_apply_pattern(LED_PATTERN_UNTETHERED);
            break;
        case STATE_RTV:
            led_apply_pattern(LED_PATTERN_RTV_ACTIVE);
            break;
        case STATE_HALTED:
            led_apply_pattern(LED_PATTERN_HALTED_ENTRY);
            break;
        default:
            led_off();  // safety default
            break;
    }
}

// ==============================
// Return current state at runtime
// ==============================
SystemState get_current_state(void)
{
    return current_state;
}

// state_machine.c

static const char *TAG = "STATE_MACHINE"; // Log tag for this module

// ==========================================
// Internal State Variables (invisible to .h)
// ==========================================
static SystemState current_state = STATE_DEV; // Default state at startup
static uint8_t security_level = 0; // Default security level

// ===============================
// Function Implementations
// ===============================

// Return the current state
SystemState get_current_state(void) {
    return current_state;
}

