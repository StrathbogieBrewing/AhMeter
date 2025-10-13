import { Port } from './node_modules/serial-port.js';

async function runTests() {
    console.log('=== Serial Port Test ===\n');
    
    let port;
    try {
        // Test 1: Create and initialize port
        console.log('1. Creating port instance...');
        port = new Port('TestDevice');
        await port.init();
        
        // Test 2: Set up message handler
        console.log('\n2. Setting up message handler...');
        port.onMessage((message) => {
            console.log('📨 Message received:', message);
            console.log('Message as string:', message.toString('ascii'));
        });
        
        // Test 3: Write data
        console.log('\n3. Writing test data...');
        const testData = Buffer.from([0x41, 0x42, 0x43, 0x00]); // "ABC" with null terminator
        await port.write(testData);
        
        // Test 4: Wait for responses
        console.log('\n4. Waiting for responses (5 seconds)...');
        await new Promise(resolve => setTimeout(resolve, 5000));
        
        // Test 5: Write more data
        console.log('\n5. Writing more test data...');
        const moreData = Buffer.from([0x44, 0x45, 0x46, 0x00]); // "DEF" with null terminator
        await port.write(moreData);
        
        // Wait a bit more
        await new Promise(resolve => setTimeout(resolve, 2000));
        
    } catch (error) {
        console.error('❌ Test failed:', error);
    } finally {
        // Test 6: Clean up
        console.log('\n6. Cleaning up...');
        if (port) {
            await port.close();
        }
    }
    
    console.log('\n=== Test completed ===');
}

// Run the tests
runTests();