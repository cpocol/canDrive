//https://github.com/sandeepmistry/arduino-CAN

// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CAN_H
#define CAN_H

#ifdef ARDUINO_ARCH_ESP32
#include "ESP32SJA1000.h"
#else
#include "my_MCP2515.h"
#endif

#endif
