# Hệ Thống Đỗ Xe Thông Minh (Smart and Safe Parking System)

## 1. Mô tả dự án

Hệ thống đỗ xe thông minh sử dụng ESP32, cảm biến và IoT để tự động hóa quản lý bãi đỗ xe, giám sát an toàn, cảnh báo sự cố và hỗ trợ cập nhật firmware OTA (Over-the-Air) từ xa.

## 2. Chức năng chính

- Quản lý số lượng xe, vị trí từng chỗ đỗ (4 vị trí)
- Tự động đóng/mở cổng bằng cảm biến và servo
- Giám sát nhiệt độ, độ ẩm, khí gas, phát hiện cháy/nổ
- Điều khiển đèn chiếu sáng tự động theo LDR hoặc thủ công
- Cảnh báo nguy hiểm qua còi buzzer
- Giao diện Dashboard Node-RED: hiển thị trạng thái, điều khiển, thống kê
- Cập nhật firmware OTA qua WiFi hoặc BLE (an toàn, không cần tháo thiết bị)

## 3. Phần cứng sử dụng

- ESP32 MH-ET LIVE MiniKit
- 2 x Servo SG90 (đóng/mở cổng)
- 2 x Cảm biến vật cản (cổng vào/ra)
- 4 x HC-SR04 (cảm biến siêu âm cho 4 vị trí đỗ)
- 2 x LDR (cảm biến ánh sáng)
- 1 x DHT11 (nhiệt độ, độ ẩm)
- 1 x MQ-2 (khí gas, khói)
- 1 x Buzzer (cảnh báo)
- Đèn LED

## 4. Sơ đồ kết nối (Pinout)

| Thiết bị      | ESP32 Pin        |
| ------------- | ---------------- |
| Servo vào     | 4                |
| Servo ra      | 2                |
| HC-SR04 Entry | trig 17, echo 16 |
| HC-SR04 Exit  | trig 33, echo 14 |
| Vị trí 1      | trig 26, echo 13 |
| Vị trí 2      | trig 19, echo 18 |
| Vị trí 3      | trig 23, echo 5  |
| Vị trí 4      | trig 32, echo 12 |
| DHT11         | 10               |
| MQ-2          | 35               |
| LDR           | 39               |
| LED1          | 25               |
| LED2          | 15               |
| Buzzer        | 9                |

## 5. Phần mềm

- Lập trình ESP32 với PlatformIO (C++)
- Node-RED Dashboard: hiển thị, điều khiển, thống kê
- Giao tiếp MQTT (EMQX broker)
- Hỗ trợ cập nhật firmware OTA:
  - Qua WiFi (ArduinoOTA)
  - Qua BLE (BLE OTA)

## 6. Luồng hoạt động

- Xe vào/ra: cảm biến phát hiện → servo mở cổng → cập nhật trạng thái
- Quản lý chỗ đỗ: cảm biến siêu âm xác định vị trí trống/đã đỗ
- Đèn: tự động bật/tắt theo LDR hoặc điều khiển từ dashboard
- Cảnh báo: khi phát hiện khí gas, nhiệt độ cao, hoặc sự cố → buzzer kêu, dashboard cảnh báo
- OTA: gửi lệnh từ dashboard hoặc app BLE để cập nhật firmware mới

## 7. Kết quả

- Mô hình hoạt động ổn định, cảnh báo an toàn, quản lý chỗ đỗ hiệu quả
- Giao diện trực quan, dễ sử dụng
- Hỗ trợ cập nhật phần mềm từ xa, an toàn

## 8. Hướng dẫn OTA

- **WiFi OTA**: Dùng PlatformIO hoặc Arduino IDE, chọn IP thiết bị, upload firmware mới
- **BLE OTA**: Dùng app Python (app_ble_v1.py) hoặc script send_firmware.py để gửi file .bin qua Bluetooth
- Có thể chuyển về chế độ OTA bằng lệnh MQTT hoặc nhấn nút BOOT >6s

## 9. Tác giả

- Duy (2025)
