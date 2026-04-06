/*
  RadioLib SX126x Blocking Receive Example

  This example listens for LoRa transmissions using SX126x Lora modules.
  To successfully receive data, the following settings have to be the same
  on both transmitter and receiver:
  - carrier frequency
  - bandwidth
  - spreading factor
  - coding rate
  - sync word
  - preamble length

  Other modules from SX126x family can also be used.

Using blocking receive is not recommended, as it will lead
to significant amount of timeouts, inefficient use of processor
time and can some miss packets!
Instead, interrupt receive is recommended.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

#include <NetworkClient.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>

const char *ssid = "EFence";
const char *password = "";
// unsigned int localPort = 2255;  // Local port to listen on (optional for sender)
unsigned int remotePort = 2255; // Remote port to send to
const char *broadcastAddress = "255.255.255.255";

WiFiUDP Udp;

// include the library
#include <RadioLib.h>

// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9

// From
// https://github.com/Seeed-Studio/one_channel_hub/components/smtc_ral/bsp/sx126x/seeed_xiao_esp32s3_devkit_sx1262.c
SX1262 radio = new Module(41, 39, 42, 40);

//   RADIO_SPI.begin(10, 6, 7);

// or detect the pinout automatically using RadioBoards
// https://github.com/radiolib-org/RadioBoards
/*
#define RADIO_BOARD_AUTO
#include <RadioBoards.h>
Radio radio = new RadioModule();
*/

#define BUFFER_SIZE 256

volatile bool operationDone = false;
bool transmitFlag = false;
int transmissionState = RADIOLIB_ERR_NONE;

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void) {
    // we sent or received a packet, set the flag
    operationDone = true;
}

void setup() {
    Serial.begin(115200);

    // initialize SX1262 with XIAO_ESP32S3_DEVKIT_SX1262 shield settings
    // Serial.print(F("Initializing ... "));
    int state =
        radio.begin(916.5, 125.0, 10, 8, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 14, 8, 3.0, true);
    if (state == RADIOLIB_ERR_NONE) {
        // Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true) {
            delay(10);
        }
    }

    radio.setDio1Action(setFlag);

    radio.startReceive();

    //   if (!WiFi.softAP(ssid, password)) {
    //     log_e("Soft AP creation failed.");
    //     while (1)
    //       ;
    //   }
    //   IPAddress myIP = WiFi.softAPIP();
    //   Serial.print("AP IP address: ");
    //   Serial.println(myIP);
}

void printDeltaT(void) {
    static long unsigned int t_us = micros();
    long unsigned int now_us = micros();
    Serial.printf("%6lu ", (now_us - t_us) / 1000UL);
    t_us = now_us;
}

void printHex(uint8_t *buffer, size_t size) {
    Serial.printf("Data[%u]: < ", size);
    for (size_t i = 0; i < size; i++) {
        Serial.printf("%2.2X ", buffer[i]);
    }
    Serial.printf(">\n");
}

int hexDump(char *buffer, uint8_t *data, size_t size) {
    char *write_pointer = buffer;
    write_pointer += sprintf(write_pointer, "Data[%u]: < ", size);
    for (size_t i = 0; i < size; i++) {
        write_pointer += sprintf(write_pointer, "%2.2X ", data[i]);
    }
    write_pointer += sprintf(write_pointer, ">\n");
    return write_pointer - buffer;
}

void loop() {

    uint8_t buffer[BUFFER_SIZE] = {0};

    if (operationDone) {
        operationDone = false;
        // if (transmitFlag) {
        //   // the previous operation was transmission, listen for response
        //   // print the result
        //   if (transmissionState == RADIOLIB_ERR_NONE) {
        //     // packet was successfully sent
        //     printDeltaT();
        //     Serial.println(F("ESP TX finished"));

        //   } else {
        //     printDeltaT();
        //     Serial.print(F("ESP TX failed, code "));
        //     Serial.println(transmissionState);
        //   }

        // transmitFlag = false;

        // the previous operation was reception
        // print data and send another packet
        size_t packet_size = radio.getPacketLength();
        if (packet_size > BUFFER_SIZE) {
            return;
        }

        // String str;
        int state = radio.readData(buffer, packet_size);

        if (state == RADIOLIB_ERR_NONE) {
            // print data of the packet
            // printDeltaT();
            // Serial.printf("ESP RX ");

            // // print RSSI (Received Signal Strength Indicator)
            // Serial.print(F("RSSI: "));
            // Serial.print(radio.getRSSI());
            // Serial.print(F(" dBm, "));

            // // print SNR (Signal-to-Noise Ratio)
            // Serial.print(F("SNR: "));
            // Serial.print(radio.getSNR());
            // Serial.print(F(" dB, "));
            // print frequency error
            // Serial.print(F("Ferr: "));
            // Serial.print(radio.getFrequencyError());
            // Serial.print(F(" Hz, "));

            // Serial.print(F("AI%: "));
            // uint32_t i;
            // memcpy(&i, &buffer[16], 4);
            // i = ntohl(i);
            // float f = *((float*)&i);
            // Serial.print(f);
            // Serial.print(F(" mA, "));

            // printHex(buffer, packet_size);
            // lora_msg[4] = avg_batt_0_millivolts >> 8;
            // lora_msg[5] = avg_batt_0_millivolts >> 0;
            // lora_msg[6] = avg_batt_1_millivolts >> 8;
            // lora_msg[7] = avg_batt_1_millivolts >> 0;

            // lora_msg[8] = avg_batt_centiamps >> 8;
            // lora_msg[9] = avg_batt_centiamps >> 0;
            // lora_msg[10] = avg_batt_centiamphours >> 8;
            // lora_msg[11] = avg_batt_centiamphours >> 0;

            // lora_msg[12] = avg_load_centiamps >> 8;
            // lora_msg[13] = avg_load_centiamps >> 0;

            // lora_msg[14] = avg_dump_centiamps >> 8;
            // lora_msg[15] = avg_dump_centiamps >> 0;

            int16_t avg_batt_0_millivolts = (((uint16_t)buffer[4] << 8) | (uint16_t)buffer[5]);
            int16_t avg_batt_1_millivolts = (((uint16_t)buffer[6] << 8) | (uint16_t)buffer[7]);
            int16_t avg_batt_centiamps = (((uint16_t)buffer[8] << 8) | (uint16_t)buffer[9]);
            int16_t avg_batt_centiamphours = (((uint16_t)buffer[10] << 8) | (uint16_t)buffer[11]);
            int16_t avg_load_centiamps = (((uint16_t)buffer[12] << 8) | (uint16_t)buffer[13]);
            int16_t avg_dump_centiamps = (((uint16_t)buffer[14] << 8) | (uint16_t)buffer[15]);

            // printf("Lora Batt_0_mV=%ldi,Batt_1_mV=%ldi,Batt_mA=%ldi,Batt_mAh=%ldi,"
            //        "Load_mA=%ldi,Dump_mA=%ldi\n",

            printf("{\"Batt_1_mV\":%ld,\"Batt_2_mV\":%ld,\"Batt_mA\":%ld,\"Batt_mAh\":%ld,\"Load_mA\":%ld,\"Dump_mA\":%ld}\n",
                   (int32_t)avg_batt_0_millivolts, (int32_t)avg_batt_1_millivolts,
                   (int32_t)avg_batt_centiamps * 10, (int32_t)avg_batt_centiamphours * 10,
                   (int32_t)avg_load_centiamps * 10, (int32_t)avg_dump_centiamps * 10);

            // int16_t battv = (((uint16_t)buffer[4] << 8) | (uint16_t)buffer[5]);
            // int16_t batti = (((uint16_t)buffer[6] << 8) | (uint16_t)buffer[7]);
            // int16_t loadah = (((uint16_t)buffer[8] << 8) | (uint16_t)buffer[9]);
            // int16_t loadi = (((uint16_t)buffer[10] << 8) | (uint16_t)buffer[11]);

            // Serial.print(F("BATT "));
            // Serial.print((float)battv / 1000.0);
            // Serial.print(F(" V "));
            // Serial.print((float)batti / 100.0);
            // Serial.print(F(" A, LOAD "));
            // Serial.print((float)loadi / 100.0);
            // Serial.print(F(" A "));
            // Serial.print((float)loadah / 100.0);
            // Serial.print(F(" Ah"));
            // Serial.println();

            radio.startReceive();

            // String message = "Hello, world! Time: ";
            // message += millis();

            // Send the UDP broadcast packet
            // Udp.beginPacket(broadcastAddress, remotePort);
            // char udp_message[BUFFER_SIZE] = { 0 };
            // int udp_message_size = hexDump(udp_message, buffer, packet_size);

            // Udp.write((uint8_t*)udp_message, udp_message_size);
            // Udp.endPacket();  // endPacket() returns status of packet transmission
        }

        // wait a second before transmitting again
        // delay(100);
        // radio.startReceive();
        // send another one
        // Serial.print(F("[SX1262] Sending another packet ... "));

        // strcpy((char*)buffer, "ACK");
        // packet_size = strlen((char*)buffer);

        // printDeltaT();
        // Serial.printf("ESP TX ");
        // printHex(buffer, packet_size);
        // transmissionState = radio.startTransmit(buffer, packet_size);
        // transmitFlag = true;
    }
}

//   Serial.print(F("[SX1262] Waiting for incoming transmission ... "));

// you can receive data as an Arduino String
// String str;
// int state = radio.receive(str);

// // you can also receive data as byte array
// /*
//   byte byteArr[8];
//   int state = radio.receive(byteArr, 8);
// */

// if (state == RADIOLIB_ERR_NONE) {
//   // packet was successfully received

//   // print the data of the packet
//   Serial.print(F("ESP RX Data:<"));
//   Serial.print(str);
//   Serial.print(F(">, "));

//   // print the RSSI (Received Signal Strength Indicator)
//   // of the last received packet
//   Serial.print(F("RSSI:"));
//   Serial.print(radio.getRSSI());
//   Serial.print(F(" dBm, "));

//   // print the SNR (Signal-to-Noise Ratio)
//   // of the last received packet
//   Serial.print(F("SNR: "));
//   Serial.print(radio.getSNR());
//   Serial.print(F(" dB, "));

//   // print frequency error
//   Serial.print(F("Freq err: "));
//   Serial.print(radio.getFrequencyError());
//   Serial.println(F(" Hz"));

//   delay(250);

//   radio.transmit("ESP32 Ack");

// } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
//   // timeout occurred while waiting for a packet
//   // Serial.println(F("RX timeout!"));

// } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
//   // packet was received, but is malformed
//   Serial.println(F("RX CRC error!"));

// } else {
//   // some other error occurred
//   Serial.print(F("RX failed, code "));
//   Serial.println(state);

// }
// }
