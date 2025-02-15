# Setup

To enable USB device passthrough install crun

```sudo apt install crun```

To use avr programmer add udev rule to host

```
# Create sysmlinks to USB serial devices
SUBSYSTEMS=="usb", ACTION=="add", ATTRS{idVendor}=="0403", ATTRS{product}=="stk500v2", MODE="660", GROUP="dialout", SYMLINK+="ttySTK500V2"
```