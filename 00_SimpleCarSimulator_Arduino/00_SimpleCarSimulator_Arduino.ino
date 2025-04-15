#include "my_mcp2515.h" //https://github.com/autowp/arduino-mcp2515

MCP2515 mcp2515(10); //CS

#define PIN_POTENTIOMETER A0

////////////////////////////////  OLED  ///////////////////////////////////////////////////////////////////////////
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#include <Wire.h>
#include "my_Adafruit_SSD1306.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
uint8_t displayBuffer[SCREEN_WIDTH * ((SCREEN_HEIGHT + 7) / 8)];

char str[50];

void printHex(long num) {
  if (num < 0x10)
    Serial.print("0");
  Serial.print(num, HEX);
}

void printMessage(can_frame msg) {
  const char SEPARATOR = ',';
  const char TERMINATOR = '\n';

  printHex(msg.can_id);
  Serial.print(SEPARATOR);
  printHex(msg.can_rtr);
  Serial.print(SEPARATOR);
  printHex(msg.can_ide);
  Serial.print(SEPARATOR);
  // DLC is determinded by number of data bytes, format: [00]
  for (int i = 0; i < msg.can_dlc; i++) {
    printHex(msg.data[i]);
  }
  Serial.print(TERMINATOR);
}

void setup() {
  Serial.begin(250000);
  Serial.println(F("00_SimpleCarSimulator_Arduino"));

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  pinMode(PIN_POTENTIOMETER, INPUT);
  
  display.setExternalBuffer(displayBuffer);
  if(!display.begin())
      Serial.println(F("!SSD1306"));

  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextWrap(false);
}

struct can_frame canMsgW;
int sentMsgRate = 0;

void loop() {
///// Engine temperature
  static uint8_t engineTemperature = 40 + 0; //subtract 40 for the real engine coolant temperature
  engineTemperature += 1;
  if (engineTemperature > 40 + 99) //0 might be reserved as invalid value
    engineTemperature = 40 + 0;

  //Serial.print("Engine temperature: ");   Serial.println(int(engineTemperature));

  canMsgW.can_id  = 0x5DA;
  canMsgW.can_dlc = 8;
  canMsgW.data[0] = engineTemperature;

  mcp2515.sendMessage(&canMsgW);
  //Serial.println("Temperature message sent");
  sentMsgRate++;

  //send via Serial as well in the format for 02_canSniffer_GUI
  //printMessage(canMsgW);

  delay(5); //some delay between messages seems to be necessary

///// engine speed
  int16_t engineSpeedRaw = analogRead(PIN_POTENTIOMETER);
  int16_t engineSpeed = engineSpeedRaw * 6; //upto 6000+ rpm

  //Serial.print("Engine speed: ");   Serial.println(int(engineSpeed));

  canMsgW.can_id  = 0x186;
  canMsgW.can_dlc = 8;
  int16_t engineSpeedCAN = 4 * engineSpeed;
  canMsgW.data[0] = engineSpeedCAN / 256;
  canMsgW.data[1] = engineSpeedCAN % 256;

  mcp2515.sendMessage(&canMsgW);
  //Serial.println("Engine speed message sent");
  sentMsgRate++;

  //send via Serial as well in the format for 02_canSniffer_GUI
  //printMessage(canMsgW);

/////////////// display ////////////////

  // display.clearDisplay();

  // display.setCursor(5, 0);
  // display.write("Simple Car Simulator");

  // display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

  // sprintf(str, "Eng. temperature: %2d%c", engineTemperature - 40, 248);
  // display.setCursor(0, 14);
  // display.write(str);

  // sprintf(str, "Eng. speed: %4d rpm", engineSpeed);
  // display.setCursor(0, 24);
  // display.write(str);

  // display.display();

  delay(5);

  // stats
  static auto time = millis();
  if (millis() - time > 1000) {
      time = time + 1000;

      Serial.print(F("Sent messages per second "));   Serial.println(sentMsgRate);

      sentMsgRate = 0;
  }
}
