#include <Arduino.h>
#include <LoRa.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include "main.h"
#include "LoRaCrypting.h"

String inputString = ""; // a String to hold incoming data

struct LoRaMessage *messageOut;
struct LoRaMessage *messageIn;

// 128x64 display
SSD1306Wire display(0x3c, SDA, SCL);  // ADDRESS, SDA, SCL

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  /*//SPI LoRa pins
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  //setup LoRa transceiver module
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);

  pinMode(A0, INPUT);

  if (LoRa.begin(BAND))
  {
    Serial.println("LoRa Initializing OK!");
  }
  else
  {
    Serial.println("Starting LoRa failed!");
  }*/

  //set spreading factor
  //LoRa.setSpreadingFactor(7);
  //set signal bandwidth
  //LoRa.setSignalBandwidth(125E3);

  //start lora
  initLoRa();

  // allocating memory for n numbers of struct person
  messageOut = (struct LoRaMessage*) malloc(sizeof(struct LoRaMessage));
  messageOut->empty = 1;
  messageOut->deviceID = ESP.getEfuseMac();
  messageOut->deviceType = 1;
  // allocating memory for n numbers of struct person
  messageIn = (struct LoRaMessage*) malloc(sizeof(struct LoRaMessage));
  messageIn->empty = 1;

  pinMode(A0, INPUT);

  display.init();
  display.flipScreenVertically();
}

void loop()
{
  if(!digitalRead(A0)) {
    sprintf(messageOut->message, "Hello World!");
    sendLoRa(messageOut);
    while(!digitalRead(A0)) { delay(1); }
  }

  getLoRa(messageIn);
  if(!messageIn->empty) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    char str[100] = { '0' };
    sprintf(str, "Message: %s", messageIn->message);
    display.drawStringMaxWidth(0, 0, 128, String(str));
    sprintf(str, "Rssi: %d \nTime: %lu", LoRa.packetRssi(), String(millis()));
    display.drawString(0, 44, String(str));
    display.display();

    messageIn->empty = 1;
  }
 /*if(!digitalRead(A0)) {
    LoRa.beginPacket();
    LoRa.print("2ff3d2a501Hello World!");
    LoRa.endPacket();
    while(!digitalRead(A0)) { delay(1); }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    String outputStr;
    while (LoRa.available())
    {
      outputStr += LoRa.readString();
    }

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    char str[100] = { '0' };
    sprintf(str, "Message: %s", outputStr);
    display.drawStringMaxWidth(0, 0, 128, String(str));
    sprintf(str, "Rssi: %d \nTime: %lu", LoRa.packetRssi(), String(millis()));
    display.drawString(0, 44, String(str));
    display.display();

    packetSize = 0;
  }*/
}