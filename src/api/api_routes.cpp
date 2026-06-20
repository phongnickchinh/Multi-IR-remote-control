#include "api_routes.h"
#include <WiFi.h>
#include "command_registry.h"
#include "config.h"

//TODO: Consider moving this HTML to a separate file in SPIFFS or LittleFS for better maintainability.
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Smart Home</title>
    <style>
        body { font-family: sans-serif; background: #f4f4f5; padding: 20px; display: flex; justify-content: center; }
        .card { background: white; padding: 24px; border-radius: 12px; width: 100%; max-width: 500px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 20px; }
        select { width: 100%; padding: 12px; margin-bottom: 20px; border-radius: 8px; border: 1px solid #d1d5db; font-size: 16px; font-weight: bold; color: #1f2937; }
        button { padding: 15px; border: none; border-radius: 8px; background: #3b82f6; color: white; font-size: 16px; cursor: pointer; transition: 0.1s; }
        button:active { transform: scale(0.95); }
        button.btn-toggle { grid-column: span 2; background: #ef4444; }
        pre { background: #1e293b; color: #a5b4fc; padding: 16px; border-radius: 8px; font-size: 13px; overflow-x: auto; min-height: 60px; }
    </style>
</head>
<body>
    <div class="card">
        <h2 style="text-align: center; margin-top: 0;">Smart Remote</h2>
        
        <select id="deviceSelector">
            <option value="light">💡 Điều Khiển Đèn</option>
            <option value="fan">🌀 Quạt Panasonic</option>
            <option value="daikin">❄️ Điều Hòa Daikin</option>
        </select>

        <div class="grid" id="btns"></div>
        <pre id="log">Đang chờ lệnh...</pre>
    </div>

    <script>
        // Cấu hình linh hoạt cho 3 thiết bị
        const configs = {
            light: [
                { id: "toggle", label: "Bật / Tắt Đèn", isToggle: true },
                { id: "temp_down", label: "Giảm Màu" },
                { id: "temp_up", label: "Tăng Màu" },
                { id: "mode_3", label: "Chế độ 3" },
                { id: "brightness_down", label: "Giảm Sáng" }
            ],
            fan: [
                { id: "toggle", label: "Bật / Tắt Quạt", isToggle: true },
                { id: "mode_1", label: "Số 1" },
                { id: "mode_2", label: "Số 2" },
                { id: "mode_3", label: "Số 3" },
                { id: "cancel_timer", label: "Hủy Hẹn Giờ" }
            ],
            daikin: [
                { id: "power_off", label: "Tắt Điều Hòa", isToggle: true },
                { id: "cool_26", label: "Bật Cool 26°C" },
                { id: "cool_27", label: "Bật Cool 27°C" }
            ]
        };
        
        const container = document.getElementById('btns');
        const selector = document.getElementById('deviceSelector');
        const log = document.getElementById('log');

        function renderButtons(category) {
            container.innerHTML = ''; // Xóa nút cũ
            configs[category].forEach(cmd => {
                const btn = document.createElement('button');
                btn.textContent = cmd.label;
                if(cmd.isToggle) btn.className = "btn-toggle";
                
                btn.onclick = async () => {
                    log.style.color = "#a5b4fc";
                    log.textContent = `Đang gửi lệnh [${category}]: ${cmd.label}...`;
                    try {
                        const res = await fetch(`/api/ir/send?category=${category}&command=${cmd.id}`);
                        const data = await res.json();
                        log.style.color = res.ok ? "#86efac" : "#fca5a5";
                        log.textContent = JSON.stringify(data, null, 2);
                    } catch(e) { 
                        log.style.color = "#fca5a5";
                        log.textContent = "Lỗi kết nối: " + e.message; 
                    }
                };
                container.appendChild(btn);
            });
        }

        // Lắng nghe sự kiện đổi thiết bị
        selector.addEventListener('change', (e) => renderButtons(e.target.value));
        
        // Khởi tạo ban đầu
        renderButtons('light');
    </script>
</body>
</html>
)rawliteral";

static void handleRoot(WebServer& server) {
  server.send(200, "text/html", INDEX_HTML);
}

static void sendJson(WebServer& server, int code, const String& body) {
  server.sendHeader("Access-Control-Allow-Origin", "*"); 
  server.send(code, "application/json", body);
}

static void handleStatus(WebServer& server) {
  String body = "{\"ok\":true,\"ssid\":\"";
  body += WiFi.isConnected() ? WiFi.SSID() : String(kApSsid);
  body += "\",\"sta_ip\":\"";
  body += WiFi.isConnected() ? WiFi.localIP().toString() : String("0.0.0.0");
  body += "\",\"ap_ip\":\"";
  body += WiFi.softAPIP().toString();
  body += "\",\"categories\":[\"light\", \"fan\", \"daikin\"]}";
  sendJson(server, 200, body);
}

static void handleSend(WebServer& server, IRsend& irsend) {
  String category = server.arg("category");
  String name = server.arg("command");

  // --- NHÁNH 1: XỬ LÝ ĐIỀU HÒA DAIKIN ---
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
      
      // Đọc Nhiệt độ & Quạt & Cánh vẫy & Powerful
      if (server.hasArg("temp")) daikin.setTemp(server.arg("temp").toInt());
      
      String fan = server.arg("fan");
      if (fan == "auto") daikin.setFan(kDaikinFanAuto);
      else if (fan == "quiet") daikin.setFan(kDaikinFanQuiet);
      else daikin.setFan(fan.toInt()); // Mức 1, 2, 3, 4, 5

      daikin.setSwingVertical(server.arg("swing") == "true");
      daikin.setPowerful(server.arg("powerful") == "true");

      daikin.send(); // Phát sóng hồng ngoại
      sendJson(server, 200, "{\"ok\":true,\"msg\":\"Đã gửi trạng thái Daikin\"}");
    }
    return;
  }

  // --- NHÁNH 2: XỬ LÝ ĐÈN & QUẠT ---
  const IrCommand* cmd = findCommand(category, name);
  if (cmd) {
    sendCommand(irsend, category, name);
    sendJson(server, 200, "{\"ok\":true,\"command\":\"" + name + "\"}");
  } else {
    sendJson(server, 404, "{\"ok\":false,\"error\":\"Not found\"}");
  }
}

void registerApiRoutes(WebServer& server, IRsend& irsend) {
  server.on("/", HTTP_GET, [&](){ handleRoot(server); });
  server.on("/api/status", HTTP_GET, [&](){ handleStatus(server); });
  server.on("/api/ir/send", HTTP_GET, [&](){ handleSend(server, irsend); });
  server.onNotFound([&](){ sendJson(server, 404, "{\"ok\":false,\"error\":\"not_found\"}"); });
}