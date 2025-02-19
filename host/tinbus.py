import cobsm, port, utils, crc, time

serial = port.port("DUSB (Dingo)")

start_time = time.time()

while(1):
    
    frame = serial.read()
    if(frame):
        decoded = cobsm.decode(frame)
        rev = bytearray()
        for b in decoded:
            rev.append(utils.rev_bits(b))
        # print(rev.hex())
        # print(crc.check(rev).hex())
        # print(rev[2:4].hex())
        i = int.from_bytes(rev[1:4], byteorder='big', signed=True)

        # print(round(i * 0.014, 3))

        ts = time.time()
        # print(round(ts-start_time, 3), round(i , 3))
        print("Current ", round(ts-start_time, 3), round(i / 414.54, 3))
        print("Voltage ", round(ts-start_time, 3), round(i / 2821.4, 3))
        start_time = ts