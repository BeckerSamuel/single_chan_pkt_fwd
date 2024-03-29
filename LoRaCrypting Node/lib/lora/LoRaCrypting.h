#ifndef LORACRYPTING
#define LORACRYPTING

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define BAND 868E6

#define encryptKey 83
#define decryptKey 219

#define DEBUGOUTPUT 1

struct LoRaMessage {
	uint64_t deviceID;
	uint8_t deviceType;
	char message[255];
  uint8_t empty;
};

bool initLoRa(void);
void endLoRa(void);
void sendLoRa(struct LoRaMessage *message);
void getLoRa(struct LoRaMessage *message);
unsigned char encryptChar(unsigned char message);
unsigned char decryptChar(unsigned char cypher);

#endif