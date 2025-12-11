# 🚗 Smart IoT Blackbox - Hệ Thống Hộp Đen Thông Minh

![PlatformIO](https://img.shields.io/badge/PlatformIO-Core-orange)
![ESP32](https://img.shields.io/badge/Device-ESP32-blue)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)

## 1. Mô tả dự án (Project Description)
**Đặt vấn đề ** 

**Smart IoT Blackbox** Là một chiếc hộp thông minh

Hệ thống đảm bảo toàn vẹn dữ liệu bằng cơ chế "Dual Logging": lưu trữ cục bộ (Offline) trên thẻ nhớ SD và đồng bộ đám mây (Online) qua giao thức MQTT.

## 2. Chức năng chính (Key Features)

- 📍 **
- 💥 **Phát hiện va chạm (Crash Detection):** Sử dụng IMU 6 trục để phát hiện rung lắc mạnh, lật xe hoặc va chạm bất ngờ.
- 🔥 **Giám sát an toàn:** Theo dõi nhiệt độ, độ ẩm và nồng độ khí gas/khói (CO/LPG) để phòng chống cháy nổ.
- 💾 **Lưu trữ cục bộ (Blackbox Mode):** Tự động ghi log dữ liệu vào thẻ nhớ SD (CSV format), đảm bảo không mất dữ liệu khi mất mạng.
- 🚨 **Cảnh báo tức thời:** Kích hoạt còi Buzzer và gửi cảnh báo `EMERGENCY` về Dashboard ngay lập tức khi có sự cố.
- 📊 **Dashboard trực quan:** Giao diện Node-RED hiển thị bản đồ (Map), biểu đồ cảm biến và lịch sử hành trình.
- 📲 **Cập nhật từ xa (OTA):** Hỗ trợ nạp Firmware mới qua WiFi hoặc Bluetooth Low Energy (BLE).

## 3. Phần cứng sử dụng (Hardware)

| Thành phần | Model | Chức năng |
| :--- | :--- | :--- |
| MCU | **ESP32 DevKit V1** | Vi điều khiển trung tâm, WiFi, Bluetooth |
| GPS | **NEO-6M** | Định vị toạ độ, vận tốc |
| IMU | **MPU6050** | Gia tốc kế & Con quay hồi chuyển |
| Storage | **Micro SD Module** | Module đọc thẻ nhớ (giao tiếp SPI) |
| Temp/Hum | **DHT11 / DHT22** | Cảm biến nhiệt độ, độ ẩm |
| Gas | **MQ-2** | Cảm biến khí gas, khói |
| Alarm | **Active Buzzer** | Còi báo động |
| Power | **DC-DC Buck Converter** | Hạ áp 12V/24V xuống 5V (nếu dùng trên xe) |

## 4. Sơ đồ kết nối (Pinout)

*Lưu ý: Sơ đồ dựa trên ESP32 30-pin/38-pin tiêu chuẩn.*

| Thiết bị | Chân thiết bị | ESP32 GPIO | Giao tiếp |
| :--- | :--- | :--- | :--- |
| **GPS NEO-6M** | RX | 17 (TX2) | UART2 |
| | TX | 16 (RX2) | UART2 |
| **MPU6050** | SDA | 21 | I2C |
| | SCL | 22 | I2C |
| **SD Card** | CS | 5 | SPI (VSPI) |
| | MOSI | 23 | SPI (VSPI) |
| | MISO | 19 | SPI (VSPI) |
| | CLK | 18 | SPI (VSPI) |
| **DHT11** | DATA | 4 | Digital |
| **MQ-2** | AO (Analog) | 35 | Analog (ADC1)* |
| **Buzzer** | VCC/SIG | 32 | Digital Out |
| **LED Status** | Anode | 2 | Digital Out (Built-in) |
| **OTA Button** | Pin 1 | 0 (BOOT) | Input Pullup |

*(Note: Sử dụng GPIO 35 cho Analog vì ADC2 không hoạt động khi dùng WiFi)*

## 5. Cấu trúc phần mềm (Software Stack)

- **Firmware:** C++ (PlatformIO / Arduino Framework).
  - Hệ điều hành: FreeRTOS (Đa luồng cho Sensor, SD Write, Network).
- **Backend/Broker:** EMQX hoặc Mosquitto MQTT Broker.
- **Frontend/Dashboard:** Node-RED (Dashboard 2.0).
- **Protocol:** MQTT (TCP/IP), Serial (UART), SPI, I2C.

## 6. Luồng hoạt động (Workflow)

1.  **Init:** Khởi động hệ thống, mount thẻ nhớ SD, kết nối WiFi.
2.  **Sensing:**
    - Đọc dữ liệu GPS (tọa độ).
    - Đọc MPU6050 (tính toán tổng gia tốc Vector $a$).
    - Đọc môi trường (MQ-2, DHT).
3.  **Processing:**
    - Nếu $a > Threshold$ (ngưỡng va chạm) -> **TRIGGER CRASH EVENT**.
    - Nếu Gas > Threshold -> **TRIGGER FIRE ALARM**.
4.  **Logging & Reporting:**
    - Ghi dòng dữ liệu vào file `/log_data.csv` trên thẻ nhớ.
    - Đóng gói JSON payload -> Publish lên topic `blackbox/data`.
    - Nếu có sự cố -> Publish lên topic `blackbox/alert` với QoS 2.

## 7. Hướng dẫn OTA (Firmware Update)

Hệ thống hỗ trợ 2 phương thức cập nhật Firmware:

### Cách 1: WiFi OTA (Mạng nội bộ)
Dùng khi thiết bị đang kết nối WiFi ổn định.
1.  Mở dự án trên **PlatformIO**.
2.  Trong `platformio.ini`, thêm dòng: `upload_protocol = espota` và `upload_port = IP_CUA_ESP32`.
3.  Nhấn nút **Upload**.

### Cách 2: BLE OTA (Khi không có WiFi)
Dùng khi ở hiện trường, thao tác qua điện thoại hoặc Laptop có Bluetooth.
1.  Nhấn giữ nút **BOOT** trên ESP32 > 5 giây. Đèn LED sẽ nháy nhanh (Mode BLE).
2.  Sử dụng script Python `ble_uploader.py` hoặc App điện thoại chuyên dụng.
3.  Chọn file firmware `.bin` và gửi đi. Thiết bị sẽ tự động Flash và Reset.

## 8. Cài đặt và Chạy thử (Installation)

1.  Clone repository này về máy.
2.  Mở bằng VS Code (đã cài extension PlatformIO).
3.  Cấu hình WiFi và MQTT Broker trong file `config.h`.
4.  Build và Upload code xuống ESP32.
5.  Import file `flows.json` vào Node-RED để tạo Dashboard.

## 9. Tác giả (Author)

- **Duy**
- Year: 2025
- Contact: [Email/Link của bạn]

---
*Dự án này được thiết kế cho mục đích học tập và nghiên cứu IoT ứng dụng.*
