#include "LoRaCrypting.h"
#include <stdlib.h>
#include <time.h>

static unsigned char encryptChar(unsigned char byte) {
  return ((byte * LORA_CRYPTING_ENCRYPT_KEY) % 256);
}

int encryptLoRaMessage(const LoRaMessage_t *loraMessage, LoRaEncryptedMessage_t *loraEncryptedMessage) {
  uint8_t ptr = 0;

  if((loraMessage == NULL) || (loraEncryptedMessage == NULL) ||
     (loraMessage->deviceID == 0) || (loraMessage->contentLength == 0)) {
    return -1;
  }

  //Device id (8 Byte)
  for (uint8_t i = 0; i < sizeof(loraMessage->deviceID); i++) {
    loraEncryptedMessage->content[ptr++] = encryptChar((uint8_t) (loraMessage->deviceID >> (i * 8)));
  }

  //Device type (1 Byte)
  loraEncryptedMessage->content[ptr++] = encryptChar(loraMessage->deviceType);

  //Content length (1 Byte)
  loraEncryptedMessage->content[ptr++] = encryptChar(loraMessage->contentLength);

  //Content (contentLength Byte)
  for(uint8_t i = 0; i < loraMessage->contentLength; i++) {
    loraEncryptedMessage->content[ptr++] = encryptChar(loraMessage->content[i]);
  }

  //Checksum (2 Byte)
  for(uint8_t i = 0; i < ptr; i++) {
    loraMessage->checksum += loraEncryptedMessage->content[i];
  }
  loraEncryptedMessage->content[ptr++] = encryptChar((uint8_t) (loraMessage->checksum));
  loraEncryptedMessage->content[ptr++] = encryptChar((uint8_t) (loraMessage->checksum >> 8));

  return 0;
}

int encryptLoRaMessageContainer(const LoRaMessageContainer_t *loraMessageContainer, LoRaEncryptedMessage_t *loraEncryptedMessage) {
  if(loraMessageContainer == NULL) {
    return -1;
  }

  return encryptLoRaMessage(loraMessageContainer->loraMessage, loraEncryptedMessage);
}

static unsigned char decryptChar(unsigned char encryptedByte) {
  return ((encryptedByte * LORA_CRYPTING_DECRYPT_KEY) % 256);
}

int decryptLoRaMessage(LoRaMessage_t *loraMessage, const LoRaEncryptedMessage_t *loraEncryptedMessage, const uint8_t length) {
  uint8_t ptr = 0;

  if((loraMessage == NULL) || (loraEncryptedMessage == NULL)) {
    return -1;
  }

  //Checksum (2 Byte)
  loraMessage->checksum += (uint16_t) decryptChar((unsigned char) loraEncryptedMessage[(length - sizeof(loraMessage->checksum))]);
  loraMessage->checksum += (uint16_t) (decryptChar((unsigned char) loraEncryptedMessage[(length - (sizeof(loraMessage->checksum) - 1))]) << 8);
  for(uint8_t i = 0; i < (length - sizeof(loraMessage->checksum)); i++) {
    loraMessage->checksum -= loraEncryptedMessage[i];
  }

  if(loraMessage->checksum == 0) {
    //Device id (8 Byte)
    for(uint8_t i = 0; i < sizeof(loraMessage->deviceID); i++) {
      loraMessage->deviceID += (uint64_t) decryptChar((unsigned char) loraEncryptedMessage[ptr++]) << (i * 8);
    }

    //Device type (1 Byte)
    loraMessage->deviceType = decryptChar((unsigned char) input[ptr++]);

    //Content length (1 Byte)
    loraMessage->contentLength = decryptChar((unsigned char) input[ptr++]);

    //Content (contentLength Byte)
    for(uint8_t i = 0; i < loraMessage->contentLength; i++) {
      loraMessage->loraEncryptedMessage[i] = decryptChar((unsigned char) input[ptr++]);
    }

    return 0;
  } else {
    return -1;
  }
}

int decryptLoRaMessageContainer(LoRaMessageContainer_t *loraMessageContainer, const LoRaEncryptedMessage_t *loraEncryptedMessage, const uint8_t length, const int16_t rssi) {
  int res = -1;

  if(loraMessageContainer != NULL) {
    res = decryptLoRaMessage(loraMessageContainer->loraMessage, loraEncryptedMessage, length);

    loraMessageContainer->timestamp = time();
    loraMessageContainer->rssi = rssi;
    loraMessageContainer->isEmpty = (res != 0);
  }

  return res;
}
