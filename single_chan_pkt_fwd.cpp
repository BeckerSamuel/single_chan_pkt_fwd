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

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <mysql.h>

#include <sys/time.h>
#include <sys/types.h>

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "LoRa.h"

using namespace std;

using namespace rapidjson;

#define BUFFER_SIZE 2000

typedef enum SpreadingFactors
{
    SF7 = 7,
    SF8,
    SF9,
    SF10,
    SF11,
    SF12
} SpreadingFactor_t;

/*******************************************************************************
 *
 * Default values, configure them in global_conf.json
 *
 *******************************************************************************/

// SX1276 - Raspberry connections
// Put them in global_conf.json
int ssPin = 0xff;
int dio0  = 0xff;
int RST   = 0xff;

//Database - Connection information
// Put them in global_conf.json
string host = "";
string user = "";
string passwd = "";
string db_name = "";
string db_raw_messages = "";
string db_device_config = "";

// Set location in global_conf.json
float lat =  0.0;
float lon =  0.0;
int   alt =  0;

// Set spreading factor (SF7 - SF12), &nd  center frequency
// Overwritten by the ones set in global_conf.json
SpreadingFactor_t sf = SF7;
uint32_t bw = 125E3;
uint32_t freq = 868E6; // in Mhz! (868)

uint8_t encryptKey = 0;
uint8_t decryptKey = 0;

map<int, string> configs;

struct LoRaMessage {
	uint64_t deviceID;
	uint8_t deviceType;
	char message[255];
  int16_t rssi;
  uint8_t empty;
};

void LoadConfiguration(string filename);
void PrintConfiguration();
void sendLoRa(struct LoRaMessage *message);
void getLoRa(struct LoRaMessage *message);
unsigned char encryptChar(unsigned char message);
unsigned char decryptChar(unsigned char cypher);

int main()
{
  /*uint32_t lasttime = 0;
  struct timeval nowtime;
  struct LoRaMessage *messageIn;
  messageIn = (struct LoRaMessage*) malloc(BUFFER_SIZE * sizeof(struct LoRaMessage));
  for(uint16_t i = 0; i < BUFFER_SIZE; i++) {
    (messageIn+pos)->empty = 1;
  }
  struct LoRaMessage *messageOut;
  messageOut = (struct LoRaMessage*) malloc(sizeof(struct LoRaMessage));
  messageOut->empty = 1;
  uint16_t pos = 0;
  bool toggle = true;
  
  LoadConfiguration("global_conf.json");
  PrintConfiguration();

  //TODO create database if not yet created

  // Init WiringPI
  wiringPiSetup() ;

  // Setup LORA
  LoRa.setPins(ssPin, RST, dio0);

  if (LoRa.begin(freq)) printf("LoRa Initializing OK!\n");
  else printf("Starting LoRa failed!\n");

  //set spreading factor
  LoRa.setSpreadingFactor(sf);
  //set signal bandwidth
  LoRa.setSignalBandwidth(bw);

  printf("Listening at SF%i on %.6lf Mhz.\n", sf,(double)freq/1000000);
  printf("-----------------------------------\n");

  while(1) {
    getLoRa(messageIn+pos);
    if((messageIn+pos)->empty == 0) {
      (messageIn+pos)->empty = 1;
      printf("%s\n", (messageIn+pos)->message);

      string config = configs[(messageIn+pos)->deviceID];
      if(config != NULL) {
        memcpy(messageOut, (messageIn+pos), sizeof(struct LoRaMessage));
        strcpy(messageOut->message, config);
        sendLoRa(messageOut);
      }
      pos++;
    }

    gettimeofday(&nowtime, NULL);
    uint32_t nowseconds = (uint32_t)(nowtime.tv_sec);
    if (nowseconds - lasttime >= 900) { //save data in the database every 30 minutes and get new config also every 30 minutes (wait 15 minutes between the both)
      lasttime = nowseconds;
      
      printf("I'm alive!\n");

      if(toggle) { //save data in database
        string database_input = "";
        for(uint8_t i = 0; i < pos; pos++) {
          //transform
          //database_input += received data
        }
        toggle = false;
      } else { //get new configs from the database

        toggle = true;
      }
    }

    // Let some time to the OS
    delay(1);
  }*/

  //TODO test database connection:
  MYSQL *connection, mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int query_state;

  char query[512] = { '\0' };


  /*
  string host = "";
  string user = "";
  string passwd = "";
  string db_name = "";
  string db_raw_messages = "";
  string db_device_config = "";
  */

  printf("%s \r\n", host);
  printf("%s \r\n", user);
  printf("%s \r\n", passwd);
  printf("%s \r\n", db_name);
  printf("%s \r\n", db_raw_messages);
  printf("%s \r\n", db_device_config);

  //initialize database connection
  mysql_init(&mysql);

  // the three zeros are: Which port to connect to, which socket to connect to 
  // and what client flags to use.  unless you're changing the defaults you only need to put 0 here
  connection = mysql_real_connect(&mysql,host.c_str(),user.c_str(),passwd.c_str(),db_name.c_str(),0,0,0); 

  // Report error if failed to connect to database
  if (connection == NULL) {
      cout << mysql_error(&mysql) << endl;
      return 1;
  }

  string rawMessagesTableCreate = "CREATE TABLE IF NOT EXISTS %s (id INTEGER NOT NULL AUTO INCREMENT, timestamp DATE NOT NULL, device_id INTEGER NOT NULL, message VARCHAR(256))";
  string deviceConfigTableCreate = "CREATE TABLE IF NOT EXISTS %s (id INTEGER NOT NULL AUTO INCREMENT, device_id INTEGER NOT NULL, config VARCHAR(256))";
  sprintf(query, rawMessagesTableCreate.c_str(), db_raw_messages);

  printf("%s \r\n", query);

  query_state = mysql_query(connection, query);

  printf("%d \r\n", query_state);

  /*//Send query to database
  query_state = mysql_query(connection, "select * from pinDays");

  // store result
  result = mysql_store_result(connection);
  while ( ( row = mysql_fetch_row(result)) != NULL ) {
    // Print result, it prints row[column_number])
    cout << row[0] << "\t" << row[1] << endl;
  }*/

  return 0;
}

void LoadConfiguration(string configurationFile)
{
  FILE* p_file = fopen(configurationFile.c_str(), "r");
  char buffer[65536];
  FileReadStream fs(p_file, buffer, sizeof(buffer));

  Document document;
  document.ParseStream(fs);

  for (Value::ConstMemberIterator fileIt = document.MemberBegin(); fileIt != document.MemberEnd(); ++fileIt) {
    string objectType(fileIt->name.GetString());
    if (objectType.compare("SX127x_conf") == 0) {
      const Value& sx127x_conf = fileIt->value;
      if (sx127x_conf.IsObject()) {
        for (Value::ConstMemberIterator confIt = sx127x_conf.MemberBegin(); confIt != sx127x_conf.MemberEnd(); ++confIt) {
          string key(confIt->name.GetString());
          if (key.compare("freq") == 0) {
            freq = confIt->value.GetUint();
          } else if (key.compare("spread_factor") == 0) {
            sf = (SpreadingFactor_t)confIt->value.GetUint();
          } else if (key.compare("bandwidth") == 0) {
            bw = confIt->value.GetUint();
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
      const Value& database_conf = fileIt->value;
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
            db_name.append(confIt->value.GetString());
          } else if (key.compare("db_raw_messages") == 0) {
            db_raw_messages.append(confIt->value.GetString());
          } else if (key.compare("db_device_config") == 0) {
            db_device_config.append(confIt->value.GetString());
          }
        }
      }
    } else if (objectType.compare("location") == 0) {
      const Value& database_conf = fileIt->value;
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

void PrintConfiguration()
{
  printf("Gateway Configuration\n");
  printf("  Host=%s\n  DB-Raw-Messages=%s\n  DB-Device-Config=%s\n  User=%s\n  Password=%s\n", host.c_str(), db_raw_messages.c_str(), db_device_config.c_str(), user.c_str(), passwd.c_str());
  printf("  Latitude=%.8f\n  Longitude=%.8f\n  Altitude=%d\n", lat, lon, alt);
}

//#
//# LORA CRYPTING
//#
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
        uint8_t i = 0;
        for(i = 0; i < (packetSize - 11); i++) {
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
}