import cobsm, port, utils

serial = port.port("DUSB (Dingo)")

while(1):
    frame = serial.read()
    if(frame):
        print(frame.hex())
        decoded = cobsm.decode(frame)
        rev = bytearray()
        for b in decoded:
            rev.append(utils.rev_bits(b))
        print(rev.hex())
