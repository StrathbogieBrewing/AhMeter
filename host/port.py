import time
import serial
import serial.tools.list_ports
import logging


def find(serial_device):
    device_list = serial.tools.list_ports.comports()
    device_string = ''
    for device in device_list:
        if device.product == serial_device and device.manufacturer == 'Plasmatronics':
            if device_string == '':
                device_string = device.device
            else:
                raise Exception('Found Multiple Devices "' + serial_device + '"')
    if device_string == '':
        raise Exception(('Could not Find Device "' + serial_device + '"'))
    return device_string

class port:
    def __init__(self, serial_device):
        self.device = serial_device
        serialPort = find(serial_device)
        logging.debug('Using ' + serialPort + ' for ' + self.device)
        self.ser = serial.Serial(
            port=serialPort,
            baudrate=9600,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE,
            parity=serial.PARITY_NONE,
            timeout=3)  # default timeout for reading response in seconds 
        self.rx_buffer = bytearray()  # Temporary storage for incoming data
        time.sleep(0.5) # allow time for initialisation of port to complete 
        self.ser.flushOutput()
        self.ser.flushInput()

    def write(self, msg):
        self.ser.flushOutput()
        self.ser.flushInput()
        self.ser.write(msg)

    def read(self):
        # global buffer
        data = self.ser.read(self.ser.in_waiting)  # Read all available bytes
        if data:
            self.rx_buffer.extend(data)  # Append new data to buffer
            while b'\x00' in self.rx_buffer:  # Check if there's a NUL-terminated string
                nul_index = self.rx_buffer.index(b'\x00') + 1
                message = self.rx_buffer[:nul_index]
                self.rx_buffer = self.rx_buffer[nul_index + 1:]  # Remove processed data
                return message  # Return the first complete message
        return None  # No complete message yet

    def close(self):
        self.ser.close()