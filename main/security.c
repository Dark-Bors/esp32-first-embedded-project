// File: main/security.c

#include "security.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "SECURITY";

#define MAGIC_KEY "open-sesame"

static uint8_t current_level = 0; // Default: open

void security_init(void) {
    ESP_LOGI(TAG, "Security system initialized at level %d", current_level);
}

uint8_t security_get_level(void) {
    return current_level;
}

void security_set_level(uint8_t level) {
    if (level <= 2) {
        current_level = level;
        ESP_LOGI(TAG, "Security level set to %d", level);
    } else {
        ESP_LOGW(TAG, "Invalid security level: %d", level);
    }
}

bool security_validate_key(const char *input_key) {
    return (strcmp(input_key, MAGIC_KEY) == 0);
}
