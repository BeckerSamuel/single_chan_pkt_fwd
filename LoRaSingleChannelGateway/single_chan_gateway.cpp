/******************************************************************************
 *
 * Copyright (c) 2015 Thomas Telkamp
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 *******************************************************************************/

// Raspberry PI pin mapping
// Pin number in this global_conf.json are Wiring Pi number (wPi colunm)
// issue a `gpio readall` on PI command line to see mapping
// +-----+-----+---------+--B Plus--+---------+-----+-----+
// | BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
// +-----+-----+---------+----++----+---------+-----+-----+
// |     |     |    3.3v |  1 || 2  | 5v      |     |     |
// |   2 |   8 |   SDA.1 |  3 || 4  | 5V      |     |     |
// |   3 |   9 |   SCL.1 |  5 || 6  | 0v      |     |     |
// |   4 |   7 | GPIO. 7 |  7 || 8  | TxD     | 15  | 14  |
// |     |     |      0v |  9 || 10 | RxD     | 16  | 15  |
// |  17 |   0 | GPIO. 0 | 11 || 12 | GPIO. 1 | 1   | 18  |
// |  27 |   2 | GPIO. 2 | 13 || 14 | 0v      |     |     |
// |  22 |   3 | GPIO. 3 | 15 || 16 | GPIO. 4 | 4   | 23  |
// |     |     |    3.3v | 17 || 18 | GPIO. 5 | 5   | 24  |
// |  10 |  12 |    MOSI | 19 || 20 | 0v      |     |     |
// |   9 |  13 |    MISO | 21 || 22 | GPIO. 6 | 6   | 25  |
// |  11 |  14 |    SCLK | 23 || 24 | CE0     | 10  | 8   |
// |     |     |      0v | 25 || 26 | CE1     | 11  | 7   |
// |   0 |  30 |   SDA.0 | 27 || 28 | SCL.0   | 31  | 1   |
// |   5 |  21 | GPIO.21 | 29 || 30 | 0v      |     |     |
// |   6 |  22 | GPIO.22 | 31 || 32 | GPIO.26 | 26  | 12  |
// |  13 |  23 | GPIO.23 | 33 || 34 | 0v      |     |     |
// |  19 |  24 | GPIO.24 | 35 || 36 | GPIO.27 | 27  | 16  |
// |  26 |  25 | GPIO.25 | 37 || 38 | GPIO.28 | 28  | 20  |
// |     |     |      0v | 39 || 40 | GPIO.29 | 29  | 21  |
// +-----+-----+---------+----++----+---------+-----+-----+
// | BCM | wPi |   Name  | Physical | Name    | wPi | BCM |
// +-----+-----+---------+--B Plus--+---------+-----+-----+

#include <mysql.h>
//#include <rapidjson/document.h>
//#include <rapidjson/filereadstream.h>
#include <sys/time.h>
#include <sys/types.h>
#include <wiringPi.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "document.h" //TODO test if that works
#include "filereadstream.h"
#include "LoRa.h"
#include "LoRaCryoting.h"

using namespace std;

using namespace rapidjson;

#define BUFFER_SIZE 2000

/*******************************************************************************
 * Default values, configure them in global_conf.json
 *******************************************************************************/
// Set spreading factor (SF7 - SF12), &nd  center frequency
// Overwritten by the ones set in global_conf.json
uint32_t freq = LORA_CRYPTING_BAND; // in Mhz! (868)
SpreadingFactor_t sf = LORA_CRYPTING_SPREAD_FACTOR;
uint32_t bw = LORA_CRYPTING_BAND;

// SX1276 - Raspberry connections
// Overwritten by the ones set in global_conf.json
int spiNr = 0xff;
int spiChannel = 0xff;
int ssPin = 0xff;
int dio0 = 0xff;
int RST = 0xff;

// Keys for LoRa message encryption
// Overwritten by the ones set in global_conf.json
uint8_t encryptKey = LORA_CRYPTING_ENCRYPT_KEY;
uint8_t decryptKey = LORA_CRYPTING_DECRYPT_KEY;

// Database - Connection information
// Overwritten by the ones set in global_conf.json
string host = "";
string user = "";
string passwd = "";
string dbName = "";
string dbRawMessages = "";
string dbDeviceConfig = "";

// Gateway location
// Overwritten by the ones set in global_conf.json
float lat = 0.0;
float lon = 0.0;
int alt = 0;

uint16_t msgPos = 0;
LoRaMessageContainer_t *messageIn;
map<int, char *> configs;   // device id to config
map<string, string> types;  // devcie type to message struct //TODO remove all string parts and use arrays instead

// Database Connection
MYSQL *connection, mysql;
MYSQL_RES *result;
MYSQL_ROW row;
int query_state;
char query[512] = {'\0'};

//on interrupt
// receive message
// process message
// if not valid stop or send nack
// if valid check


// message valid:
//  do nothing if checksum is wrong
//  if number is wrong send correct number in nack
//  if anything else is wrong, just accept it (e.g. the content length)

static void onLoRaMessageReceived(int packetSize) {
	// load message
	if (packetSize) {
		LoRaEncryptedMessage receivedMessage;
		LoRaMessageContainer_t loraMessageContainer;
		
        for (uint8_t i = 0; i < packetSize; i++) {
            receivedMessage.content[i] = LoRa.read();
        }
		
		//process message
		if(decryptLoRaMessageContainer(&loraMessageContainer, &receivedMessage, packetSize, LoRa.packetRssi()) == 0) {
			//received successful
			//save message
			//trigger ack
		} else {
			//some problem occoured
			//send nack
		}
}

void getLoRa(struct LoRaMessage *message) {
    uint8_t packetSize = LoRa.parsePacket();
    if (packetSize) {
        uint16_t checksum = 0;
        char input[packetSize] = {'\0'};
        for (uint8_t i = 0; i < packetSize; i++) {
            input[i] = LoRa.read();
        }

        // Checksum (2 Byte)
        checksum += (uint16_t)decryptChar(input[(packetSize - 2)]) << 8;
        checksum += (uint16_t)decryptChar(input[(packetSize - 1)]);
        for (uint8_t i = 0; i < (packetSize - 2); i++) {
            checksum -= input[i];
        }

        if (!checksum) {
            uint8_t ptr = 0;

            // Device id (8 Byte)
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 56;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 48;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 40;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 32;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 24;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 16;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 8;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]);

            // Device type (1 Byte)
            message->deviceType = decryptChar(input[ptr++]);

            // message (messageLength Byte)
            uint8_t i = 0;
            for (i = 0; i < (packetSize - 11); i++) {
                message->message[i] = decryptChar(input[ptr++]);
            }
            message->message[i] = '\0';

            message->rssi = LoRa.packetRssi();

            message->empty = 0;
        }
    }
}

//TODO use one thread for getting messages inside the array and notify the main thread
//TODO use the main thread to send them per api call (libcurl) pref gnutls (slower but more open source?)
//https://curl.se/download.html
//https://packages.debian.org/search?keywords=libcurl4-gnutls-dev
//https://packages.debian.org/search?keywords=libcurl4-openssl-dev

//TODO so the main thread will walk behind the other one

//TODO every hour (have a timestamp and set the sleep/delay accordingly after an notify/interrupt) check the api for new configs
//TODO if a config was received, lock the config array (the lora thread will not send configs then)
//TODO and notify the api about all successfully transfered configs, delete the successfull ones and save all others (override old ones if neccessary)
//TODO the time (1 hour) will be changeably by the config file (so check every 5 minutes or so for example)

//TODO the lora thread will answer with ok to every received message
//TODO in case of an config it will send the config first and answers ok to the device ok
//TODO have the posibility to disable ok messages per device (or device type) => will also be loaded every hour from the api

// TODO overhaul everything again
// TODO use onReceive function of the LoRa class and check if onTxDone is also needed?
// TODO only have a timer here to access db and put received emssages in there as well as get new configurations
int main() {
    uint32_t lasttime = 0;
    struct timeval nowtime;
    bool toggle = true;

    messageIn = (LoRaMessageContainer_t *)malloc(BUFFER_SIZE * sizeof(LoRaMessageContainer_t));
    for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
        (messageIn + i)->empty = 1;
    }

    LoRaMessageContainer_t *messageOut;
    messageOut = (LoRaMessageContainer_t *)malloc(sizeof(LoRaMessageContainer_t));
    messageOut->empty = 1;

    LoadConfiguration("global_conf.json");
    PrintConfiguration();

    // TODO create database if not yet created
    // open db connection (will be used by all following functions)
    // create raw messages table (if not exists)
    // try load types (create table if not exists)
    // try load configs (create table if not exists)

    // Init WiringPI
    wiringPiSetup();

    // Setup LORA
    LoRa.setPins(ssPin, RST, dio0);

    if (LoRa.begin(freq, spiNr, spiChannel))
        printf("LoRa Initializing OK!\n");
    else
        printf("Starting LoRa failed!\n");

    // set spreading factor
    LoRa.setSpreadingFactor(sf);
    // set signal bandwidth
    LoRa.setSignalBandwidth(bw);
	
	// set up interrupt on receive
	LoRa.onReceive(void (*callback)(int))

    printf("Listening at SF%i on %.6lf Mhz.\n", sf, (double)freq / 1000000);
    printf("-----------------------------------\n");
	
	
	
	
	
	
	
	/*

    while (1) {
        getLoRa(messageIn + msgPos);
        if ((messageIn + msgPos)->empty == 0) {
            printf("%s\n", (messageIn + msgPos)->message);

            char *config = configs[(messageIn + msgPos)->deviceID];
            if (config[0] != '\0') {
                memcpy(messageOut, (messageIn + msgPos), sizeof(struct LoRaMessage));
                strcpy(messageOut->message, config);
                sendLoRa(messageOut);
                configs[(messageIn + msgPos)->deviceID][0] = '\0';
            }

            // increase ringbuffer
            msgPos++;
            if (msgPos >= (BUFFER_SIZE - 25)) {
                saveLoRaMessages();
            }
        }

        gettimeofday(&nowtime, NULL);
        uint32_t nowseconds = (uint32_t)(nowtime.tv_sec);
        if (nowseconds - lasttime >= 900) {  // save data in the database every 30 minutes and get new config also every 30 minutes (wait 15 minutes between the both)
            lasttime = nowseconds;

            printf("I'm alive!\n");  // TODO remove that later

            if (toggle) {  // save data in database
                saveLoRaMessages();
                toggle = false;
            } else {  // get new configs from the database
                getLoRaConfigs();
                toggle = true;
            }
        }

        // Let some time to the OS
        delay(1);
    }

    // TODO test database connection:
    /*
    LoadConfiguration("global_conf.json");
    PrintConfiguration();

    printf("%s \r\n", host.c_str());
    printf("%s \r\n", user.c_str());
    printf("%s \r\n", passwd.c_str());
    printf("%s \r\n", dbName.c_str());
    printf("%s \r\n", dbRawMessages.c_str());
    printf("%s \r\n", dbDeviceConfig.c_str());

    //initialize database connection
    mysql_init(&mysql);

    // the three zeros are: Which port to connect to, which socket to connect to
    // and what client flags to use.  unless you're changing the defaults you only need to put 0 here
    connection = mysql_real_connect(&mysql, host.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), 0, 0, 0);

    // Report error if failed to connect to database
    if (connection == NULL) {
        cout << mysql_error(&mysql) << endl;
        return 1;
    }

    string rawMessagesTableCreate = "CREATE TABLE IF NOT EXISTS %s (uniqueId INTEGER NOT NULL AUTO INCREMENT, timestamp DATE NOT NULL, device_id INTEGER NOT NULL, message VARCHAR(256))";
    string deviceConfigTableCreate = "CREATE TABLE IF NOT EXISTS %s (id INTEGER NOT NULL AUTO INCREMENT, device_id INTEGER NOT NULL, config VARCHAR(256))";
    sprintf(query, rawMessagesTableCreate.c_str(), dbRawMessages.c_str());

    printf("%s \r\n", query);

    query_state = mysql_query(connection, query);

    printf("%d \r\n", query_state);

    //Send query to database
    query_state = mysql_query(connection, "select * from pinDays");

    // store result
    result = mysql_store_result(connection);
    while ( ( row = mysql_fetch_row(result)) != NULL ) {
      // Print result, it prints row[column_number])
      cout << row[0] << "\t" << row[1] << endl;
    }*/

    return 0;
}

void LoadConfiguration(string configurationFile) {
    FILE *p_file = fopen(configurationFile.c_str(), "r");
    char buffer[65536];
    FileReadStream fs(p_file, buffer, sizeof(buffer));

    Document document;
    document.ParseStream(fs);

    for (Value::ConstMemberIterator fileIt = document.MemberBegin(); fileIt != document.MemberEnd(); ++fileIt) {
        string objectType(fileIt->name.GetString());
        if (objectType.compare("SX127x_conf") == 0) {
            const Value &sx127x_conf = fileIt->value;
            if (sx127x_conf.IsObject()) {
                for (Value::ConstMemberIterator confIt = sx127x_conf.MemberBegin(); confIt != sx127x_conf.MemberEnd(); ++confIt) {
                    string key(confIt->name.GetString());
                    if (key.compare("freq") == 0) {
                        freq = confIt->value.GetUint();
                    } else if (key.compare("spread_factor") == 0) {
                        sf = (SpreadingFactor_t)confIt->value.GetUint();
                    } else if (key.compare("bandwidth") == 0) {
                        bw = confIt->value.GetUint();
                    } else if (key.compare("spi_number") == 0) {
                        spiNr = confIt->value.GetUint();
                    } else if (key.compare("spi_channel") == 0) {
                        spiChannel = confIt->value.GetUint();
                    } else if (key.compare("pin_nss") == 0) {
                        ssPin = confIt->value.GetUint();
                    } else if (key.compare("pin_dio0") == 0) {
                        dio0 = confIt->value.GetUint();
                    } else if (key.compare("pin_rst") == 0) {
                        RST = confIt->value.GetUint();
                    } else if (key.compare("encrypt_key") == 0) {
                        encryptKey = confIt->value.GetUint();
                    } else if (key.compare("decrypt_key") == 0) {
                        decryptKey = confIt->value.GetUint();
                    }
                }
            }
        } else if (objectType.compare("database") == 0) {
            const Value &database_conf = fileIt->value;
            if (database_conf.IsObject()) {
                for (Value::ConstMemberIterator confIt = database_conf.MemberBegin(); confIt != database_conf.MemberEnd(); ++confIt) {
                    string key(confIt->name.GetString());
                    if (key.compare("host") == 0) {
                        host.append(confIt->value.GetString());
                    } else if (key.compare("user") == 0) {
                        user.append(confIt->value.GetString());
                    } else if (key.compare("passwd") == 0) {
                        passwd.append(confIt->value.GetString());
                    } else if (key.compare("database_name") == 0) {
                        dbName.append(confIt->value.GetString());
                    } else if (key.compare("db_raw_messages") == 0) {
                        dbRawMessages.append(confIt->value.GetString());
                    } else if (key.compare("db_device_config") == 0) {
                        dbDeviceConfig.append(confIt->value.GetString());
                    }
                }
            }
        } else if (objectType.compare("location") == 0) {
            const Value &database_conf = fileIt->value;
            if (database_conf.IsObject()) {
                for (Value::ConstMemberIterator confIt = database_conf.MemberBegin(); confIt != database_conf.MemberEnd(); ++confIt) {
                    string key(confIt->name.GetString());
                    if (key.compare("ref_latitude") == 0) {
                        lat = confIt->value.GetDouble();
                    } else if (key.compare("ref_longitude") == 0) {
                        lon = confIt->value.GetDouble();
                    } else if (key.compare("ref_altitude") == 0) {
                        alt = confIt->value.GetInt();
                    }
                }
            }
        }
    }
}

void PrintConfiguration() {
    printf("Gateway Configuration\n");
    printf("  Host=%s\n  DB-Raw-Messages=%s\n  DB-Device-Config=%s\n  User=%s\n  Password=%s\n", host.c_str(), dbRawMessages.c_str(), dbDeviceConfig.c_str(), user.c_str(), passwd.c_str());
    printf("  Latitude=%.8f\n  Longitude=%.8f\n  Altitude=%d\n", lat, lon, alt);
}

void saveLoRaMessages(void) {
    uint16_t oldPos = msgPos;

    for (uint16_t i = 0; i < msgPos; i++) {
        memcpy((messageInTemp + i), (messageIn + i), sizeof(struct LoRaMessage));
        (messageIn + i)->empty = 1;
    }
    msgPos = 0;

    // TODO start new thread (use oldPos and messageInTemp)
    // 1. get device type of message
    // 2. check for that type if there is a db
    // 3. if yes, save it there (split data according)
    // 4. if no, then create table
    // 5. spezial case, when there is no type, then save the message raw in the raw messages table
}

void getLoRaConfigs(void) {
    // TODO load new configurations from the database
    // 1. start thread (all following in the thread)
    // 2. start database request
    // 3. save all in the configs map
    // 4. make a database request to set all downloaded configs to send (so we wont download them again)

    // TODO also load all types into a map (types to message struct (how to spilt data, means 2 characters into first, then 6 into second usw, id is always the same))
}

//TODO use new library functions instead
/*// #
// # LORA CRYPTING
// #
void sendLoRa(struct LoRaMessage *message) {
    uint8_t messageLength = strlen(message->message) + 11;
    char output[messageLength] = {'\0'};
    uint8_t ptr = 0;
    uint16_t checksum = 0;

    // Device id (8 Byte)
    output[ptr++] = encryptChar((uint8_t)(message->deviceID >> 56));
    output[ptr++] = encryptChar((uint8_t)(message->deviceID >> 48));
    output[ptr++] = encryptChar((uint8_t)(message->deviceID >> 40));
    output[ptr++] = encryptChar((uint8_t)(message->deviceID >> 32));
    output[ptr++] = encryptChar((uint8_t)(message->deviceID >> 24));
    output[ptr++] = encryptChar((uint8_t)(message->deviceID >> 16));
    output[ptr++] = encryptChar((uint8_t)(message->deviceID >> 8));
    output[ptr++] = encryptChar((uint8_t)(message->deviceID));

    // Device type (1 Byte)
    output[ptr++] = encryptChar(message->deviceType);

    // message (messageLength Byte)
    for (uint8_t i = 0; i < messageLength; i++) {
        output[ptr++] = encryptChar(message->message[i]);
    }

    // Checksum (2 Byte)
    for (uint8_t i = 0; i < ptr; i++) {
        checksum += output[i];
    }
    output[ptr++] = encryptChar((uint8_t)(checksum >> 8));
    output[ptr++] = encryptChar((uint8_t)(checksum));

    LoRa.beginPacket();
    LoRa.write((uint8_t *)output, ptr);
    LoRa.endPacket();
}

void getLoRa(struct LoRaMessage *message) {
    uint8_t packetSize = LoRa.parsePacket();
    if (packetSize) {
        uint16_t checksum = 0;
        char input[packetSize] = {'\0'};
        for (uint8_t i = 0; i < packetSize; i++) {
            input[i] = LoRa.read();
        }

        // Checksum (2 Byte)
        checksum += (uint16_t)decryptChar(input[(packetSize - 2)]) << 8;
        checksum += (uint16_t)decryptChar(input[(packetSize - 1)]);
        for (uint8_t i = 0; i < (packetSize - 2); i++) {
            checksum -= input[i];
        }

        if (!checksum) {
            uint8_t ptr = 0;

            // Device id (8 Byte)
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 56;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 48;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 40;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 32;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 24;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 16;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]) << 8;
            message->deviceID += (uint64_t)decryptChar(input[ptr++]);

            // Device type (1 Byte)
            message->deviceType = decryptChar(input[ptr++]);

            // message (messageLength Byte)
            uint8_t i = 0;
            for (i = 0; i < (packetSize - 11); i++) {
                message->message[i] = decryptChar(input[ptr++]);
            }
            message->message[i] = '\0';

            message->rssi = LoRa.packetRssi();

            message->empty = 0;
        }
    }
}

unsigned char encryptChar(unsigned char message) {
    return ((message * encryptKey) % 256);
}

unsigned char decryptChar(unsigned char cypher) {
    return ((cypher * decryptKey) % 256);
}*/