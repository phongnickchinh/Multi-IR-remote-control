#include <Arduino.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ir_Daikin.h>
#include <esp_wifi.h>

#include "config.h"
#include "api_routes.h"
#include "command_registry.h"
#include "storage.h"
#include "support/ir_sniffer.h"

IRrecv irrecv(kRecvPin);
IRsend irsend(kSendPin);
IRDaikinESP daikin(kSendPin);
decode_results results;
WebServer server(80);

enum SystemMode { MODE_WEB_SERVER, MODE_SNIFFER };
SystemMode currentMode = MODE_WEB_SERVER;

void connectWiFi() {
  if (kWifiSsid[0] == '\0') return;
  
  //Connect to WiFi in Station mode
  WiFi.mode(WIFI_STA); 
  WiFi.begin(kWifiSsid, kWifiPassword);
  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) { 
    Serial.print("WiFi connected: "); 
    Serial.println(WiFi.localIP()); 
    
    //Modem sleep
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    Serial.println("Modem Sleep: KÍCH HOẠT (Giảm nhiệt độ chip)");
    
  } else { 
    Serial.println("WiFi connect failed! (Chỉ chạy chế độ Sniffer hoặc khởi động lại)"); 
  }
}

// This function is only needed if you want to run in AP mode, which is not recommended for this project due to heat issues.
void startAccessPoint() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(kApSsid, kApPassword);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(kBaudRate); delay(200);
  storageBegin();
  irrecv.enableIRIn();
  irsend.begin();
  daikin.begin();
  connectWiFi();
  // startAccessPoint();  //only for AP mode, not needed for Station mode
  
  registerApiRoutes(server, irsend);
  server.begin();
  Serial.println("System Ready.");
}

void loop() {
  // 1. LẮNG NGHE LỆNH TỪ SERIAL MONITOR ĐỂ ĐẢO MODE
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Xóa khoảng trắng thừa
    
    if (input.equalsIgnoreCase("switch")) {
      if (currentMode == MODE_WEB_SERVER) {
        // Đang ở Web -> Đổi sang Sniffer
        Serial.println("\n=========================================");
        Serial.println(">>> ĐANG CHUYỂN SANG CHẾ ĐỘ: HỌC LỆNH (SNIFFER) <<<");
        
        // Tắt toàn bộ cụm Radio WiFi để chống nhiễu
        WiFi.disconnect(true); 
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF); 
        
        currentMode = MODE_SNIFFER;
        Serial.println("- Đã tắt hoàn toàn sóng WiFi.");
        Serial.println("- Mắt thu hồng ngoại đang chạy với 100% sức mạnh CPU.");
        Serial.println("- Hãy chĩa Remote Quạt vào và bấm!");
        Serial.println("=========================================\n");
        
      } else {
        // Đang ở Sniffer -> Đổi sang Web
        Serial.println("\n=========================================");
        Serial.println(">>> ĐANG CHUYỂN SANG CHẾ ĐỘ: ĐIỀU KHIỂN (WEB SERVER) <<<");
        
        // Khởi động lại WiFi như lúc setup
        connectWiFi();
        // startAccessPoint(); không cần cho Station mode
        
        currentMode = MODE_WEB_SERVER;
        Serial.println("- Hệ thống API Web Server đã sẵn sàng.");
        Serial.println("=========================================\n");
      }
    }
  }

  // 2. CHẠY NHIỆM VỤ THEO TỪNG CHẾ ĐỘ
  if (currentMode == MODE_WEB_SERVER) {
    // Chỉ xử lý API, không đọc mắt thu để tránh rác
    server.handleClient();
  } 
  else if (currentMode == MODE_SNIFFER) {
    // Chỉ đọc mắt thu, bỏ qua API
    pollIrSniffer(irrecv, results);
  }
}