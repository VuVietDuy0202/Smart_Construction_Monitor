# 🏗️ Smart Construction Monitor - Hệ Thống Giám Sát An Toàn Công Trình

![PlatformIO](https://img.shields.io/badge/PlatformIO-Core-orange)
![ESP32](https://img.shields.io/badge/Device-ESP32-blue)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)

## 1. Mô tả dự án (Project Description)

**Đặt vấn đề:**
Hiện nay, các công trình xây dựng ngày càng có quy mô lớn và độ cao phức tạp, đi kèm với đó là những rủi ro tiềm ẩn về an toàn lao động và chất lượng kết cấu. Các vấn đề như độ rung lắc bất thường, giàn giáo bị nghiêng, hay việc kiểm soát nhiệt độ/độ ẩm để bảo dưỡng bê tông cần được giám sát chặt chẽ 24/7. Tuy nhiên, việc giám sát thủ công thường tốn kém và không báo cáo kịp thời khi sự cố xảy ra.

**Giải pháp:**
Dự án **Smart Construction Monitor** là một thiết bị IoT nhỏ gọn, thông minh được gắn trực tiếp lên các kết cấu (giàn giáo, cốp pha, cột trụ). Thiết bị có khả năng giám sát liên tục độ nghiêng, độ rung chấn và môi trường, từ đó đưa ra cảnh báo sớm giúp ngăn chặn tai nạn và đảm bảo chất lượng công trình.

## 2. Chức năng chính (Key Features)

- 📐 **Giám sát kết cấu (Structural Health):**
  - Đo **Góc nghiêng (Tilt):** Phát hiện giàn giáo hoặc cốp pha bị nghiêng vượt mức cho phép.
  - Đo **Độ rung (Vibration):** Phát hiện rung chấn bất thường do gió lớn hoặc mất ổn định kết cấu.
- 🌡️ **Giám sát môi trường (Environment):**
  - Theo dõi **Nhiệt độ & Độ ẩm:** Hỗ trợ giám sát quá trình đóng rắn/bảo dưỡng của bê tông hoặc môi trường làm việc của công nhân.
  - Phát hiện **Khí thải/Cháy nổ:** Cảnh báo khi nồng độ khói bụi hoặc khí Gas tăng cao tại công trường.
- 🚨 **Cảnh báo đa tầng:**
  - **Tại chỗ:** Còi Buzzer hú ngay lập tức khi phát hiện nghiêng/rung lắc mạnh.
  - **Từ xa:** Gửi cảnh báo khẩn cấp về trung tâm điều hành (Dashboard) qua Internet.
- 💾 **Lưu trữ an toàn:** Ghi dữ liệu vào thẻ nhớ SD (Log file) để phân tích nguyên nhân nếu có sự cố xảy ra (hoạt động như hộp đen công trình).
- 📲 **Cập nhật không dây (OTA):** Bảo trì và nâng cấp phần mềm từ xa mà không cần tháo gỡ thiết bị khỏi vị trí lắp đặt trên cao.

## 3. Phần cứng sử dụng (Hardware)

| Thành phần | Chức năng | Ghi chú |
| :--- | :--- | :--- |
| **ESP32 DevKit V1** | Vi điều khiển trung tâm, WiFi, BLE | Xử lý dữ liệu & Gửi tin |
| **MPU6050** | Cảm biến Gia tốc & Con quay hồi chuyển | Đo góc nghiêng X/Y, đo độ rung |
| **DHT11 / DHT22** | Cảm biến Nhiệt độ, Độ ẩm | Giám sát môi trường/bê tông |
| **MQ-2** | Cảm biến Khí gas/Khói | Phát hiện nguy cơ cháy nổ |
| **Micro SD Module** | Module thẻ nhớ SPI | Lưu trữ dữ liệu Offline |
| **Active Buzzer** | Còi báo động | Cảnh báo âm thanh tại chỗ |
| **Nguồn** | Pin dự phòng hoặc Adapter 5V | Cấp nguồn ổn định |

## 4. Sơ đồ kết nối (Pinout)

| Thiết bị | Chân thiết bị | ESP32 GPIO | Giao tiếp |
| :--- | :--- | :--- | :--- |
| **MPU6050** | SDA | 21 | I2C |
| | SCL | 22 | I2C |
| **SD Card** | CS | 5 | SPI (VSPI) |
| | MOSI | 23 | SPI (VSPI) |
| | MISO | 19 | SPI (VSPI) |
| | CLK | 18 | SPI (VSPI) |
| **DHT11** | DATA | 4 | Digital |
| **MQ-2** | AO (Analog) | 35 | Analog (ADC1) |
| **Buzzer** | VCC/SIG | 32 | Digital Out |
| **LED Status** | Anode | 2 | Digital Out |
| **OTA Button** | Pin 1 | 0 (BOOT) | Input Pullup |

## 5. Phần mềm & Công nghệ

- **Firmware:** C++ trên nền tảng PlatformIO.
  - Sử dụng **Kalman Filter** hoặc **Complementary Filter** để lọc nhiễu dữ liệu từ MPU6050, giúp đo góc nghiêng chính xác.
- **Giao thức:** MQTT (gửi dữ liệu thời gian thực).
- **Dashboard:** Node-RED (Hiển thị biểu đồ rung, đồng hồ đo góc nghiêng, trạng thái an toàn).
- **OTA:** ArduinoOTA (WiFi) và Custom BLE OTA (Bluetooth).

## 6. Luồng hoạt động (Workflow)

1.  **Thu thập:** ESP32 đọc dữ liệu từ MPU6050 (Gia tốc, Góc quay) và DHT11/MQ-2 cứ 100ms/lần.
2.  **Tính toán:**
    - Tính toán góc nghiêng hiện tại so với trục thẳng đứng.
    - Tính toán biên độ rung động.
3.  **So sánh ngưỡng an toàn (Safety Logic):**
    - Nếu `Góc nghiêng > 15 độ` HOẶC `Độ rung > Ngưỡng A` $\rightarrow$ **NGUY HIỂM**.
    - Kích hoạt Buzzer.
    - Gửi gói tin `ALARM` ngay lập tức lên Server.
4.  **Lưu trữ & Báo cáo:**
    - Ghi thông số vào thẻ nhớ SD (chu kỳ 1s).
    - Gửi dữ liệu thông thường lên Dashboard (chu kỳ 5s) để vẽ biểu đồ giám sát.

## 7. Kết quả mong đợi

- Phát hiện sớm hiện tượng lún, nghiêng giàn giáo trước khi sập đổ.
- Giám sát được điều kiện nhiệt độ giúp đảm bảo chất lượng bê tông.
- Hệ thống hoạt động bền bỉ trong môi trường công trường (bụi, rung).
- Cung cấp dữ liệu minh bạch cho chủ đầu tư và giám sát an toàn.

## 8. Hướng dẫn OTA

Do thiết bị thường lắp ở vị trí cao, khó tiếp cận, tính năng OTA rất quan trọng:
- **WiFi OTA:** Khi công trường có phủ sóng WiFi, nạp code trực tiếp qua IP thiết bị.
- **BLE OTA:** Khi mất mạng, kỹ sư đứng dưới đất dùng điện thoại kết nối Bluetooth để nạp firmware mới hoặc thay đổi ngưỡng cảnh báo an toàn.

## 9. Tác giả

- **Duy**
- Năm thực hiện: 2025
