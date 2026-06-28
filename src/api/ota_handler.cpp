#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>

#include "HardwareSerial.h"
#include "config.h"

// LED tích hợp trên ESP32 DOIT DevKit V1
static const uint8_t kOtaLedPin = 2;
static bool otaLedState = false;

// ---------- helpers ----------
static void sendOtaJson(WebServer &server, int code, const String &body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Connection", "close");
  server.send(code, "application/json", body);
}

// ---------- POST /api/ota/begin ----------
// PC gửi JSON hoặc query-params: size (bytes) và md5 (hex string).
// ESP32 kiểm tra dung lượng flash, chuẩn bị Update, trả {"ready":true}.
static void handleOtaBegin(WebServer &server) {
  if (!server.hasArg("size")) {
    sendOtaJson(server, 400,
                "{\"ready\":false,\"error\":\"missing 'size' param\"}");
    return;
  }

  size_t fwSize = (size_t)server.arg("size").toInt();
  String md5 = server.hasArg("md5") ? server.arg("md5") : "";

  if (fwSize == 0) {
    sendOtaJson(server, 400, "{\"ready\":false,\"error\":\"invalid size\"}");
    return;
  }

  // Kiểm tra còn đủ flash không
  if (!Update.begin(fwSize)) {
    String err = Update.errorString();
    Serial.printf("[OTA] Begin error: %s\n", err.c_str());
    sendOtaJson(server, 500, "{\"ready\":false,\"error\":\"" + err + "\"}");
    return;
  }

  // Set MD5 checksum nếu có
  if (md5.length() == 32) {
    Update.setMD5(md5.c_str());
    Serial.printf("[OTA] MD5 expected: %s\n", md5.c_str());
  }

  // Chuẩn bị LED nhấp nháy
  pinMode(kOtaLedPin, OUTPUT);
  otaLedState = false;

  Serial.printf("[OTA] Ready to receive %u bytes\n", fwSize);
  sendOtaJson(server, 200, "{\"ready\":true}");
}

// ---------- POST /api/ota/upload ----------
// PC stream firmware binary trong HTTP body.
// Sử dụng raw upload handler của WebServer để nhận từng chunk.
static void handleOtaUploadDone(WebServer &server) {
  // Handler này được gọi SAU KHI toàn bộ upload hoàn tất.
  if (Update.hasError()) {
    String err = Update.errorString();
    digitalWrite(kOtaLedPin, HIGH); // Bật sáng LED khi lỗi
    Serial.printf("[OTA] FAILED: %s\n", err.c_str());
    sendOtaJson(server, 500, "{\"ok\":false,\"error\":\"" + err + "\"}");
  } else {
    digitalWrite(kOtaLedPin, LOW); // Tắt LED khi thành công
    Serial.println("[OTA] SUCCESS — Restarting...");
    sendOtaJson(server, 200,
                "{\"ok\":true,\"msg\":\"Update thanh cong, dang restart...\"}");
    delay(1000);
    ESP.restart();
  }
}

static void handleOtaUploadData(WebServer &server) {
  HTTPUpload &upload = server.upload();

  switch (upload.status) {
  case UPLOAD_FILE_START:
    Serial.printf("[OTA] Receiving firmware: %s\n", upload.filename.c_str());
    // Update.begin() đã gọi ở /api/ota/begin, không cần gọi lại.
    break;

  case UPLOAD_FILE_WRITE:
    // Nhấp nháy LED mỗi chunk
    otaLedState = !otaLedState;
    digitalWrite(kOtaLedPin, otaLedState ? HIGH : LOW);
    // Ghi từng chunk vào flash
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Serial.printf("[OTA] Write error: %s\n", Update.errorString());
    }
    break;

  case UPLOAD_FILE_END:
    // Kết thúc ghi, xác nhận firmware
    if (Update.end(true)) {
      Serial.printf("[OTA] Upload complete: %u bytes\n", upload.totalSize);
    } else {
      Serial.printf("[OTA] End error: %s\n", Update.errorString());
    }
    break;

  case UPLOAD_FILE_ABORTED:
    digitalWrite(kOtaLedPin, HIGH); // Bật sáng LED khi hủy (lỗi)
    Update.abort();
    Serial.println("[OTA] Upload aborted");
    break;
  }
}

// ---------- Register ----------
void registerOtaRoutes(WebServer &server) {
  server.on("/api/ota/begin", HTTP_POST, [&]() { handleOtaBegin(server); });

  server.on(
      "/api/ota/upload", HTTP_POST,
      [&]() { handleOtaUploadDone(server); }, // onComplete
      [&]() { handleOtaUploadData(server); }  // onUpload (per-chunk)
  );

  Serial.println("[OTA] Routes registered: /api/ota/begin, /api/ota/upload");
}
