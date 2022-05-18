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

#include <sys/time.h>
#include <sys/types.h>

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "LoRa.h"

using namespace std;

using namespace rapidjson;

uint32_t cp_nb_rx_rcv;
uint32_t cp_nb_rx_ok;
uint32_t cp_nb_rx_ok_tot;
uint32_t cp_nb_rx_bad;
uint32_t cp_nb_rx_nocrc;
uint32_t cp_up_pkt_fwd;

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
string db_raw_messages = "";
string db_device_config = "";

// Set location in global_conf.json
float lat =  0.0;
float lon =  0.0;
int   alt =  0;

// Set spreading factor (SF7 - SF12), &nd  center frequency
// Overwritten by the ones set in global_conf.json
SpreadingFactor_t sf = SF7;
uint16_t bw = 125E3;
uint32_t freq = 868E6; // in Mhz! (868)

struct LoRaMessage {
	uint32_t deviceID;
	uint8_t devicetype;
	char message[255];
	uint16_t checksum;
  uint8_t empty;
};

void LoadConfiguration(string filename);
void PrintConfiguration();
void sendLoRa(struct LoRaMessage message);
void getLoRa(struct LoRaMessage message);

int main()
{
  uint32_t lasttime = 0;
  struct timeval nowtime;
  struct LoRaMessage message;
  message.empty = 1;

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
    getLoRa(&message);
    if(message.empty == 0) {
      message.message += "\n";
      printf(message.message);
      //TODO save the message in the db
      //TODO check if there is a new config
      //TODO answer the device with ok/or the config

      message.message = "OK";
      sendLoRa(message);
      message.empty = 1;
    }
    /*int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
      string outputStr;
      while (LoRa.available())
      {
        outputStr += LoRa.read();
      }

      outputStr += "\n";

      printf(outputStr.c_str());

      packetSize = 0;
    }*/


    gettimeofday(&nowtime, NULL);
    uint32_t nowseconds = (uint32_t)(nowtime.tv_sec);
    if (nowseconds - lasttime >= 30) {
      printf("I'm alive!\n");
      lasttime = nowseconds;
      cp_nb_rx_rcv = 0;
      cp_nb_rx_ok = 0;
      cp_up_pkt_fwd = 0;
    }

    // Let some time to the OS
    delay(1);
  }

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
          } else if (key.compare("pin_nss") == 0) {
            ssPin = confIt->value.GetUint();
          } else if (key.compare("pin_dio0") == 0) {
            dio0 = confIt->value.GetUint();
          } else if (key.compare("pin_rst") == 0) {
            RST = confIt->value.GetUint();
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
//Device id (8 Byte)
//Device type (2 Byte)
//message (Get length)
//Checksum (4 Byte)
void sendLoRa(struct LoRaMessage message) {
  uint8_t messageLength = strlen(message) + 14;
  char output[messagelength] = { '\0' };

	sprintf(output, "%08xl%02d%s", message.deviceID, message.devicetype, message.message);
	
	char check[5] = { '\0' };
	for(uint8_t i = 0; i < strlen(output); i++) {
		message.checksum += output[i];
	}
	sprintf(check, "%04x", message.checksum);
	
	output.append(check);

  LoRa.beginPacket();
  LoRa.write(output, strlen(output));
  LoRa.endPacket();
}

//Device id (8 Byte)
//Device type (2 Byte)
//message (Get length)
//Checksum (2 Byte)
void getLoRa(struct LoRaMessage *message) {
    uint8_t packetSize = LoRa.parsePacket();
    if (packetSize) {
      /*string inputStr;
      while(LoRa.available()) {
        inputStr += LoRa.read();
      }*/
      char input[packetSize] = { '\0' }
      for(uint8_t i = 0; i < packetSize; i++) {
        input[i] = LoRa.read();
      }
  
      char pattern[18];
      uint8_t messageSize = packetSize - 12;
      sprintf(pattern, "t8xlt2dt%dst4x", messageSize);
      pattern[0] = '%';
      pattern[4] = '%';
      pattern[8] = '%';
      pattern[strlen(pattern) - 3] = '%';

      //Split the incoming message
      sscanf(input, pattern, &message->deviceID, &message->devicetype, message->message, &message->checksum);
		
      uint16_t generated_checksum = 0;
      for(uint8_t i = 0; i < packetSize-2; i++) {
        generated_checksum += input[i];
      }
		
      if(generated_checksum == message->checksum) message->empty = 0;
    }

    return message;
}