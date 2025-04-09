// Developer: 
//        Adam Varga, 2020, All rights reserved.
// Licence: 
//        Licenced under the MIT licence. See LICENCE file in the project root.
// Usage of this code: 
//        This code creates the interface between the car
//        and the canSniffer_GUI application. If the RANDOM_CAN
//        define is set to 1, this code is generating random
//        CAN packets in order to test the higher level code.
//        The received packets will be echoed back. If the 
//        RANDOM_CAN define is set to 0, the CAN_SPEED define 
//        has to match the speed of the desired CAN channel in
//        order to receive and transfer from and to the CAN bus.
//        Serial speed is 250000baud <- might need to be increased.
// Required arduino packages: 
//        - CAN by Sandeep Mistry (https://github.com/sandeepmistry/arduino-CAN)
// Required modifications: 
//        - MCP2515.h: 16e6 clock frequency reduced to 8e6 (depending on MCP2515 clock)
//        - MCP2515.cpp: extend CNF_MAPPER with your desired CAN speeds

////////////////////////////////  OLED  ///////////////////////////////////////////////////////////////////////////
#define USE_DISPLAY 1

#if USE_DISPLAY == 1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#include <Wire.h>
#include "my_Adafruit_SSD1306.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
uint8_t displayBuffer[SCREEN_WIDTH * ((SCREEN_HEIGHT + 7) / 8)];

char str[50];
#endif

int engineTemperature = -41; //not valid, yet
int engineSpeed = -1; //not valid, yet

////////////////////////////////  CAN  ///////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
#include "my_CAN.h"
//------------------------------------------------------------------------------
// Settings
#define RANDOM_CAN 0
//OBD CAN
#define CAN_SPEED (500E3) //LOW=33E3, MID=95E3, HIGH=500E3 (for Vectra); 500E3 (for Dacia)
//#define CAN_SPEED (80E3) //Multimedia CAN

//Multimedia CAN
//#define CAN_SPEED (500E3) // for Dacia, it works with 500E3
// #define CAN_SPEED (250E3)
// #define CAN_SPEED (250E3)
// #define CAN_SPEED (200E3)
// #define CAN_SPEED (125E3)
// #define CAN_SPEED (100E3)
// #define CAN_SPEED (95E3)
// #define CAN_SPEED (83E3)
// #define CAN_SPEED (80E3)
// #define CAN_SPEED (50E3)
// #define CAN_SPEED (40E3)
// #define CAN_SPEED (33E3)
// #define CAN_SPEED (20E3)
// #define CAN_SPEED (10E3)
// #define CAN_SPEED (5E3)

//------------------------------------------------------------------------------
// Inits, globals
typedef struct {
  long id;
  byte rtr;
  byte ide;
  byte dlc;
  byte dataArray[20];
} packet_t;

const char SEPARATOR = ',';
const char TERMINATOR = '\n';
const char RXBUF_LEN = 100;

//------------------------------------------------------------------------------
// Printing a packet to serial
void printHex(long num) {
  if ( num < 0x10 ){ Serial.print("0"); }
  Serial.print(num, HEX);
}

void printPacket(packet_t * packet) {
  // packet format (hex string): [ID],[RTR],[IDE],[DATABYTES 0..8B]\n
  // example: 014A,00,00,1A002B003C004D\n
  printHex(packet->id);
  Serial.print(SEPARATOR);
  printHex(packet->rtr);
  Serial.print(SEPARATOR);
  printHex(packet->ide);
  Serial.print(SEPARATOR);
  // DLC is determinded by number of data bytes, format: [00]
  for (int i = 0; i < packet->dlc; i++) {
    printHex(packet->dataArray[i]);
  }
  Serial.print(TERMINATOR);
}
//------------------------------------------------------------------------------
// CAN packet simulator
void CANsimulate(void) {
  packet_t txPacket;

  long sampleIdList[] = {0x110, 0x18DAF111, 0x23A, 0x257, 0x412F1A1, 0x601, 0x18EA0C11};
  int idIndex = random (sizeof(sampleIdList) / sizeof(sampleIdList[0]));
  int sampleData[] = {0xA, 0x1B, 0x2C, 0x3D, 0x4E, 0x5F, 0xA0, 0xB1};

  txPacket.id = sampleIdList[idIndex];
  txPacket.ide = txPacket.id > 0x7FF ? 1 : 0;
  txPacket.rtr = 0; //random(2);
  txPacket.dlc = random(1, 9);

  for (int i = 0; i < txPacket.dlc ; i++) {
    int changeByte = random(4);
    if (changeByte == 0) {
      sampleData[i] = random(256);
    }
    txPacket.dataArray[i] = sampleData[i];
  }

  printPacket(&txPacket);
}
//------------------------------------------------------------------------------
// CAN RX, TX
void onCANReceive(int packetSize) {
  // received a CAN packet
  packet_t rxPacket;
  rxPacket.id = CAN.packetId();
  rxPacket.rtr = CAN.packetRtr() ? 1 : 0;
  rxPacket.ide = CAN.packetExtended() ? 1 : 0;
  rxPacket.dlc = CAN.packetDlc();
  byte i = 0;
  while (CAN.available()) {
    rxPacket.dataArray[i++] = CAN.read();
    if (i >= (sizeof(rxPacket.dataArray) / (sizeof(rxPacket.dataArray[0])))) {
      break;
    }
  }

  if (rxPacket.id == 0x5DA) {
    engineTemperature = rxPacket.dataArray[0] - 40;
  }

  if (rxPacket.id == 0x186) {
    engineSpeed = ((int)256 * rxPacket.dataArray[0] + rxPacket.dataArray[1]) / 4;
  }

  printPacket(&rxPacket);
}

void sendPacketToCan(packet_t * packet) {
  for (int retries = 10; retries > 0; retries--) {
    bool rtr = packet->rtr ? true : false;
    if (packet->ide){
      CAN.beginExtendedPacket(packet->id, packet->dlc, rtr);
    } else {
      CAN.beginPacket(packet->id, packet->dlc, rtr);
    }
    CAN.write(packet->dataArray, packet->dlc);
    if (CAN.endPacket()) {
      // success
      break;
    } else if (retries <= 1) {
      return;
    }
  }
}
//------------------------------------------------------------------------------
// Serial parser
char getNum(char c) {
  if (c >= '0' && c <= '9') { return c - '0'; }
  if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
  if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
  return 0;
}

char * strToHex(char * str, byte * hexArray, byte * len) {
  byte *ptr = hexArray;
  char * idx;
  for (idx = str ; *idx != SEPARATOR && *idx != TERMINATOR; ++idx, ++ptr ) {
    *ptr = (getNum( *idx++ ) << 4) + getNum( *idx );
  }
  *len = ptr - hexArray;
  return idx;
}

void rxParse(char * buf, int len) {
  packet_t rxPacket;
  char * ptr = buf;
  // All elements have to have leading zero!

  // ID
  byte idTempArray[8], tempLen;
  ptr = strToHex(ptr, idTempArray, &tempLen);
  rxPacket.id = 0;
  for (int i = 0; i < tempLen; i++) {
    rxPacket.id |= (long)idTempArray[i] << ((tempLen - i - 1) * 8);
  }

  // RTR
  ptr = strToHex(ptr + 1, &rxPacket.rtr, &tempLen);

  // IDE
  ptr = strToHex(ptr + 1, &rxPacket.ide, &tempLen);

  // DATA
  ptr = strToHex(ptr + 1, rxPacket.dataArray, &rxPacket.dlc);

#if RANDOM_CAN == 1
  // echo back
  printPacket(&rxPacket);
#else
  sendPacketToCan(&rxPacket);
#endif
}

void RXcallback(void) {
  static int rxPtr = 0;
  static char rxBuf[RXBUF_LEN];

  while (Serial.available() > 0) {
    if (rxPtr >= RXBUF_LEN) {
      rxPtr = 0;
    }
    char c = Serial.read();
    rxBuf[rxPtr++] = c;
    if (c == TERMINATOR) {
      rxParse(rxBuf, rxPtr);
      rxPtr = 0;
    }
  }
}

//------------------------------------------------------------------------------
// Setup
void setup() {
  Serial.begin(250000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

#if RANDOM_CAN == 1
  randomSeed(12345);
  Serial.println(F("randomCAN Started"));
#else
  //Serial.println(CAN_SPEED);
  if (!CAN.begin(CAN_SPEED)) {
    Serial.println(F("Starting CAN failed!"));
    while (1);
  }
  // register the receive callback
  CAN.onReceive(onCANReceive);
  Serial.println(F("CAN RX TX Started"));
#endif

#if USE_DISPLAY == 1
  display.setExternalBuffer(displayBuffer);
  if(!display.begin())
    Serial.println(F("!SSD1306"));

  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextWrap(false);

  display.clearDisplay();

  display.setCursor(0, 0);
  display.write("01_canSniffer_Arduino");

  display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

  display.display();
#endif
}
//------------------------------------------------------------------------------
// Main
void loop() {
#if USE_DISPLAY == 1
  //Serial.println(F("clear display"));
  display.clearDisplay();

  display.setCursor(0, 0);
  display.write("01_canSniffer_Arduino");

  display.drawFastHLine(0, 10, SCREEN_WIDTH, SSD1306_WHITE);

  bool refreshDisplay = false;
#endif

  RXcallback(); //gets message from Serial and sends it to CAN
#if RANDOM_CAN == 1
  CANsimulate();
  delay(100);
#else
  static int lastEngineTemperature = -41; //not valid yet
  if (engineTemperature != lastEngineTemperature) {
    lastEngineTemperature = engineTemperature;
    Serial.print(F("Engine temperature: "));
    Serial.println(engineTemperature);

#if USE_DISPLAY == 1
    sprintf(str, "Eng. temperature: %2d%c", engineTemperature, 248);
    display.setCursor(0, 14);
    display.write(str);
    refreshDisplay = true;
#endif
  }

  static int lastEngineSpeed = -1; //not valid yet
  if (engineSpeed != lastEngineSpeed) {
    lastEngineSpeed = engineSpeed;
    Serial.print(F("Engine speed: "));
    Serial.println(engineSpeed);

#if USE_DISPLAY == 1
    refreshDisplay = true;
#endif
  }

#if USE_DISPLAY == 1
  if (refreshDisplay)
  {
    sprintf(str, "Eng. speed: %4d rpm", engineSpeed);
    display.setCursor(0, 24);
    display.write(str);
  }
#endif

#endif //RANDOM_CAN == 1

#if USE_DISPLAY == 1
  if (refreshDisplay)
    display.display();
#endif
}
