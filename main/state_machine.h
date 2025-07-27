/**
 * @file state_machine.h
 * @brief OptiPulse™ State Machine API
 *
 * This module manages the finite state machine (FSM) controlling
 * the core behavior of the OptiPulse™ firmware. It handles all 
 * state transitions, event processing, and runtime state tracking.
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>  ///< Enables use of bool, true, false
#include <stdint.h>   ///< Enables use of uint8_t, uint32_t, etc.

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @enum SystemState
 * @brief Logical high-level modes of system operation.
 */
typedef enum {
    STATE_DEV,         ///< Development mode: full CLI, YAML config, logging
    STATE_OPERATIONAL, ///< Normal operating mode: LED feedback, session triggers
    STATE_TETHERED,    ///< USB connected: file transfer to PC
    STATE_UNTETHERED,  ///< Wi-Fi upload mode: MQTT or HTTP to cloud/server
    STATE_RTV,         ///< Real-Time View mode: camera capture + SD logging
    STATE_HALTED       ///< HALTED mode: error recovery, secure exit only
} SystemState;

/**
 * @enum event_t
 * @brief Internal and external events that trigger state transitions.
 */
typedef enum {
    EVENT_NONE,              ///< No event
    EVENT_CLI_MAGIC_KEY,     ///< Magic key typed via CLI
    EVENT_CLI_SET_OP,        ///< CLI requested transition to OPERATIONAL
    EVENT_TIMEOUT,           ///< Timeout reached (e.g., RTV session ended)
    EVENT_RTV_ON,            ///< RTV session requested
    EVENT_RTV_OFF,           ///< RTV session stopped manually
    EVENT_TRANSFER_COMPLETE, ///< Transfer task finished successfully
    EVENT_TRANSFER_FAILED,   ///< Transfer task failed
    EVENT_ERROR              ///< System or hardware error detected
} event_t;

// ============================================================================
// API FUNCTIONS
// ============================================================================


/**
 * @brief Convert FSM enum to human-readable string
 */
const char* state_to_string(SystemState state);


/**
 * @brief Initialize the state machine.
 * 
 * Should be called at system startup.
 * Loads saved state if supported or defaults to safe mode (e.g., DEV).
 */
void state_machine_init(void);

/**
 * @brief Perform a direct transition to a new state.
 * 
 * Automatically invokes side effects like LED patterns.
 * Avoid using this outside the FSM core unless required.
 *
 * @param new_state Target state to transition into.
 */
void transition_to_state(SystemState new_state);

/**
 * @brief Returns the currently active state.
 *
 * Useful for CLI inspection, logging, or behavior decisions.
 *
 * @return Current state as a SystemState enum.
 */
SystemState get_current_state(void);

/**
 * @brief Process a single system event.
 *
 * Determines the appropriate next state based on current state and input event.
 * Typically called by CLI, timers, or peripheral modules.
 *
 * @param event The event to process.
 */
void handle_event(event_t event);

/**
 * @brief (Optional) Abstracted event posting API.
 * 
 * Future-proof entry point if event queues or RTOS messaging added later.
 * Currently wraps `handle_event()` directly.
 *
 * @param event The event to be posted.
 */
void post_event(event_t event);  // Not yet implemented — reserved for future

#ifdef __cplusplus
}
#endif

#endif // STATE_MACHINE_H
