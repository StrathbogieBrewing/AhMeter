#!/bin/bash
set -e

arduino-cli compile --fqbn ATTinyCore:avr:attinyx313 AhMeter

# arduino-cli burn-bootloader -b ATTinyCore:avr:attinyx313 -P stk500v2 -p /dev/ttyUSB  --board-options "chip=2313"