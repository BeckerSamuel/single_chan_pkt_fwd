#include <Arduino.h>
#include "LoRaCrypting.h"
#include "LoRa.h"

bool initLoRa(void) {
    //SPI LoRa pins
    SPI.begin(SCK, MISO, MOSI, SS);
    //setup LoRa transceiver module
    LoRa.setPins(SS, RST, DIO0);

    if (LoRa.begin(BAND)) {
#if DEBUGOUTPUT
        Serial.println("LoRa Initializing OK!");
#endif
        delay(1000);
        return true;
    } else {
#if DEBUGOUTPUT
        Serial.println("Starting LoRa failed!");
#endif
        return false;
    }

    //set spreading factor
    LoRa.setSpreadingFactor(7);
    //set signal bandwidth
    LoRa.setSignalBandwidth(125E3);
}

void endLoRa() {
    LoRa.end();
}

void sendLoRa(struct LoRaMessage *message) {
  uint8_t messageLength = strlen(message->message) + 11;
  char output[messageLength] = { '\0' };
  uint8_t ptr = 0;
  uint16_t checksum = 0;

  //Device id (8 Byte)
  output[ptr++] = encryptChar((uint8_t) (message->deviceID >> 56));
  output[ptr++] = encryptChar((uint8_t) (message->deviceID >> 48));
  output[ptr++] = encryptChar((uint8_t) (message->deviceID >> 40));
  output[ptr++] = encryptChar((uint8_t) (message->deviceID >> 32));
  output[ptr++] = encryptChar((uint8_t) (message->deviceID >> 24));
  output[ptr++] = encryptChar((uint8_t) (message->deviceID >> 16));
  output[ptr++] = encryptChar((uint8_t) (message->deviceID >> 8));
  output[ptr++] = encryptChar((uint8_t) (message->deviceID));

  //Device type (1 Byte)
  output[ptr++] = encryptChar(message->deviceType);

  //message (messageLength Byte)
  for(uint8_t i = 0; i < messageLength; i++) {
    output[ptr++] = encryptChar(message->message[i]);
  }

  //Checksum (2 Byte)
  for(uint8_t i = 0; i < ptr; i++) {
    checksum += output[i];
  }
  output[ptr++] = encryptChar((uint8_t) (checksum >> 8));
  output[ptr++] = encryptChar((uint8_t) (checksum));

  LoRa.beginPacket();
  LoRa.write((uint8_t *) output, ptr);
  LoRa.endPacket();
}

void getLoRa(struct LoRaMessage *message) {
    uint8_t packetSize = LoRa.parsePacket();
    if (packetSize) {
      uint16_t checksum = 0;
      char input[packetSize] = { '\0' };
      for(uint8_t i = 0; i < packetSize; i++) {
        input[i] = LoRa.read();
      }

      //Checksum (2 Byte)
      checksum += (uint16_t) decryptChar(input[(packetSize - 2)]) << 8;
      checksum += (uint16_t) decryptChar(input[(packetSize - 1)]);
      for(uint8_t i = 0; i < (packetSize - 2); i++) {
        checksum -= input[i];
      }

      if(!checksum) {
        uint8_t ptr = 0;

        //Device id (8 Byte)
        message->deviceID += (uint64_t) decryptChar(input[ptr++]) << 56;
        message->deviceID += (uint64_t) decryptChar(input[ptr++]) << 48;
        message->deviceID += (uint64_t) decryptChar(input[ptr++]) << 40;
        message->deviceID += (uint64_t) decryptChar(input[ptr++]) << 32;
        message->deviceID += (uint64_t) decryptChar(input[ptr++]) << 24;
        message->deviceID += (uint64_t) decryptChar(input[ptr++]) << 16;
        message->deviceID += (uint64_t) decryptChar(input[ptr++]) << 8;
        message->deviceID += (uint64_t) decryptChar(input[ptr++]);

        //Device type (1 Byte)
        message->deviceType = decryptChar(input[ptr++]);

        //message (messageLength Byte)
        for(uint8_t i = 0; i < (packetSize - 11); i++) {
          message->message[i] = decryptChar(input[ptr++]);
        }

        message->empty = 0;
      }
    }
}

unsigned char encryptChar(unsigned char message) {
    return ((message * encryptKey) % 256);
}

unsigned char decryptChar(unsigned char cypher) {
    return ((cypher * decryptKey) % 256);
}