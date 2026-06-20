#include "support/ir_sniffer.h"
#include <Arduino.h>
#include <IRutils.h>

void pollIrSniffer(IRrecv& irrecv, decode_results& results) {
  if (irrecv.decode(&results)) {
    if (results.decode_type != UNKNOWN) {
      Serial.print("Protocol: "); Serial.println(typeToString(results.decode_type));
      Serial.print("HEX: 0x"); Serial.println(results.value, HEX); Serial.println();
    }
    irrecv.resume();
  }
}
