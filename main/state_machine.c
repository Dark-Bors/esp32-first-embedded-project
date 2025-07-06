<<<<<<< HEAD
#include "state_machine.h"
#include "esp_log.h"     // For logging
#include "nvs_flash.h"   // Future: to persist state
#include "driver/gpio.h" // Optional: for LED indication

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
// Transition to a new state with side effects
// ============================================
void transition_to_state(SystemState new_state)
{
    // TODO: Add validation if needed
    // TODO: Call per-state entry behavior (LED pattern, etc.)

    // Log state transition
    ESP_LOGI(TAG, "State change: %d -> %d", current_state, new_state);

    current_state = new_state;
}

// ==============================
// Return current state at runtime
// ==============================
SystemState get_current_state(void)
{
    return current_state;
}
=======
// state_machine.c

#include "state_machine.h" // Header file for the state machine
#include "esp_log.h" // ESP-IDF logging library for ESP_LOGI/WARNING/ERROR

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

>>>>>>> recovery/led-patterns
