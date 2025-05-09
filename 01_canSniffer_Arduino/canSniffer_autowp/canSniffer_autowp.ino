#include "my_mcp2515.h" //https://github.com/autowp/arduino-mcp2515

MCP2515 mcp2515(10);

void setup() {
  Serial.begin(250000);
  Serial.println(F("canSniffer_autowp started"));

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
}

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

int receivedMsgRate = 0;

void loop() {
  struct can_frame canMsgW;

  // static uint16_t rpm = 0;
  // rpm++;
  // canMsgW.can_id  = 0x186; //EngineRPM
  // canMsgW.can_dlc = 7;
  // canMsgW.data[0] = rpm/256;
  // canMsgW.data[1] = 0x00;
  // canMsgW.data[2] = 0x00;
  // canMsgW.data[3] = 0x00;
  // canMsgW.data[4] = 0x00;
  // canMsgW.data[5] = 0x00;
  // canMsgW.data[6] = 0x00;
  // canMsgW.data[7] = 0x00;

  // static uint16_t speed = 0;
  // int factor = 3;
  // for (int i = 0; i < 300; i++) //simmulate delay
  //   if (i < 1)
  //     speed++;
  // if (speed > factor*21000)
  //   speed = factor*21000;

  // canMsgW.can_id  = 0x217; //Speed
  // canMsgW.can_dlc = 7;
  // canMsgW.data[0] = 0x00;
  // canMsgW.data[1] = 0x00;
  // canMsgW.data[2] = 0x00;
  // canMsgW.data[3] = speed/100/factor;
  // canMsgW.data[4] = 0x00;
  // canMsgW.data[5] = 0x00;
  // canMsgW.data[6] = 0x00;
  // canMsgW.data[7] = 0x00;

  // canMsgW.can_id  = 0x5DE; //Lights
  // canMsgW.can_dlc = 8;
  // //canMsgW.data[0] = 0b01111111;
  // canMsgW.data[0] = 0b00000000;
  // canMsgW.data[1] = 0x00;
  // canMsgW.data[2] = 0x00;
  // canMsgW.data[3] = 0x00;
  // canMsgW.data[4] = 0x00;
  // canMsgW.data[5] = 0x00;
  // canMsgW.data[6] = 0x00;
  // canMsgW.data[7] = 0x00;

  // canMsgW.can_id  = 0x743; //Odo request
  // canMsgW.can_dlc = 8;
  // canMsgW.data[0] = 0x03;
  // canMsgW.data[1] = 0x22;
  // canMsgW.data[2] = 0x02;
  // canMsgW.data[3] = 0x07;
  // canMsgW.data[4] = 0x00;
  // canMsgW.data[5] = 0x00;
  // canMsgW.data[6] = 0x00;
  // canMsgW.data[7] = 0x00;

  // canMsgW.can_id  = 0x7DF; //OBD request
  // canMsgW.can_dlc = 8;
  // canMsgW.data[0] = 1 + 1; //Number of additional data bytes
  // canMsgW.data[1] = 0x01; //Service/Mode 1 => show current data
  // canMsgW.data[2] = 0x05; //Engine coolant temperature

  //mcp2515.sendMessage(&canMsgW);
  //delay(1);

  struct can_frame canMsg;
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    //if (canMsg.can_id == 0x0763) //Odo response
    //if (canMsg.can_id > 0x700)
      printMessage(canMsg);
    receivedMsgRate++;
  }

  // stats
  static auto time = millis();
  if (millis() - time > 1000) {
      time = time + 1000;

      // !!! may crash 02_canSniffer_GUI !!!
      //Serial.print(F("Received messages per second "));   Serial.println(receivedMsgRate);

      //bus load:
      //- one CAN frame/message has at most 132 bits (inter frame spacing included), according to https://en.wikipedia.org/wiki/CAN_bus
      //- for CAN speed 500 kbps and 1000 messages per second => 26%

      receivedMsgRate = 0;
  }
}
