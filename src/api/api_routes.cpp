#include <WebServer.h>
#include <WiFi.h>
#include <ir_Daikin.h>

#include "command_registry.h"
#include "config.h"
#include "index_html.h" 

extern IRDaikinESP daikin;

static void sendJson(WebServer& server, int code, const String& body) {
  server.sendHeader("Access-Control-Allow-Origin", "*"); 
  server.send(code, "application/json", body);
}

static void handleRoot(WebServer& server) {
  server.send(200, "text/html", WEB_UI);
}

static void handleStatus(WebServer& server) {
  String body = "{\"ok\":true,\"ssid\":\"" + (WiFi.isConnected() ? WiFi.SSID() : String(kApSsid)) + "\"}";
  sendJson(server, 200, body);
}

static void handleSend(WebServer& server, IRsend& irsend) {
  if (!server.hasArg("category") || !server.hasArg("command")) {
    sendJson(server, 400, "{\"ok\":false,\"error\":\"missing_args\"}"); return;
  }
  
  String category = server.arg("category");
  String name = server.arg("command");

  // --- LUỒNG 1: XỬ LÝ ĐIỀU HÒA DAIKIN ---
  if (category.equalsIgnoreCase("daikin")) {
    if (name == "power_off") {
      daikin.off();
      daikin.send();
      sendJson(server, 200, "{\"ok\":true,\"msg\":\"Đã tắt điều hòa\"}");
    } 
    else if (name == "set_state") {
      daikin.on();
      
      // Đọc Mode
      String mode = server.arg("mode");
      if (mode == "cool") daikin.setMode(kDaikinCool);
      else if (mode == "dry") daikin.setMode(kDaikinDry);
      else if (mode == "fan") daikin.setMode(kDaikinFan);
      
      // Đọc Nhiệt độ & Quạt
      if (server.hasArg("temp")) daikin.setTemp(server.arg("temp").toInt());
      
      String fan = server.arg("fan");
      if (fan == "auto") daikin.setFan(kDaikinFanAuto);
      else if (fan == "quiet") daikin.setFan(kDaikinFanQuiet);
      else daikin.setFan(fan.toInt());

      // Đọc Cánh vẫy & Powerful
      daikin.setSwingVertical(server.arg("swing") == "true");
      daikin.setPowerful(server.arg("powerful") == "true");

      // BỔ SUNG: Đọc Hẹn Giờ (On/Off Timer)
      if (server.hasArg("on_timer")) {
          int on_mins = server.arg("on_timer").toInt();
          if (on_mins > 0) daikin.enableOnTimer(on_mins);
          else daikin.disableOnTimer();
      }
      
      if (server.hasArg("off_timer")) {
          int off_mins = server.arg("off_timer").toInt();
          if (off_mins > 0) daikin.enableOffTimer(off_mins);
          else daikin.disableOffTimer();
      }

      daikin.send(); // Bắn tín hiệu hồng ngoại
      sendJson(server, 200, "{\"ok\":true,\"msg\":\"Đã gửi lệnh Daikin\"}");
    }
    return;
  }

  // --- LUỒNG 2: XỬ LÝ ĐÈN & QUẠT ---
  if (sendCommand(irsend, category, name)) {
    sendJson(server, 200, "{\"ok\":true,\"msg\":\"Đã gửi lệnh " + name + "\"}");
  } else {
    sendJson(server, 404, "{\"ok\":false,\"error\":\"Lệnh không tồn tại\"}");
  }
}

void registerApiRoutes(WebServer& server, IRsend& irsend) {
  server.on("/", HTTP_GET, [&](){ handleRoot(server); });
  server.on("/api/status", HTTP_GET, [&](){ handleStatus(server); });
  server.on("/api/ir/send", HTTP_GET, [&](){ handleSend(server, irsend); });
}