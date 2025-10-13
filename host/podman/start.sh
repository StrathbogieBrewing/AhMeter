#!/bin/bash
set -e

podman run -it -p 1880:1880 -v nodereddata:/data --group-add keep-groups --privileged --device /dev/ttyUSB0:/dev/ttyUSB0  nodered-custom:latest bash