cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Add the CLI handler manually
set(EXTRA_COMPONENT_DIRS components/cli_handler)

project(optipulse_firmware)
