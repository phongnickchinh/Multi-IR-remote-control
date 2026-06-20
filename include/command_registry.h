#pragma once
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ir_Daikin.h>
extern IRDaikinESP daikin;
struct IrCommand {
  const char* name;
  decode_type_t protocol;
  uint64_t code;
  uint16_t bits;
};

const IrCommand* findCommand(const String& category, const String& name);
String protocolName(decode_type_t protocol);
String hexString(uint64_t value);
String categoryCommandsJson(const String& category);
bool sendCommand(IRsend& irsend, const String& category, const String& name);
