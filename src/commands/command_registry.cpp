#include "command_registry.h"

static const IrCommand kLightCommands[] = {
    {"toggle", NEC, 0x807F00FF, 32},
    {"temp_down", NEC, 0x807F40BF, 32},
    {"temp_up", NEC, 0x807F50AF, 32},
    {"mode_3", NEC, 0x807F807F, 32},
    {"brightness_down", NEC, 0x807F20DF, 32},
};
static const size_t kLightCommandCount = sizeof(kLightCommands) / sizeof(kLightCommands[0]);

const IrCommand* findCommand(const String& category, const String& name) {
  if (category.equalsIgnoreCase("light")) {
    for (size_t i = 0; i < kLightCommandCount; i++) {
      if (name.equalsIgnoreCase(kLightCommands[i].name)) return &kLightCommands[i];
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
  body += "]";
  return body;
}

bool sendCommand(IRsend& irsend, const String& category, const String& name) {
  const IrCommand* cmd = findCommand(category, name);
  if (!cmd || cmd->protocol != NEC) return false;
  irsend.sendNEC(cmd->code, cmd->bits);
  return true;
}
