idf_component_register(
    SRCS
        main.c
        state_machine.c
        led_handler.c
        security.c
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        cli_handler
        esp_timer
        esp_driver_gpio
        esp_driver_uart
        nvs_flash
        console
)
