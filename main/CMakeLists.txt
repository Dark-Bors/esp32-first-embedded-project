idf_component_register(SRCS "main.c" "led_handler.c" "led_handler.h"
                      INCLUDE_DIRS ".")
idf_component_register(SRCS "main.c"
                       "led_handler.c"
                       "state_machine.c"
                       INCLUDE_DIRS ".")
