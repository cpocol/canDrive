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
  struct can_frame canMsg;
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
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
