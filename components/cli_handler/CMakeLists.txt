set(COMPONENT_KCONFIG Kconfig)

idf_component_register(
    SRCS "cli_handler_usb.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES console main
)
