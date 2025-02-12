#!/bin/bash
set -e

podman run -ti -v /dev/ttyUSB0:/dev/ttyUSB0 --group-add keep-groups -v $(pwd):/work localhost/avr-tools:1.0.0

