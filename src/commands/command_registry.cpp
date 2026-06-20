#include "command_registry.h"

static const IrCommand kLightCommands[] = {
    {"toggle", NEC, 0x807F00FF, 32},
    {"temp_down", NEC, 0x807F40BF, 32},
    {"temp_up", NEC, 0x807F50AF, 32},
    {"mode_3", NEC, 0x807F807F, 32},
    {"brightness_down", NEC, 0x807F20DF, 32},
};
static const size_t kLightCommandCount = sizeof(kLightCommands) / sizeof(kLightCommands[0]);

static const IrCommand kFanCommands[] = {
    {"toggle", PANASONIC, 0x40040B210C8C, 48},
    {"mode_1", PANASONIC, 0x40040B210C84, 48},
    {"mode_2", PANASONIC, 0x40040B210C44, 48},
    {"mode_3", PANASONIC, 0x40040B210CC4, 48},
    {"timer_1h", PANASONIC, 0x40040B210C82, 48},
    {"timer_3h", PANASONIC, 0x40040B210CC2, 48},
    {"timer_6h", PANASONIC, 0x40040B210C62, 48},
    {"cancel_timer", PANASONIC, 0x40040B210C02, 48},
    {"sleep_mode", PANASONIC, 0x40040B210C72, 48},
};
static const size_t kFanCommandCount = sizeof(kFanCommands) / sizeof(kFanCommands[0]);

const IrCommand* findCommand(const String& category, const String& name) {
  if (category.equalsIgnoreCase("light")) {
    for (size_t i = 0; i < kLightCommandCount; i++) {
      if (name.equalsIgnoreCase(kLightCommands[i].name)) return &kLightCommands[i];
    }
  } 
  // Thêm nhánh tìm kiếm cho Quạt
  else if (category.equalsIgnoreCase("fan")) {
    for (size_t i = 0; i < kFanCommandCount; i++) {
      if (name.equalsIgnoreCase(kFanCommands[i].name)) return &kFanCommands[i];
    }
  }
  return nullptr;
}

String protocolName(decode_type_t protocol) { return String(typeToString(protocol)); }
String hexString(uint64_t value) {
  char buffer[21];
  snprintf(buffer, sizeof(buffer), "0x%llX", (unsigned long long)value);
  return String(buffer);
}

String categoryCommandsJson(const String& category) {
  String body = "[";
  if (category.equalsIgnoreCase("light")) {
    for (size_t i = 0; i < kLightCommandCount; i++) {
      if (i > 0) body += ",";
      body += "\""; body += kLightCommands[i].name; body += "\"";
    }
  } 
  // Thêm nhánh JSON cho Quạt
  else if (category.equalsIgnoreCase("fan")) {
    for (size_t i = 0; i < kFanCommandCount; i++) {
      if (i > 0) body += ",";
      body += "\""; body += kFanCommands[i].name; body += "\"";
    }
  }
  body += "]";
  return body;
}

bool sendCommand(IRsend& irsend, const String& category, const String& name) {
  // Chỉ tìm và bắn lệnh cho Đèn và Quạt
  if (category.equalsIgnoreCase("light") || category.equalsIgnoreCase("fan")) {
    const IrCommand* cmd = findCommand(category, name);
    if (!cmd) return false;

    if (cmd->protocol == NEC) {
      irsend.sendNEC(cmd->code, cmd->bits);
      return true;
    } else if (cmd->protocol == PANASONIC) {
      irsend.sendPanasonic64(cmd->code, cmd->bits, 2);
      return true;
    }
  } 
  
  return false;
}
