import cobsm, port, crc, time

import json

import threading, sys


serial = port.port("Tinbus")

def read_jsa():
    global serial
    global write_api
    battery_time = 0
    load_time = 0
    battery_amphours = 0
    battery_amps = 0
    load_amphours = 0
    load_amps = 0
    voltage_count = 0
    # buffer = Buffer()
    points = []

    while True:
        frame = serial.read()
        if(frame):
            frame = cobsm.decode(frame)
            frame = crc.check(frame)
            if len(frame) != 0:

                if frame[0] == 0x84:  
                    if len(frame) == 4:
                        i = int.from_bytes(frame[1:4], byteorder='big', signed=True)
                        battery_volts = i / 2821
                        # points.append(Point("battery").time(time.time_ns()).field('battery_volts', round(battery_volts, 3)))
                        # buffer.row('solar',
                        #             at=TimestampNanos.now(),
                        #             columns={'battery_volts': round(battery_volts, 3)}
                        #             )

                        voltage_count += 1
                        # if voltage_count >= 30:
                            # try:
                            #     write_api.write(bucket=bucket, org=org, record=points)
                            #     points = []
                            # except Exception as e:
                            #     print(f"Failed to ingest: {e}") 

                            # voltage_count = 0     
                            # try:
                            #     with Sender.from_conf(conf) as sender:        
                            #         sender.flush(buffer)
                            # except Exception as err:
                            #     sys.stderr.write(f'Got error: {err}\n')

                        print("Battery ", round(battery_volts, 3), " V")

                if frame[0] == 0x82:  
                    if len(frame) == 4:
                        i = int.from_bytes(frame[1:4], byteorder='big', signed=True)
                        load_amps = i / 414.54  # Calibrate measurement
                        if load_time == 0:
                            load_time = time.time()
                        else:
                            ts = time.time()
                            delat_t = ts-load_time
                            load_time = ts
                            load_amphours += load_amps * delat_t / 3600
                            # points.append(Point("battery").time(time.time_ns())
                            #     .field('load_amps', round(load_amps, 3))
                            #     .field('load_amphours', round(load_amphours, 3)))
                            # buffer.row('solar',
                            #             at=TimestampNanos.now(),
                            #             columns={'load_amps': round(load_amps, 3), 'load_amphours': round(load_amphours, 3)}
                            #             )

                            print("Load ", round(load_amps, 3), " A\t", round(load_amphours, 3), " Ah")

                if frame[0] == 0x80:  # JSA current message
                    if len(frame) == 4:
                        i = int.from_bytes(frame[1:4], byteorder='big', signed=True)
                        battery_amps = i / 414.54  # Calibrate measurement
                        if battery_time == 0:
                            battery_time = time.time()
                        else:
                            ts = time.time()
                            delat_t = ts-battery_time
                            battery_time = ts
                            battery_amphours += battery_amps * delat_t / 3600
                            # points.append(Point("battery").time(time.time_ns())
                            #     .field('battery_amps', round(battery_amps, 3))
                            #     .field('battery_amphours', round(battery_amphours, 3)))

                            # buffer.row('solar',
                            #             at=TimestampNanos.now(),
                            #             columns={'battery_amps': round(battery_amps, 3), 'battery_amphours': round(battery_amphours, 3)}
                            #             )

                            print("Battery ", round(battery_amps, 3), " A\t", round(battery_amphours, 3), " Ah")
    
threading.Thread(target=read_jsa, daemon=True).start()

while True:
    time.sleep(1)
