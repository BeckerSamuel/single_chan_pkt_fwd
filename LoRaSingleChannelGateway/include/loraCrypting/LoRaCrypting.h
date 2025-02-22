#ifndef LORACRYPTING
#define LORACRYPTING

#include <stdint.h>

typedef enum SpreadingFactors {
    SF7 = 7,
    SF8 = 8,
    SF9 = 9,
    SF10 = 10,
    SF11 = 11,
    SF12 = 12
} SpreadingFactor_t;

//band 868 mHz
#define LORA_CRYPTING_BAND 868E6
//band width 259 kHz
#define LORA_CRYPTING_BAND 250E3
//spread factor SF7
#define LORA_CRYPTING_SPREAD_FACTOR SF7

#define LORA_CRYPTING_ENCRYPT_KEY 83
#define LORA_CRYPTING_DECRYPT_KEY 219

typedef struct LoRaMessage {
  uint64_t deviceID;
  uint8_t deviceType;
  uint8_t messageCounter;
  uint8_t contentLength;
  uint8_t content[242];
  uint16_t checksum;
} LoRaMessage_t;

typedef struct LoRaEncryptedMessage {
  uint8_t content[255];
} LoRaEncryptedMessage_t;

typedef struct LoRaMessageContainer {
  LoRaMessage_t loraMessage;
  int64_t timestamp;
  int16_t rssi;
  uint8_t isEmpty;
} LoRaMessageContainer_t;

int encryptLoRaMessageconst LoRaMessage_t *loraMessage, LoRaEncryptedMessage_t *loraEncryptedMessage;
int encryptLoRaMessageContainer(const LoRaMessageContainer_t *loraMessageContainer, LoRaEncryptedMessage_t *loraEncryptedMessage);
int decryptLoRaMessage(LoRaMessage_t *loraMessage, const LoRaEncryptedMessage_t *loraEncryptedMessage, const uint8_t length);
int decryptLoRaMessageContainer(LoRaMessageContainer_t *loraMessageContainer, const LoRaEncryptedMessage_t *loraEncryptedMessage, const uint8_t length, const int16_t rssi);

#endif