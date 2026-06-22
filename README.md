# ESP32 Multi-IR Smart Home Remote

Hệ thống trung tâm điều khiển (Smart Home Hub) phát triển trên nền tảng **ESP32**. Dự án ứng dụng ESP32 làm WebServer và bộ phát tín hiệu hồng ngoại (IR Transmitter), cho phép quản lý tập trung đa thiết bị (Điều hòa Daikin, Quạt Panasonic, hệ thống chiếu sáng) thông qua giao diện Web đồng bộ trạng thái theo thời gian thực.

---

## Tính Năng Cốt Lõi

* **Giao diện Web SPA (Single Page Application):** Cấu trúc trực quan, tối ưu cho thiết bị di động, phân tách rõ ràng theo phân hệ thiết bị.
* **Điều khiển Điều hòa Daikin chuyên sâu:**
    * Mô phỏng màn hình LCD hiển thị các thông số kỹ thuật (Nhiệt độ, Chế độ, Quạt, Swing, Hẹn giờ).
    * Xử lý logic giao diện: Tự động ẩn thông số nhiệt độ khi thiết bị chuyển sang chế độ `DRY` hoặc `FAN ONLY`.
    * Quản lý State trực tiếp trên RAM ESP32, duy trì tính nhất quán của dữ liệu điều khiển.
* **Điều khiển Quạt Panasonic & Đèn:** Triển khai phương thức gửi mảng mã tín hiệu thô (`sendRaw`) nhằm đảm bảo độ tin cậy và chính xác đối với các thiết bị không truyền toàn bộ State trong một chu kỳ lệnh.
* **Đồng bộ trạng thái (Real-time Sync):** Cơ chế tự động polling API để đồng bộ hóa trạng thái thiết bị giữa nhiều phiên truy cập (client) độc lập trong cùng một thời điểm.
* **Tự động hóa & Tối ưu năng lượng (Phase 2):** Tích hợp cảm biến hiện diện Radar mmWave nhằm thiết lập kịch bản tự động ngắt hệ thống khi không phát hiện người trong không gian.

---

## Yêu Cầu Kỹ Thuật

### Phần cứng
* Board phát triển: **ESP32** (NodeMCU, ESP32-WROOM, v.v.).
* Module phát hồng ngoại (IR Transmitter) 38kHz.
* Module thu hồng ngoại (IR Receiver) phục vụ việc trích xuất mã lệnh từ thiết bị gốc.
* *Phụ trợ:* Cảm biến hiện diện Radar mmWave (HLK-LD2410 hoặc tương đương) cho module Auto-Off.

### Phần mềm & Thư viện
* Môi trường lập trình: **VS Code + PlatformIO**.
* Framework: `Arduino`.
* Thư viện lõi: [`IRremoteESP8266`](https://github.com/crankyoldgit/IRremoteESP8266) (Ứng dụng class `IRDaikinESP` cho Điều hòa và hàm `sendRaw()` cho thiết bị cơ bản).

---

## Kiến Trúc Mã Nguồn

Kiến trúc phần mềm được thiết kế phân tách rõ ràng giữa Frontend (UI), Backend (Routing) và Logic điều khiển (Command Registry):

```text
├── include/
│   ├── index_html.h          # Chứa giao diện Web (HTML/CSS/JS) định dạng SPA.
│   └── ...                   # Các file header cấu hình hệ thống.
├── src/
│   ├── api/
│   │   └── api_routes.cpp    # Routing xử lý HTTP GET request từ Frontend.
│   ├── commands/
│   │   └── command_registry.cpp # Chứa cấu trúc mảng mã lệnh thô (Raw Array) cho Quạt và Đèn.
│   ├── storage/
│   │   └── ...               # Module xử lý lưu trữ bộ nhớ non-volatile (EEPROM/Flash).
│   ├── support/
│   │   └── ir_sniffer.cpp    # Module hỗ trợ phân tích và đọc mã lệnh từ remote phần cứng.
│   └── main.cpp              # Khởi tạo WiFi, WebServer, cấu trúc Object và Logic Main Loop.
├── platformio.ini            # File cấu hình môi trường PlatformIO.
└── README.md                 # Tài liệu mô tả dự án.
```

---

## Tài Liệu API

Hệ thống cung cấp 2 luồng API chính để Frontend giao tiếp với ESP32:

### 1. Luồng truyền lệnh: `/api/ir/send`
* **Chức năng:** Tiếp nhận lệnh từ Web UI và kích hoạt tín hiệu IR.
* **Parameters (Phân hệ Daikin):** `category=daikin`, `command=set_state`, `mode`, `temp`, `fan`, `swing`, `powerful`, `on_timer`, `off_timer`.
* **Parameters (Phân hệ Quạt/Đèn):** `category=[device_category]`, `command=[action_command]`.
* **Xử lý Logic:** Phân rã payload từ URL. Nạp State vào object `IRDaikinESP` đối với Điều hòa; hoặc tra cứu `command_registry` và kích hoạt `sendRaw()` đối với Quạt/Đèn.

### 2. Luồng đồng bộ: `/api/daikin/state`
* **Chức năng:** Trích xuất trạng thái hiện hành (State) của Điều hòa từ RAM.
* **Response Format (JSON):**
    ```json
    {
      "power": true,
      "mode": 3,
      "temp": 26,
      "fan": 2,
      "on_timer": 0,
      "off_timer": 2
    }
    ```

---

## Hướng Dẫn Triển Khai

1.  **Clone repository:**
    ```bash
    git clone [https://github.com/phongnickchinh/multi-ir-remote-control.git](https://github.com/phongnickchinh/multi-ir-remote-control.git)
    ```
2.  **Thiết lập môi trường:** Mở thư mục dự án bằng VS Code có tích hợp extension PlatformIO.
3.  **Cấu hình mạng:** Cập nhật thông số `SSID` và `PASSWORD` cho module WiFi trong tệp `main.cpp` hoặc `config.h`.
4.  **Thu thập mã lệnh (Optional):** Chạy module `ir_sniffer.cpp` để xuất mảng `uint16_t rawData` của thiết bị (Quạt/Đèn), sau đó khai báo vào `command_registry.cpp`.
5.  **Biên dịch & Nạp firmware:** Kết nối ESP32 và thực thi lệnh `Upload` trên PlatformIO.
6.  **Vận hành:** Mở Serial Monitor để xác định địa chỉ IP cấp phát cho ESP32. Sử dụng trình duyệt truy cập vào IP (Ví dụ: `http://192.168.1.xxx`) để kiểm tra hệ thống.

---

## Lộ Trình Phát Triển (Roadmap)

- [x] Thiết kế Web UI định dạng SPA với màn hình trạng thái giả lập.
- [x] Tích hợp class chuyên sâu cho Điều hòa Daikin (`IRDaikinESP`).
- [x] Xây dựng module phát lệnh định dạng thô (Raw Data) cho Quạt và Đèn.
- [x] Phát triển hệ thống API và cơ chế Polling đồng bộ thiết bị.
- [ ] **Phase 2 (Current Focus):** Tích hợp cảm biến Radar mmWave (LD2410) phục vụ việc giám sát hiện diện qua nhịp thở.
- [ ] Triển khai kịch bản Auto-Off: Đếm ngược 15 phút từ thời điểm không phát hiện sự hiện diện để tự động ngắt toàn bộ thiết bị.
- [ ] **Phase 3:** Xây dựng luồng gửi cảnh báo an ninh và báo cáo trạng thái phòng về thiết bị di động (Tích hợp Telegram Bot hoặc giao thức MQTT).