#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
// File: main/state_machine.h

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
#include <stdbool.h>  // Enables use of `bool`, `true`, `false`
#include <stdint.h> // Enables use of `uint8_t`, `uint32_t`, etc.

#ifdef __cplusplus
extern "C" {
#endif

// ===============================
// ENUM: Define all system states
// ===============================
typedef enum {
    STATE_DEV,         // Development mode: CLI tools, config, debug
    STATE_OPERATIONAL, // Normal operation: follows behavior in YAML
    STATE_TETHERED,    // Connected via USB OTG for transfer
    STATE_UNTETHERED,  // Transfer via Wi-Fi
    STATE_RTV,         // Real-Time View session (live camera)
    STATE_HALTED       // Fatal error or intentional halt
} SystemState;

// ===============================
// ENUM: Define all system events
// ===============================
typedef enum {
    EVENT_NONE,             // No new event
    EVENT_CLI_MAGIC_KEY,    // User typed the magic key
    EVENT_CLI_SET_OP,       // User asked to go to operational
    EVENT_TIMEOUT,          // A timeout occurred
    EVENT_RTV_ON,           // RTV triggered (via CLI or internal logic)
    EVENT_RTV_OFF,          // RTV timed out or stopped
    EVENT_TRANSFER_COMPLETE,// File/data transfer completed
    EVENT_TRANSFER_FAILED,  // Transfer failed (Wi-Fi/USB)
    EVENT_ERROR             // Generic error
} event_t;


// =====================================
// Function Prototypes (APIs we expose)
// =====================================

/**
 * @brief Initialize the state machine. 
 *        Should be called at system startup.
 *        Will read from NVS (non-volatile storage) if implemented,
 *        or start in a default safe state like DEV or HALTED.
 */
void state_machine_init(void);

/**
 * @brief Perform a transition to a new system state.
 *        Will include logging, LED signaling, and future behavior hooks.
 */
void transition_to_state(SystemState new_state);

/**
 * @brief Returns the current system state.
 */
