// File: main/security.h

#ifndef SECURITY_H
#define SECURITY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize security system (load key, set level, etc.).
 */
void security_init(void);

/**
 * @brief Get current security level (0 = open, 1 = protected, 2 = locked).
 */
uint8_t security_get_level(void);

/**
 * @brief Set current security level (restricted to valid range).
 */
void security_set_level(uint8_t level);

/**
 * @brief Validate a given key against the magic unlock key.
 * @param input_key The string entered by user or system.
 * @return true if valid, false otherwise.
 */
bool security_validate_key(const char *input_key);

#ifdef __cplusplus
}
#endif

#endif // SECURITY_H
