/**
 * @file version_config.h
 * @brief Firmware version configuration and feature toggles.
 *
 * Defines the OptiPulse™ firmware version numbers and optional debug-mode macros
 * based on MAJOR version (used as a tier indicator). Enables developer/test
 * features in builds where MAJOR > 100.
 */

#ifndef VERSION_CONFIG_H
#define VERSION_CONFIG_H

// ===============================
// Firmware Version Info
// ===============================
#define FW_VERSION_MAJOR   101   ///< Use >100 to enable developer/test features
#define FW_VERSION_MINOR   4     ///< Minor update or feature milestone
#define FW_VERSION_BUILD   0     ///< Internal build number or commit-based tag

// ===============================
// Formatted Version String
// ===============================
#define FW_VERSION_STRING  "v101.4.0-dev"  ///< Displayed in main banner and logs

/**
 * @brief Enables CLI developer shortcuts or debug hooks.
 *
 * If MAJOR > 100, dev-mode features are enabled:
 *  - Numeric CLI override to set system state
 *  - Skip magic key in `STATE_DEV`
 *  - Trigger mock events via UART for testing
 * 
 * This is useful during bring-up, factory t*
*/