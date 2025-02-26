cmake_minimum_required(VERSION 3.18)

set(AVR_UPLOADTOOL avrdude)
set(AVR_UPLOADTOOL_BAUDRATE 19200)
set(AVR_UPLOADTOOL_PORT /dev/ttySTK500V2)
set(AVR_PROGRAMMER stk500v2)

set(AVR_MCU_TYPE atmega324p)
set(AVR_MCU_SPEED "3686400UL")

set(AVR_L_FUSE 0xEC)
set(AVR_H_FUSE 0xD9)
set(AVR_E_FUSE 0xFE)

add_definitions("-Os")

include(../cmake/avr-gcc.cmake)

project(gateway C)

include(../cmake/check-git.cmake)
CheckGitSetup()

include_directories(../lib/timer)
include_directories(../lib/uart)
include_directories(../lib/cobsm)

add_avr_executable(${PROJECT_NAME} 
    main.c
    uart0.c
    tinbus.c

    ../lib/uart/uart.c
    ../lib/uart/uart1.c
    ../lib/timer/timer.c
    ../lib/cobsm/cobsm.c

    ${CMAKE_BINARY_DIR}/generated/git_version.c
)

