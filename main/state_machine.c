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

