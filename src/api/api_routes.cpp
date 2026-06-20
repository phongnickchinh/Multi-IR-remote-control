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
        .card { background: white; padding: 24px; border-radius: 12px; width: 100%; max-width: 500px; }
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 20px; }
        button { padding: 15px; border: none; border-radius: 8px; background: #3b82f6; color: white; font-size: 16px; cursor: pointer; }
        button:active { transform: scale(0.95); }
        button.btn-toggle { grid-column: span 2; background: #ef4444; }
        pre { background: #1e293b; color: #a5b4fc; padding: 16px; border-radius: 8px; font-size: 12px; overflow-x: auto; }
    </style>
</head>
<body>
    <div class="card">
        <h2 style="text-align: center;">Điều Khiển Đèn</h2>
        <div class="grid" id="btns"></div>
        <pre id="log">Đang chờ lệnh...</pre>
    </div>
    <script>
        const cmds = [
            { id: "toggle", label: "Bật / Tắt Đèn" },
            { id: "temp_down", label: "Giảm Màu" },
            { id: "temp_up", label: "Tăng Màu" },
            { id: "mode_3", label: "Chế độ 3" },
            { id: "brightness_down", label: "Giảm Sáng" }
        ];
        
        const container = document.getElementById('btns');
        cmds.forEach(cmd => {
            const btn = document.createElement('button');
            btn.textContent = cmd.label;
            if(cmd.id === "toggle") btn.className = "btn-toggle";
            
            btn.onclick = async () => {
                const log = document.getElementById('log');
                log.textContent = `Đang gửi: ${cmd.label}...`;
                try {
                    // Gọi API nội bộ (cùng host) nên không cần ghi rõ IP
                    const res = await fetch(`/api/ir/send?category=light&command=${cmd.id}`);
                    log.textContent = await res.text();
                } catch(e) { log.textContent = e; }
            };
            container.appendChild(btn);
        });
    </script>
</body>
</html>
)rawliteral";

static void handleRoot(WebServer& server) {
  server.send(200, "text/html", INDEX_HTML);
}

static void sendJson(WebServer& server, int code, const String& body) {
  // Thêm dòng giấy thông hành này
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
  body += "\",\"categories\":[\"light\"]}";
  sendJson(server, 200, body);
}

static void handleSend(WebServer& server, IRsend& irsend) {
  if (!server.hasArg("category") || !server.hasArg("command")) {
    sendJson(server, 400, "{\"ok\":false,\"error\":\"missing_args\"}"); return;
  }
  String category = server.arg("category");
  String name = server.arg("command");
  const IrCommand* cmd = findCommand(category, name);
  if (!cmd) { sendJson(server, 404, "{\"ok\":false,\"error\":\"unknown_command\"}"); return; }
  if (!sendCommand(irsend, category, name)) { sendJson(server, 501, "{\"ok\":false,\"error\":\"unsupported_protocol\"}"); return; }
  
  String body = "{\"ok\":true,\"command\":\"" + String(cmd->name) + "\",\"hex\":\"" + hexString(cmd->code) + "\"}";
  sendJson(server, 200, body);
}

void registerApiRoutes(WebServer& server, IRsend& irsend) {
  server.on("/", HTTP_GET, [&](){ handleRoot(server); });
  server.on("/api/status", HTTP_GET, [&](){ handleStatus(server); });
  server.on("/api/ir/send", HTTP_GET, [&](){ handleSend(server, irsend); });
  server.onNotFound([&](){ sendJson(server, 404, "{\"ok\":false,\"error\":\"not_found\"}"); });
}
