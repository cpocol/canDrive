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
  Serial.println("Started");

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

  display.clearDisplay();

  display.setCursor(0, 0);
  display.write("000_OBDCarSimulator");

  display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

  display.display();
}

struct can_frame canMsgR;
struct can_frame canMsgW;

void printHex(long num, int digits) {
  if (digits >= 3 && num < 0x100){ Serial.print("0"); }
  if (digits >= 2 && num < 0x10){ Serial.print("0"); }
  Serial.print(num, HEX);
}

void showMessage(bool show, can_frame msg, const char prefix[]) {
  if (!show)
    return;

  Serial.print(prefix); 
  printHex(msg.can_id, 3); // print ID
  Serial.print(" "); 
  Serial.print(msg.can_dlc, HEX); // print DLC
  Serial.print(" ");
  for (int i = 0; i < msg.can_dlc; i++) {  // print the data
    printHex(msg.data[i], 2);
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  int32_t engineSpeedRaw = analogRead(PIN_POTENTIOMETER);
  int engineSpeed = (engineSpeedRaw * 60 /100) * 10;

  if (mcp2515.readMessage(&canMsgR) == MCP2515::ERROR_OK) {
    showMessage(true, canMsgR, "From TESTER: ");

    //go decode the message
    __u32 id = canMsgR.can_id;
    __u8* data = canMsgR.data;
    if (id == 0x7DF || id == 0x7E0) { //broadcast OR assigned ID (of the one and only ECU)
      if (data[0] == 2) { //SAE standard; Number of additional data bytes
        if (data[1] == 0x01) { //Service/Mode 1 => show current data
          if (data[2] == 0x00) { //PIDs supported  [$01 - $20]
            Serial.println(F("Request: PIDs supported [$01 - $20]"));
            canMsgW.can_id  = 0x7E8;
            canMsgW.can_dlc = 8;
            canMsgW.data[0] = 1 + 1 + 4; //Number of additional data bytes
            canMsgW.data[1] = 0x41; //0x40 + Service/Mode 1 => show current data
            canMsgW.data[2] = 0x00; //PIDs supported

            __u32 pidsSupported = 0;
            //Each bit, from MSB to LSB, represents one of the next 32 PIDs and specifies whether that PID is supported => bit = 32 - PID
            pidsSupported |= (__u32(1) << (32 - 0x05)); //Engine coolant temperature => PID 05 is on bit 27 = 32 - 05
            pidsSupported |= (__u32(1) << (32 - 0x0C)); //Engine speed               => PID 0C is on bit 20 = 32 - 0C

            canMsgW.data[3] = __u8(pidsSupported >> 24);
            canMsgW.data[4] = __u8(pidsSupported >> 16);
            canMsgW.data[5] = __u8(pidsSupported >> 8);
            canMsgW.data[6] = __u8(pidsSupported >> 0);

            mcp2515.sendMessage(&canMsgW);
          }
          else
          if (data[2] == 0x04) { //Calculated engine load
            Serial.println(F("Request: Calculated engine load"));
            static uint8_t load = 0;
            load += 1;
            if (load > 250)
              load = 0;

            canMsgW.can_id  = 0x7E8;
            canMsgW.can_dlc = 8;
            canMsgW.data[0] = 1 + 1 + 1; //Number of additional data bytes
            canMsgW.data[1] = 0x41; //0x40 + Service/Mode 1 => show current data
            canMsgW.data[2] = 0x04; //Calculated engine load
            canMsgW.data[3] = load;

            //mcp2515.sendMessage(&canMsgW);
          }
          else
          if (data[2] == 0x05) { //Engine coolant temperature
            Serial.println(F("Request: Engine coolant temperature"));
            static uint8_t temperature = 40 + 0; //subtract 40 for the real engine coolant temperature
            temperature += 1;
            if (temperature > 40 + 99) //0 might be reserved as invalid value
              temperature = 40 + 0;

            canMsgW.can_id  = 0x7E8;
            canMsgW.can_dlc = 8;
            canMsgW.data[0] = 1 + 1 + 1; //Number of additional data bytes; 3 to 6 according to docs
            canMsgW.data[1] = 0x41; //0x40 + Service/Mode 1 => show current data
            canMsgW.data[2] = 0x05; //Engine coolant temperature
            canMsgW.data[3] = temperature;

            mcp2515.sendMessage(&canMsgW);
          }
          else
          if (data[2] == 0x0C) { //Engine speed
            Serial.println(F("Request: Engine speed"));
            // static uint16_t rpm = 1000 * 4;
            // rpm += 100;
            // if (rpm > 4000 * 4)
            //   rpm = 1000 * 4;

            canMsgW.can_id  = 0x7E8;
            canMsgW.can_dlc = 8;
            canMsgW.data[0] = 1 + 1 + 2; //Number of additional data bytes
            canMsgW.data[1] = 0x41; //0x40 + Service/Mode 1 => show current data
            canMsgW.data[2] = 0x0C; //Engine speed
            int16_t engineSpeedCAN = 4 * engineSpeed;
            canMsgW.data[3] = engineSpeedCAN / 256;
            canMsgW.data[4] = engineSpeedCAN % 256;

            mcp2515.sendMessage(&canMsgW);
          }
          else
          if (data[2] == 0x0D) { //Vehicle speed
            Serial.println(F("Request: Vehicle speed"));
            static uint16_t speed = 1;
            speed += 1;
            if (speed > 250)
              speed = 250;

            canMsgW.can_id  = 0x7E8;
            canMsgW.can_dlc = 8;
            canMsgW.data[0] = 1 + 1 + 1; //Number of additional data bytes
            canMsgW.data[1] = 0x41; //0x40 + Service/Mode 1 => show current data
            canMsgW.data[2] = 0x0D; //Vehicle speed
            canMsgW.data[3] = speed;

            //mcp2515.sendMessage(&canMsgW);
          }
          else
          if (data[2] == 0x20) { //PIDs supported  [$21 - $40]
            Serial.println(F("Request: PIDs supported [$21 - $40]"));
            canMsgW.can_id  = 0x7E8;
            canMsgW.can_dlc = 8;
            canMsgW.data[0] = 1 + 1 + 4; //Number of additional data bytes
            canMsgW.data[1] = 0x41; //0x40 + Service/Mode 1 => show current data
            canMsgW.data[2] = 0x20; //PIDs supported

            __u32 pidsSupported = 0;
            //pidsSupported |= (__u32(1) << (32 - 5)); //abc => PID x is on bit 32 - x = 27

            canMsgW.data[3] = __u8(pidsSupported > 24);
            canMsgW.data[4] = __u8(pidsSupported > 16);
            canMsgW.data[5] = __u8(pidsSupported > 8);
            canMsgW.data[6] = __u8(pidsSupported > 0);

            //mcp2515.sendMessage(&canMsgW);
          }
        } //Service/Mode 1 => show current data
      }
    }
  }
  // /////////////// display ////////////////

  // display.clearDisplay();

  // display.setCursor(5, 0);
  // display.write("Simple Car Simulator");

  // display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

  // sprintf(str, "Eng. temperature: %2d%c", temperature - 40, 248);
  // display.setCursor(0, 15);
  // display.write(str);

  // display.display();

  // delay(500);
}
