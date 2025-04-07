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

void setup() {
  Serial.begin(250000);
  Serial.println("setup -------------------------------");

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

void loop() {
///// temperature
  static uint8_t temperature = 40 + 0; //subtract 40 for the real engine coolant temperature
  temperature += 1;
  if (temperature > 40 + 99) //0 might be reserved as invalid value
    temperature = 40 + 0;

  Serial.print("Temperature: ");   Serial.println(int(temperature));

  canMsgW.can_id  = 0x5DA;
  canMsgW.can_dlc = 8;
  canMsgW.data[0] = temperature;

  mcp2515.sendMessage(&canMsgW);

  Serial.println("Temperature message sent");

///// engine speed
  int32_t engineSpeedRaw = analogRead(PIN_POTENTIOMETER);
  int engineSpeed = (engineSpeedRaw * 70 /100) * 10;

  Serial.print("Engine speed: ");   Serial.println(int(engineSpeed));

  canMsgW.can_id  = 0x186;
  canMsgW.can_dlc = 8;
  int16_t engineSpeedCAN = 4 * engineSpeed;
  canMsgW.data[0] = engineSpeedCAN / 256;
  canMsgW.data[1] = engineSpeedCAN % 256;

  mcp2515.sendMessage(&canMsgW);

  Serial.println("Engine speed sent");

/////////////// display ////////////////

  display.clearDisplay();

  display.setCursor(5, 0);
  display.write("Simple Car Simulator");

  display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

  sprintf(str, "Eng. temperature: %2d%c", temperature - 40, 248);
  display.setCursor(0, 14);
  display.write(str);

  sprintf(str, "Eng. speed: %4d rpm", engineSpeed);
  display.setCursor(0, 24);
  display.write(str);

  display.display();

  delay(500);
}
