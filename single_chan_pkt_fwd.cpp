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

static const int SPI_CHANNEL = 0;

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
uint16_t bw = 125;
uint32_t freq = 868E6; // in Mhz! (868)

// #############################################
// #############################################

/*#define REG_FIFO                    0x00
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_AD         0x0E
#define REG_FIFO_RX_BASE_AD         0x0F
#define REG_RX_NB_BYTES             0x13
#define REG_OPMODE                  0x01
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_MODEM_CONFIG            0x1D
#define REG_MODEM_CONFIG2           0x1E
#define REG_MODEM_CONFIG3           0x26
#define REG_SYMB_TIMEOUT_LSB        0x1F
#define REG_PKT_SNR_VALUE           0x19
#define REG_PAYLOAD_LENGTH          0x22
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_MAX_PAYLOAD_LENGTH      0x23
#define REG_HOP_PERIOD              0x24
#define REG_SYNC_WORD               0x39
#define REG_VERSION                 0x42

#define SX72_MODE_RX_CONTINUOS      0x85
#define SX72_MODE_TX                0x83
#define SX72_MODE_SLEEP             0x80
#define SX72_MODE_STANDBY           0x81

#define PAYLOAD_LENGTH              0x40

// LOW NOISE AMPLIFIER
#define REG_LNA                     0x0C
#define LNA_MAX_GAIN                0x23
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN                0x20

// CONF REG
#define REG1                        0x0A
#define REG2                        0x84

#define SX72_MC2_FSK                0x00
#define SX72_MC2_SF7                0x70
#define SX72_MC2_SF8                0x80
#define SX72_MC2_SF9                0x90
#define SX72_MC2_SF10               0xA0
#define SX72_MC2_SF11               0xB0
#define SX72_MC2_SF12               0xC0

#define SX72_MC1_LOW_DATA_RATE_OPTIMIZE  0x01 // mandated for SF11 and SF12

// FRF
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08

#define FRF_MSB                  0xD9 // 868.1 Mhz
#define FRF_MID                  0x06
#define FRF_LSB                  0x66*/

void LoadConfiguration(string filename);
void PrintConfiguration();

void Die(const char *s)
{
  perror(s);
  exit(1);
}

/*void SelectReceiver()
{
  digitalWrite(ssPin, LOW);
}

void UnselectReceiver()
{
  digitalWrite(ssPin, HIGH);
}

uint8_t ReadRegister(uint8_t addr)
{
  uint8_t spibuf[2];
  spibuf[0] = addr & 0x7F;
  spibuf[1] = 0x00;

  SelectReceiver();
  wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 2);
  UnselectReceiver();

  return spibuf[1];
}

void WriteRegister(uint8_t addr, uint8_t value)
{
  uint8_t spibuf[2];
  spibuf[0] = addr | 0x80;
  spibuf[1] = value;

  SelectReceiver();
  wiringPiSPIDataRW(SPI_CHANNEL, spibuf, 2);
  UnselectReceiver();
}

bool ReceivePkt(char* payload, uint8_t* p_length)
{
  // clear rxDone
  WriteRegister(REG_IRQ_FLAGS, 0x40);

  int irqflags = ReadRegister(REG_IRQ_FLAGS);

  cp_nb_rx_rcv++;

  //  payload crc: 0x20
  if((irqflags & 0x20) == 0x20) {
    printf("CRC error\n");
    WriteRegister(REG_IRQ_FLAGS, 0x20);
    return false;

  } else {
    cp_nb_rx_ok++;
    cp_nb_rx_ok_tot++;

    uint8_t currentAddr = ReadRegister(REG_FIFO_RX_CURRENT_ADDR);
    uint8_t receivedCount = ReadRegister(REG_RX_NB_BYTES);
    *p_length = receivedCount;

    WriteRegister(REG_FIFO_ADDR_PTR, currentAddr);

    for(int i = 0; i < receivedCount; i++) {
      payload[i] = ReadRegister(REG_FIFO);
    }
  }

  return true;
}

char * PinName(int pin, char * buff) {
  strcpy(buff, "unused");
  if (pin != 0xff) {
    sprintf(buff, "%d", pin);
  }
  return buff;
}

void SetupLoRa()
{
  char buff[16];

  printf("Trying to detect module with ");
  printf("NSS=%s "  , PinName(ssPin, buff));
  printf("DIO0=%s " , PinName(dio0 , buff));
  printf("Reset=%s ", PinName(RST  , buff));
  
  // check basic 
  if (ssPin == 0xff || dio0 == 0xff) {
    Die("Bad pin configuration ssPin and dio0 need at least to be defined");
  }

  digitalWrite(RST, LOW);
  delay(100);
  digitalWrite(RST, HIGH);
  delay(100);
  uint8_t version = ReadRegister(REG_VERSION);
  if (version == 0x12) { // sx1276
    printf("SX1276 detected, starting.\n");
  } else {
    printf("Transceiver version 0x%02X\n", version);
    Die("Unrecognized transceiver");
  }

  WriteRegister(REG_OPMODE, SX72_MODE_SLEEP);

  // set frequency
  uint64_t frf = ((uint64_t)freq << 19) / 32000000;
  WriteRegister(REG_FRF_MSB, (uint8_t)(frf >> 16) );
  WriteRegister(REG_FRF_MID, (uint8_t)(frf >> 8) );
  WriteRegister(REG_FRF_LSB, (uint8_t)(frf >> 0) );

  WriteRegister(REG_SYNC_WORD, 0x34); // LoRaWAN public sync word

  if (sf == SF11 || sf == SF12) {
    WriteRegister(REG_MODEM_CONFIG3, 0x0C);
  } else {
    WriteRegister(REG_MODEM_CONFIG3, 0x04);
  }
  WriteRegister(REG_MODEM_CONFIG, 0x72);
  WriteRegister(REG_MODEM_CONFIG2, (sf << 4) | 0x04);

  if (sf == SF10 || sf == SF11 || sf == SF12) {
    WriteRegister(REG_SYMB_TIMEOUT_LSB, 0x05);
  } else {
    WriteRegister(REG_SYMB_TIMEOUT_LSB, 0x08);
  }
  WriteRegister(REG_MAX_PAYLOAD_LENGTH, 0x80);
  WriteRegister(REG_PAYLOAD_LENGTH, PAYLOAD_LENGTH);
  WriteRegister(REG_HOP_PERIOD, 0xFF);
  WriteRegister(REG_FIFO_ADDR_PTR, ReadRegister(REG_FIFO_RX_BASE_AD));

  // Set Continous Receive Mode
  WriteRegister(REG_LNA, LNA_MAX_GAIN);  // max lna gain
  WriteRegister(REG_OPMODE, SX72_MODE_RX_CONTINUOS);
}

bool Receivepacket()
{
  long int SNR;
  int rssicorr;
  bool ret = false;

  if (digitalRead(dio0) == 1) {
    char message[256];
    uint8_t length = 0;
    if (ReceivePkt(message, &length)) {
      // OK got one
      ret = true;

      uint8_t value = ReadRegister(REG_PKT_SNR_VALUE);
      if (value & 0x80) { // The SNR sign bit is 1
        // Invert and divide by 4
        value = ((~value + 1) & 0xFF) >> 2;
        SNR = -value;
      } else {
        // Divide by 4
        SNR = ( value & 0xFF ) >> 2;
      }

      rssicorr = 157;

      printf("Packet RSSI: %d, ", ReadRegister(0x1A) - rssicorr);
      printf("RSSI: %d, ", ReadRegister(0x1B) - rssicorr);
      printf("SNR: %li, ", SNR);
      printf("Length: %hhu Message:'", length);
      for (int i=0; i<length; i++) {
        char c = (char) message[i];
        printf("%c",isprint(c)?c:'.');
      }
      printf("'\n");

      //TODO save the message in the database (don't process it. there will be a programm every hour or so that will process incoming data)
      //create database row
      //start database connection

      fflush(stdout);
    }
  }

  return ret;
}*/

int main()
{
  struct timeval nowtime;
  uint32_t lasttime;

  LoadConfiguration("global_conf.json");
  PrintConfiguration();

  //TODO create database if not yet created

  // Init WiringPI
  wiringPiSetup() ;
  /*pinMode(ssPin, OUTPUT);
  pinMode(dio0, INPUT);
  pinMode(RST, OUTPUT);*/

  // Init SPI
  wiringPiSPISetup(SPI_CHANNEL, 500000);

  // Setup LORA
  //SetupLoRa();
  LoRa.setPins(ssPin, RST, dio0);

  if (LoRa.begin(freq))
  {
    printf("LoRa Initializing OK!");
  }
  else
  {
    printf("Starting LoRa failed!");
  }

  printf("Listening at SF%i on %.6lf Mhz.\n", sf,(double)freq/1000000);
  printf("-----------------------------------\n");

  while(1) { //TODO instead use a interrupt?
    // Packet received ?
    /*if (Receivepacket()) {
      //TODOwe received a packet, what to do now?
    }*/

    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
      string outputStr;
      while (LoRa.available())
      {
        outputStr += LoRa.read(); //LoRa.readString()
      }

      printf(outputStr);

      packetSize = 0;
    }


    gettimeofday(&nowtime, NULL);
    uint32_t nowseconds = (uint32_t)(nowtime.tv_sec);
    if (nowseconds - lasttime >= 30) {
      print("I'm alive!")
      lasttime = nowseconds;
      cp_nb_rx_rcv = 0;
      cp_nb_rx_ok = 0;
      cp_up_pkt_fwd = 0;
    }

    // Let some time to the OS
    delay(1);
  }

  return (0);
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