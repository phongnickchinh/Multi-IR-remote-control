# KẾ HOẠCH DỰ ÁN: ESP32 CLOCK RETROFIT MODULE
**Mô tả:** Chế tạo một module "ký sinh" gắn vào phía sau đồng hồ báo thức cơ học truyền thống. Module này sử dụng ESP32 và động cơ bước để tự động vặn núm điều chỉnh giờ báo thức dựa trên lệnh từ điện thoại (qua Wi-Fi/MQTT), sau đó tự động chìm vào giấc ngủ để tiết kiệm năng lượng.

---

## 1. Danh sách linh kiện (BOM - Bill of Materials)

### Khối Xử lý & Kết nối
* **Vi điều khiển:** ESP32 (Khuyên dùng bản ESP32-C3 hoặc NodeMCU chuẩn). Đóng vai trò nhận lệnh từ Server và điều khiển động cơ.

### Khối Truyền động (Actuator)
* **Động cơ:** Động cơ bước (Stepper Motor) **28BYJ-48**. (Lực xoắn mạnh, quay được 360 độ và tính toán được chính xác góc quay).
* **Mạch Driver:** **ULN2003** (Đi kèm với động cơ để ESP32 có thể điều khiển).
* **Cơ khí:** Ngàm nối in 3D hoặc hệ thống bánh răng để nối trục động cơ vào núm vặn báo thức của đồng hồ.

### Khối Định vị (Homing) - *Bắt buộc*
* **Cảm biến:** Cảm biến từ tính (Hall Effect Sensor) hoặc Cảm biến quang chữ U (Optical Endstop).
* **Vật làm mốc:** Một viên nam châm liti (nếu dùng cảm biến từ) dán lên viền của núm vặn báo thức.

### Khối Nguồn năng lượng
* **Giai đoạn 1 (Cắm điện):** Cáp Micro-USB/Type-C và củ sạc 5V.
* **Giai đoạn 2 (Dùng pin):**
    * Pin Lithium 18650 (1 cell) + Đế pin.
    * Mạch sạc và bảo vệ pin **TP4056**.
    * Mạch tăng áp mini (Boost Converter) từ 3.7V lên 5V (để cấp đủ áp cho động cơ bước 28BYJ-48).

---

## 2. Nguyên lý hoạt động (Logic Flow)

Hệ thống hoạt động theo chu trình "Nhận lệnh -> Tìm Điểm 0 -> Vặn Góc -> Ngủ":

1.  **Chờ lệnh (Sleep):** ESP32 duy trì kết nối Wi-Fi ở chế độ ngủ nông (Modem Sleep) hoặc ngủ sâu hẳn (Deep Sleep). Động cơ được ngắt điện hoàn toàn.
2.  **Nhận lệnh:** Người dùng chọn "6:00 AM" trên App/Web. Lệnh được đẩy về ESP32.
3.  **Khởi động Motor:** ESP32 cấp điện cho mạch ULN2003.
4.  **Dò mốc 0 (Homing):** Động cơ quay núm báo thức từ từ cho đến khi cảm biến Hall nhận diện được viên nam châm. ESP32 ghi nhận đây là mốc `12:00`.
5.  **Tính toán & Vặn góc:** ESP32 tính toán khoảng cách từ `12:00` đến `6:00` (tương đương 180 độ). Động cơ bước thực hiện quay chính xác số bước tương ứng với 180 độ.
6.  **Hoàn thành:** ESP32 ngắt điện động cơ, hạ xung nhịp, tắt Wi-Fi (nếu cần) và quay lại trạng thái Ngủ.

---

## 3. Chiến lược Tối ưu Năng lượng (Power Management)

Thay vì dùng nút bấm vật lý, hệ thống sẽ được cấu hình bằng phần mềm để tự động quản lý năng lượng:

* **Khi cắm nguồn USB:** * ESP32 chạy ở `Active Mode`.
    * Kết nối Wi-Fi 24/7, nhận lệnh và vặn núm ngay lập tức (Độ trễ < 1s).
* **Khi chạy Pin Lithium (Tương lai):** * *Cách 1 (Ngủ nông - Modem Sleep):* Thêm lệnh `esp_wifi_set_ps(WIFI_PS_MIN_MODEM);`. ESP32 vẫn giữ Wi-Fi lơ mơ, nhận lệnh tức thì, tiêu thụ khoảng ~1-2mA. Pin dùng được 1-2 tháng.
    * *Cách 2 (Ngủ chợp mắt - Timer Deep Sleep):* Dùng lệnh `esp_sleep_enable_timer_wakeup()`. ESP32 tắt mọi thứ, tiêu thụ 10µA. Cứ 15 phút tự tỉnh dậy mở Wi-Fi check server 1 lần rồi ngủ tiếp. Pin dùng được > 6 tháng (Đổi lại có độ trễ nhận lệnh tối đa 15 phút).

---

## 4. Các rủi ro cơ khí & Lưu ý kỹ thuật

1.  **Giới hạn chiều quay (Cực kỳ quan trọng):** Các đồng hồ báo thức cơ học truyền thống thường sử dụng cơ cấu bánh cóc (ratchet) ở núm vặn báo thức, **chỉ cho phép vặn theo 1 chiều duy nhất** (thường là ngược chiều kim đồng hồ). Lập trình ESP32 tuyệt đối chỉ cho động cơ quay theo 1 chiều này, nếu quay ngược sẽ làm gãy bánh răng bên trong đồng hồ.
2.  **Sai số vật lý:** Khác với đồng hồ điện tử số, cơ cấu cam báo thức của đồng hồ cơ có dung sai. Đặt 6:00 có thể sẽ reo lúc 5:55 hoặc 6:05. Code ESP32 cần có một biến `calibration_offset` để bạn tinh chỉnh bù trừ góc quay bằng phần mềm.
3.  **Cách ly nguồn motor:** Không cấp nguồn 5V cho động cơ bước thông qua chân `3V3` hoặc `VIN` của ESP32 nếu dây điện quá mỏng. Nên đi dây nguồn 5V riêng từ củ sạc (hoặc mạch Boost) rẽ làm 2 nhánh: 1 nhánh vào ESP32, 1 nhánh vào mạch ULN2003 để tránh sụt áp gây reset chip.