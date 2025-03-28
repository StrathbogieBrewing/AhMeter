# AhMeter
Amp Hour Meter

socat  /dev/ttyUSB2,b9600,raw,echo=0 - | hd

# InfluxDB Token

6Ahml_tO3rWw207013Vn1dLBE9q6zLnxtFPGbIdheXnl447NNdAPg6twMuLQoeRScSzcian5VScLptRNXQsgng==

podman run  -d -p 192.168.6.1:8086:8086 --network=host -v "/root/podman/influx/data:/var/lib/influxdb2" -v "/root/podman/influx/config:/etc/influxdb2" docker.io/influxdb:2

