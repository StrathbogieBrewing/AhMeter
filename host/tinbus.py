import cobsm, port, crc, time

import json

import threading
from fastapi import FastAPI


import influxdb_client, os, time
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

token = os.environ.get("INFLUXDB_TOKEN")
org = "magiltan"
url = "http://192.168.8.100:8086"

write_client = influxdb_client.InfluxDBClient(url=url, token=token, org=org)


write_api = write_client.write_api(write_options=SYNCHRONOUS)
   
# app = FastAPI()

serial = port.port("Tinbus")

# battery = json.dumps({})

def read_jsa():
    # global battery
    global serial
    global write_api
    start_time = 0
    c_amphours = 0
    i_amps = 0
    while True:
        frame = serial.read()
        if(frame):
            frame = cobsm.decode(frame)
            frame = crc.check(frame)

            if frame[0] == 0x80:  # JSA current message
                if len(frame) == 4:
                    i = int.from_bytes(frame[1:4], byteorder='big', signed=True)
                    i_amps = i / 414.54  # Calibrate measurement
                    
                    if start_time == 0:
                        start_time = time.time()
                    else:
                        ts = time.time()
                        delat_t = ts-start_time
                        start_time = ts
                        c_amphours += i_amps * delat_t / 3600

                        # battery = json.dumps({"current_a": round(i_amps, 3), "charge_ah": round(c_amphours, 3)})
                        
                        point = (
                            Point("battery")
                            
                            .field("current", i_amps)
                            .field("charge", c_amphours)
                        )
                        write_api.write(bucket="shed", org="magiltan", record=point)

                        # print("Current ", round(i_amps, 3), round(c_amphours, 3))
                        # print("Voltage ", round(ts-start_time, 3), round(i / 2821.4, 3))

                        # Start serial reading in a separate thread

threading.Thread(target=read_jsa, daemon=True).start()

while True:
    time.sleep(1)

# @app.get("/battery")
# async def get_data():
#     return battery

# if __name__ == "__main__":
#     import uvicorn
#     uvicorn.run(app, host="0.0.0.0", port=8000)