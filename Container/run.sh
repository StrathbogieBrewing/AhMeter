#!/bin/bash
set -e

podman run -ti -v /dev/ttySTK500V2:/dev/ttySTK500V2 --group-add keep-groups -v $(pwd):/work localhost/avr-tools:1.0.0

