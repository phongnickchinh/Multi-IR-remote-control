#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <Arduino.h>

const char WEB_UI[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Smart Home Dashboard</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f1f5f9; display: flex; flex-direction: column; align-items: center; padding: 15px; margin: 0; padding-bottom: 80px; }
        .page-title { color: #1e293b; margin: 10px 0 25px 0; font-size: 24px; font-weight: 800; text-transform: uppercase; letter-spacing: 1px; }
        .card { background: white; width: 100%; max-width: 380px; padding: 22px; border-radius: 20px; box-shadow: 0 4px 15px rgba(0,0,0,0.05); margin-bottom: 25px; box-sizing: border-box; border-top: 4px solid #cbd5e1; }
        .card-light { border-top-color: #fbbf24; }
        .card-fan { border-top-color: #38bdf8; }
        .card-daikin { border-top-color: #818cf8; }
        .card-header { font-size: 18px; font-weight: bold; color: #334155; margin-bottom: 15px; }
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
        button { padding: 14px 8px; border-radius: 12px; border: 1px solid #e2e8f0; font-weight: bold; cursor: pointer; background: #f8fafc; color: #475569; font-size: 13px; transition: 0.1s; display: flex; align-items: center; justify-content: center; }
        button:active { transform: translateY(2px); background: #e2e8f0; }
        button.btn-toggle { grid-column: span 2; background: #ef4444; color: white; border: none; font-size: 16px; padding: 16px; }
        .screen { background: #94a3b8; height: 110px; border-radius: 12px; margin-bottom: 15px; padding: 12px 15px; box-sizing: border-box; display: flex; flex-direction: column; justify-content: space-between; color: #0f172a; font-weight: bold; border: 3px solid #64748b; box-shadow: inset 0 3px 6px rgba(0,0,0,0.25); }
        .screen-top { display: flex; justify-content: space-between; font-size: 14px; }
        .screen-center { display: flex; justify-content: center; align-items: center; font-size: 46px; font-family: 'Courier New', monospace; letter-spacing: -2px; }
        .screen-bottom { display: flex; justify-content: space-between; font-size: 12px; }
        .row { display: flex; justify-content: space-between; margin-bottom: 10px; gap: 8px; }
        .col { display: flex; flex-direction: column; gap: 10px; flex: 1; }
        .btn-cool { background: #a7f3d0; border-color: #10b981; color: #065f46; }
        .btn-off { background: #fef08a; border-color: #eab308; color: #854d0e; }
        .btn-temp { height: 50px; font-size: 16px; background: white; }
        .log-bar { position: fixed; bottom: 0; left: 0; width: 100%; background: #1e293b; color: #a5b4fc; padding: 15px; box-sizing: border-box; font-size: 13px; font-family: monospace; text-align: center; border-top: 2px solid #0f172a; z-index: 1000; }
    </style>
</head>
<body>
    <div class="page-title">My Smart Home</div>

    <div class="card card-light">
        <div class="card-header">💡 Đèn Phòng</div>
        <div class="grid">
            <button class="btn-toggle" onclick="sendSimpleAPI('light', 'toggle')">Bật / Tắt Đèn</button>
            <button onclick="sendSimpleAPI('light', 'temp_up')">Trắng Hơn ☀️</button>
            <button onclick="sendSimpleAPI('light', 'temp_down')">Vàng Hơn 🌙</button>
            <button onclick="sendSimpleAPI('light', 'mode_3')">Đổi Chế Độ 🛠️</button>
            <button onclick="sendSimpleAPI('light', 'brightness_down')">Giảm Sáng 📉</button>
        </div>
    </div>

    <div class="card card-daikin">
        <div class="card-header">❄️ Điều Hòa Daikin</div>
        <div class="screen" id="lcd">
            <div class="screen-top">
                <span id="lcd-mode">COOL</span>
                <span id="lcd-swing"></span>
            </div>
            <div class="screen-center">
                <span id="lcd-temp">28</span><span id="lcd-unit" style="font-size: 20px; margin-top: -15px;">°C</span>
            </div>
            <div class="screen-bottom">
                <span id="lcd-fan">FAN: AUTO</span>
                <span id="lcd-timer"></span>
                <span id="lcd-powerful"></span>
            </div>
        </div>
        
        <div class="row">
            <button class="btn-cool" onclick="setDaikinMode('cool')">COOL</button>
            <button onclick="setDaikinMode('dry')">DRY</button>
            <button onclick="setDaikinMode('fan')">FAN ONLY</button>
            <button class="btn-off" onclick="turnOffDaikin()">OFF</button>
        </div>
        
        <div class="row">
            <div class="col">
                <button class="btn-temp" onclick="changeDaikinTemp(1)">▲ TEMP</button>
                <button class="btn-temp" onclick="changeDaikinTemp(-1)">▼ TEMP</button>
            </div>
            <div class="col">
                <button class="btn-temp" onclick="changeDaikinFan()">FAN</button>
                <button class="btn-temp" onclick="toggleDaikinPowerful()">POWERFUL</button>
            </div>
        </div>
        <button style="width: 100%; margin-top: 5px; margin-bottom: 15px;" onclick="toggleDaikinSwing()">SWING ↕ (Đảo Gió)</button>
        
        <div class="row" style="border-top: 1px dashed #cbd5e1; padding-top: 15px;">
            <button onclick="addTimer('on')" style="flex: 1; background: #e0f2fe;">ON TIMER</button>
            <button onclick="addTimer('off')" style="flex: 1; background: #fee2e2;">OFF TIMER</button>
            <button onclick="cancelTimer()" style="flex: 1; background: #f1f5f9; color: #ef4444;">CANCEL</button>
        </div>
    </div>

    <div class="card card-fan">
        <div class="card-header">🌀 Quạt Panasonic</div>
        <div class="grid">
            <button class="btn-toggle" onclick="sendSimpleAPI('fan', 'toggle')">Bật / Tắt Quạt</button>
            <button onclick="sendSimpleAPI('fan', 'mode_1')">Số 1</button>
            <button onclick="sendSimpleAPI('fan', 'mode_2')">Số 2</button>
            <button onclick="sendSimpleAPI('fan', 'mode_3')">Số 3</button>
            <button onclick="sendSimpleAPI('fan', 'sleep_mode')">Ngủ 💤</button>
            <button onclick="sendSimpleAPI('fan', 'timer_1h')">Hẹn 1H ⏰</button>
            <button onclick="sendSimpleAPI('fan', 'timer_3h')">Hẹn 3H ⏰</button>
            <button style="grid-column: span 2;" onclick="sendSimpleAPI('fan', 'cancel_timer')">Hủy Hẹn Giờ ✖</button>
        </div>
    </div>

    <div class="log-bar" id="log">Hệ thống sẵn sàng...</div>

<script>
    const log = document.getElementById('log');
    let isRequesting = false;

    function showLog(msg, isError = false) {
        log.style.color = isError ? '#fca5a5' : '#86efac';
        log.innerText = msg;
    }

    async function sendSimpleAPI(category, command) {
        if(isRequesting) return; isRequesting = true;
        showLog(`Đang gửi lệnh [${command}]...`);
        try {
            const res = await fetch(`/api/ir/send?category=${category}&command=${command}`);
            showLog(res.ok ? `✅ Thành công: ${command}` : `❌ Lỗi Server ESP32`, !res.ok);
        } catch (e) { showLog(`❌ Lỗi mạng: ${e.message}`, true); }
        isRequesting = false;
    }

    // Biến lưu trữ thêm onTimer và offTimer (đơn vị: Giờ)
    let dkState = { power: true, mode: 'cool', temp: 28, fan: 'auto', swing: false, powerful: false, onTimer: 0, offTimer: 0 };
    const fanLevels = ['auto', 'quiet', '1', '2', '3', '4', '5'];
    
    function updateDaikinLCD() {
        const lcd = document.getElementById('lcd');
        lcd.style.opacity = dkState.power ? '1' : '0.3';
        document.getElementById('lcd-mode').innerText = dkState.mode.toUpperCase();
        
        // CẬP NHẬT: Ẩn hoàn toàn chữ số và ký hiệu °C khi ở chế độ Dry và Fan
        const hideTemp = (dkState.mode === 'fan' || dkState.mode === 'dry');
        document.getElementById('lcd-temp').innerText = hideTemp ? '--' : dkState.temp;
        document.getElementById('lcd-unit').style.display = hideTemp ? 'none' : 'inline';
        
        document.getElementById('lcd-fan').innerText = `FAN: ${dkState.fan.toUpperCase()}`;
        document.getElementById('lcd-swing').innerText = dkState.swing ? '↕ SWING' : '';
        document.getElementById('lcd-powerful').innerText = dkState.powerful ? 'POWERFUL' : '';
        
        // Hiển thị trạng thái Hẹn giờ
        let timerText = "";
        if (dkState.onTimer > 0) timerText += `ON: ${dkState.onTimer}H `;
        if (dkState.offTimer > 0) timerText += `OFF: ${dkState.offTimer}H`;
        document.getElementById('lcd-timer').innerText = timerText.trim();
    }

    function setDaikinMode(mode) { dkState.power = true; dkState.mode = mode; dkState.powerful = false; updateDaikinLCD(); sendDaikinAPI(); }
    function turnOffDaikin() { dkState.power = false; updateDaikinLCD(); sendDaikinOffAPI(); }
    function changeDaikinTemp(delta) { if (!dkState.power || dkState.mode === 'fan' || dkState.mode === 'dry') return; dkState.temp = Math.min(32, Math.max(18, dkState.temp + delta)); updateDaikinLCD(); sendDaikinAPI(); }
    function changeDaikinFan() { if (!dkState.power) return; dkState.fan = fanLevels[(fanLevels.indexOf(dkState.fan) + 1) % fanLevels.length]; updateDaikinLCD(); sendDaikinAPI(); }
    function toggleDaikinSwing() { if (!dkState.power) return; dkState.swing = !dkState.swing; updateDaikinLCD(); sendDaikinAPI(); }
    function toggleDaikinPowerful() { if (!dkState.power || dkState.mode === 'fan') return; dkState.powerful = !dkState.powerful; updateDaikinLCD(); sendDaikinAPI(); }

    // Logic Tăng Hẹn Giờ
    function addTimer(type) {
        if (type === 'on') {
            dkState.onTimer = dkState.onTimer >= 12 ? 1 : dkState.onTimer + 1; // Tối đa 12 tiếng rồi quay lại 1
        } else {
            dkState.offTimer = dkState.offTimer >= 12 ? 1 : dkState.offTimer + 1;
        }
        updateDaikinLCD();
        sendDaikinAPI();
    }

    // Logic Hủy Hẹn Giờ
    function cancelTimer() {
        dkState.onTimer = 0;
        dkState.offTimer = 0;
        updateDaikinLCD();
        sendDaikinAPI();
    }

    async function sendDaikinAPI() {
        if(isRequesting) return; isRequesting = true;
        // Chuyển đổi Giờ thành Phút (x60) để gửi xuống cho backend C++ xử lý
        const params = new URLSearchParams({ 
            category: 'daikin', 
            command: 'set_state', 
            mode: dkState.mode, 
            temp: dkState.temp, 
            fan: dkState.fan, 
            swing: dkState.swing, 
            powerful: dkState.powerful,
            on_timer: dkState.onTimer * 60,
            off_timer: dkState.offTimer * 60
        });
        showLog(`[DAIKIN] Đang nạp trạng thái...`);
        try {
            const res = await fetch(`/api/ir/send?${params.toString()}`);
            showLog(res.ok ? `✅ DAIKIN: Đã cập nhật!` : `❌ Lỗi Daikin`, !res.ok);
        } catch (e) { showLog(`❌ Lỗi mạng`, true); }
        isRequesting = false;
    }

    async function sendDaikinOffAPI() {
        if(isRequesting) return; isRequesting = true;
        showLog("[DAIKIN] Đang Tắt...");
        try {
            const res = await fetch(`/api/ir/send?category=daikin&command=power_off`);
            showLog(res.ok ? "✅ DAIKIN: Đã TẮT máy." : "❌ Lỗi tắt máy", !res.ok);
        } catch (e) { showLog(`❌ Lỗi mạng`, true); }
        isRequesting = false;
    }

    updateDaikinLCD();
</script>
</body>
</html>
)rawliteral";

#endif