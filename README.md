# AhMeter
Amp Hour Meter

socat  /dev/ttyUSB2,b9600,raw,echo=0 - | hd

# InfluxDB Token

hjAFrA0r_I72tEuR66HJxUnF1vbkfLr_OAqH_WEoWqdZ_c0xu-PmmW5-V_4OYhdRslDjpqLS7dcmit-RS89qEQ==

run \
    -p 8086:8086 \
    -v "$PWD/data:/var/lib/influxdb2" \
    -v "$PWD/config:/etc/influxdb2" \
    docker.io/influxdb:2
