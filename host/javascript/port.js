const { SerialPort } = require('serialport');
const { list } = require('@serialport/list');

async function find(serialDevice) {
    const deviceList = await list();
    let deviceString = '';
    let foundCount = 0;
    
    for (const device of deviceList) {
        if (device.product === serialDevice && device.manufacturer === 'Plasmatronics') {
            if (deviceString === '') {
                deviceString = device.path;
                foundCount++;
            } else {
                throw new Error(`Found Multiple Devices "${serialDevice}"`);
            }
        }
    }
    
    if (deviceString === '') {
        throw new Error(`Could not Find Device "${serialDevice}"`);
    }
    return deviceString;
}

class Port {
    constructor(serialDevice) {
        this.device = serialDevice;
        this.rxBuffer = Buffer.alloc(0); // Temporary storage for incoming data
    }

    async init() {
        const serialPortPath = await find(this.device);
        console.debug(`Using ${serialPortPath} for ${this.device}`);
        
        this.ser = new SerialPort({
            path: serialPortPath,
            baudRate: 9600,
            dataBits: 8,
            stopBits: 1,
            parity: 'none',
            autoOpen: false
        });

        // Set up event handlers
        this.ser.on('open', () => {
            console.debug('Serial port opened');
        });

        this.ser.on('error', (err) => {
            console.error('Serial port error:', err);
        });

        this.ser.on('data', (data) => {
            this.handleData(data);
        });

        // Open the port
        await new Promise((resolve, reject) => {
            this.ser.open((err) => {
                if (err) reject(err);
                else resolve();
            });
        });

        // Allow time for initialization
        await new Promise(resolve => setTimeout(resolve, 500));
        
        await this.flush();
        return this;
    }

    async flush() {
        return new Promise((resolve) => {
            this.ser.flush((err) => {
                if (err) console.error('Flush error:', err);
                resolve();
            });
        });
    }

    async write(msg) {
        await this.flush();
        
        return new Promise((resolve, reject) => {
            this.ser.write(msg, (err) => {
                if (err) reject(err);
                else resolve();
            });
        });
    }

    handleData(data) {
        // Append new data to buffer
        this.rxBuffer = Buffer.concat([this.rxBuffer, data]);
        
        // Check for NUL-terminated strings
        const nulIndex = this.rxBuffer.indexOf(0x00);
        if (nulIndex !== -1) {
            // Extract message including the NUL terminator
            const message = this.rxBuffer.subarray(0, nulIndex + 1);
            // Remove processed data from buffer
            this.rxBuffer = this.rxBuffer.subarray(nulIndex + 1);
            
            // Emit the message (you might want to use events or callbacks)
            if (this.onMessage) {
                this.onMessage(message);
            }
        }
    }

    // Alternative read method that returns a promise
    read(timeout = 3000) {
        return new Promise((resolve, reject) => {
            const timeoutId = setTimeout(() => {
                reject(new Error('Read timeout'));
            }, timeout);

            const messageHandler = (message) => {
                clearTimeout(timeoutId);
                this.removeListener('message', messageHandler);
                resolve(message);
            };

            this.on('message', messageHandler);
        });
    }

    // Event emitter methods (simplified)
    on(event, callback) {
        if (event === 'message') {
            this.onMessage = callback;
        }
    }

    removeListener(event, callback) {
        if (event === 'message' && this.onMessage === callback) {
            this.onMessage = null;
        }
    }

    async close() {
        return new Promise((resolve) => {
            this.ser.close((err) => {
                if (err) console.error('Close error:', err);
                resolve();
            });
        });
    }
}

// Usage example:
async function main() {
    try {
        const port = new Port('YourDeviceName');
        await port.init();
        
        // Set up message handler
        port.on('message', (message) => {
            console.log('Received message:', message);
        });
        
        // Write data
        await port.write(Buffer.from([0x01, 0x02, 0x03]));
        
        // Read with timeout
        try {
            const response = await port.read(5000);
            console.log('Response:', response);
        } catch (err) {
            console.log('No response received within timeout');
        }
        
        await port.close();
    } catch (err) {
        console.error('Error:', err);
    }
}

module.exports = { Port, find };