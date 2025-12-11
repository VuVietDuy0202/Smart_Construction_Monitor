#include "sensor_module.h"
#include <Wire.h>

// ============================================
// BIẾN TOÀN CỤC
// ============================================
DHT dhtSensor(PIN_DHT, DHT11);
bool sensorInitialized = false;

// MPU6050 I2C
#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_CONFIG 0x1A

// IMU Variables
float RatePitch, RateRoll, RateYaw;
float RateCalibrationPitch = 0, RateCalibrationRoll = 0, RateCalibrationYaw = 0;
float AccXCalibration = 0, AccYCalibration = 0, AccZCalibration = 0; // THÊM CALIBRATION CHO ACCEL
float AccX, AccY, AccZ;
float AngleRoll, AnglePitch;
float CompAngleRoll = 0, CompAnglePitch = 0;

const float LOOP_TIME = 0.004f; // 4ms = 250Hz
const float COMP_ALPHA = 0.96f; // bạn muốn 0.991

uint32_t MPULoopTimer = 0;

// ============================================
// HÀM GHI THANH GHI I2C (giữ nguyên)
// ============================================
void writeMPU(uint8_t reg, uint8_t data) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}


static bool readMPU_raw_block() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    uint8_t got = Wire.requestFrom((uint16_t)MPU6050_ADDR, (uint8_t)14, (uint8_t)true);
    if (got < 14) return false;

    int16_t AccXLSB = (Wire.read() << 8) | Wire.read();
    int16_t AccYLSB = (Wire.read() << 8) | Wire.read();
    int16_t AccZLSB = (Wire.read() << 8) | Wire.read();
    Wire.read(); Wire.read(); // temp discard
    int16_t GyroX = (Wire.read() << 8) | Wire.read();
    int16_t GyroY = (Wire.read() << 8) | Wire.read();
    int16_t GyroZ = (Wire.read() << 8) | Wire.read();
    AccX = (float)AccXLSB / 4096.0f;
    AccY = (float)AccYLSB / 4096.0f;
    AccZ = (float)AccZLSB / 4096.0f;

    RateRoll  = (float)GyroX / 65.5f;
    RatePitch = (float)GyroY / 65.5f;
    RateYaw   = (float)GyroZ / 65.5f;

    return true;
}

// chỉnh lại thế offset gyro và accel
void calibrateMPU() {
    Serial.println("[MPU6050] Calibrating... Đặt PHẲNG và giữ yên 3 giây!");
    float rateRollSum = 0, ratePitchSum = 0, rateYawSum = 0;
    float accXSum = 0, accYSum = 0, accZSum = 0;
    for (int i = 0; i < 2000; i++) {
        if (!readMPU_raw_block()) {
            delay(1);
            continue;
        }
        rateRollSum += RateRoll;
        ratePitchSum += RatePitch;
        rateYawSum += RateYaw;
        accXSum += AccX;
        accYSum += AccY;
        accZSum += AccZ;
        delay(1);
    }
    RateCalibrationRoll = rateRollSum / 2000.0f;
    RateCalibrationPitch = ratePitchSum / 2000.0f;
    RateCalibrationYaw = rateYawSum / 2000.0f;
    AccXCalibration = accXSum / 2000.0f;
    AccYCalibration = accYSum / 2000.0f;
    AccZCalibration = (accZSum / 2000.0f) - 1.0f; // TRỪ 1g
    Serial.printf("[MPU6050] Gyro Calib: R=%.3f P=%.3f Y=%.3f\n", 
                  RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw);
    Serial.printf("[MPU6050] Accel Calib: X=%.3f Y=%.3f Z=%.3f\n", 
                  AccXCalibration, AccYCalibration, AccZCalibration);
    MPULoopTimer = micros();
}
// sensorInit: init DHT, GAS, MPU (cấu hình MPU 1 lần)
void sensorInit() {
    Serial.println("[SENSOR] Initializing sensors...");
    dhtSensor.begin();
    Serial.println("[SENSOR] DHT11 initialized");
    pinMode(PIN_GAS_SENSOR, INPUT);
    analogSetAttenuation(ADC_11db); // 0-3.3V range
    Serial.println("[SENSOR] Gas sensor initialized on GPIO1");
    
    Wire.begin(PIN_MPU_SDA, PIN_MPU_SCL);
    Wire.setClock(400000);

    // wakeup and configure MPU6050 ONCE
    writeMPU(MPU6050_PWR_MGMT_1, 0x00);
    delay(50);
    writeMPU(MPU6050_CONFIG, 0x05);      // DLPF
    writeMPU(MPU6050_ACCEL_CONFIG, 0x10); // ±8g
    writeMPU(MPU6050_GYRO_CONFIG, 0x08);  // ±500deg/s
    delay(50);
    Wire.beginTransmission(MPU6050_ADDR);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
        Serial.println("[SENSOR] MPU6050 connected");
        // calibrate (recommended)
        calibrateMPU();
        sensorInitialized = true;
    } else {
        Serial.println("[SENSOR] ERROR: MPU6050 not found!");
        sensorInitialized = false;
    }
}

// ============================================
// Hàm đọc cảm biến GAS (giữ nguyên)
// ============================================// ...existing code...

GasData sensorReadGas() {
    GasData d = {};

    d.raw = analogRead(PIN_GAS_SENSOR);
    d.valid = (d.raw > 0);

    if (!d.valid) return d;

    // ESP32: ADC 12-bit (0-4095), Vref = 3.3V với ADC_11db
    const float Vref = 3.3f;
    const float ADC_MAX = 4095.0f;
    
    d.voltage = (d.raw / ADC_MAX) * Vref;

    // Tính Rs = (Vc - Vout) / Vout * RL
    // Lưu ý: Vc là điện áp cấp cho module MQ (thường 5V từ VIN)
    // Nhưng Vout đo được qua ADC là 0-3.3V
    const float Vc_sensor = 3.3f;  // Điện áp cấp cho module MQ
    
    if (d.voltage > 0.1f) {
        // Nếu module có voltage divider, cần điều chỉnh
        // Giả sử Vout tỷ lệ thuận: Vout_actual = Vout_measured * (5/3.3)
        float Vout_actual = d.voltage * (Vc_sensor / Vref);
        d.rs = ((Vc_sensor - Vout_actual) / Vout_actual) * GAS_RL;
    } else {
        d.rs = 999999;
    }

    // Ratio Rs/R0
    d.ratio = d.rs / GAS_R0;

    // PPM = 613.9 * (Rs/R0)^(-2.074)
    if (d.ratio > 0.01f && d.ratio < 100.0f) {
        d.ppm = 613.9f * pow(d.ratio, -2.074f);
    } else {
        d.ppm = 0;
    }

    // Giới hạn
    if (d.ppm > 10000) d.ppm = 10000;
    if (d.ppm < 0) d.ppm = 0;

    return d;
}

// ...existing code...

// ============================================
// DHT đọc (giữ nguyên)
// ============================================
DHT11Data sensorReadDHT() {
    DHT11Data data;
    data.temperature = dhtSensor.readTemperature();
    data.humidity = dhtSensor.readHumidity();
    if (isnan(data.temperature) || isnan(data.humidity)) {
        data.isValid = false;
        data.temperature = 0;
        data.humidity = 0;
    } else {
        data.isValid = true;
    }
    
    return data;
}

// ============================================
// sensorReadMPU: sử dụng readMPU_raw_block, tính góc, filter bù
// ============================================
MPU6050Data sensorReadMPU() {
    MPU6050Data data;
    
    if (!sensorInitialized) {
        data.accelX = data.accelY = data.accelZ = 0;
        data.gyroX = data.gyroY = data.gyroZ = 0;
        data.isValid = false;
        return data;
    }

    // đọc block raw
    if (!readMPU_raw_block()) {
        data.isValid = false;
        return data;
    }

    // trừ lệch (calibration)
    RateRoll  -= RateCalibrationRoll;
    RatePitch -= RateCalibrationPitch;
    RateYaw   -= RateCalibrationYaw;
    AccX -= AccXCalibration;
    AccY -= AccYCalibration;
    AccZ -= AccZCalibration;

    // góc từ accel (deg)
    AngleRoll = atan2(AccY, sqrt(AccX*AccX + AccZ*AccZ)) * 57.29578f;
    AnglePitch = -atan2(AccX, sqrt(AccY*AccY + AccZ*AccZ)) * 57.29578f;

    // complementary filter
    CompAngleRoll = COMP_ALPHA * (CompAngleRoll + RateRoll * LOOP_TIME) + (1 - COMP_ALPHA) * AngleRoll;
    CompAnglePitch = COMP_ALPHA * (CompAnglePitch + RatePitch * LOOP_TIME) + (1 - COMP_ALPHA) * AnglePitch;

    // optional constrain to avoid weird huge values
    CompAngleRoll = constrain(CompAngleRoll, -180.0f, 180.0f);
    CompAnglePitch = constrain(CompAnglePitch, -180.0f, 180.0f);

    data.accelX = AccX;
    data.accelY = AccY;
    data.accelZ = AccZ;
    data.gyroX = RateRoll;
    data.gyroY = RatePitch;
    data.gyroZ = RateYaw;
    data.isValid = true;
    return data;
}

SensorData sensorReadAll() {
    SensorData data;
    data.timestamp = millis();
    data.dht = sensorReadDHT();
    data.mpu = sensorReadMPU();
    data.gas = sensorReadGas(); 
    data.eventType = EVENT_NORMAL;
    return data;
}
float sensorGetRoll() {
    return CompAngleRoll;
}
float sensorGetPitch() {
    return CompAnglePitch;
}

bool sensorIsOnline() {
    return sensorInitialized;
}


void sensorTest() {
    Serial.println("\n========== SENSOR TEST ==========");
    
    DHT11Data dht = sensorReadDHT();
    Serial.println("\n--- DHT11 ---");
    Serial.printf("T=%.1f°C  H=%.0f%%  Valid=%s\n", 
                  dht.temperature, dht.humidity, dht.isValid ? "YES" : "NO");
    MPU6050Data mpu = sensorReadMPU();
    Serial.println("\n--- MPU6050 ---");
    Serial.printf("Accel: X=%.2fg Y=%.2fg Z=%.2fg\n", mpu.accelX, mpu.accelY, mpu.accelZ);
    Serial.printf("Gyro:  X=%.1f°/s Y=%.1f°/s Z=%.1f°/s\n", mpu.gyroX, mpu.gyroY, mpu.gyroZ);
    Serial.printf("Angle: Roll=%.1f° Pitch=%.1f°\n", CompAngleRoll, CompAnglePitch);
    Serial.printf("Valid=%s\n", mpu.isValid ? "YES" : "NO");
    Serial.println("=================================\n");
    GasData gas = sensorReadGas();
    Serial.println("\n--- GAS SENSOR ---");
    Serial.printf("Raw=%d  V=%.2fV  Rs=%.1fkΩ  Ratio=%.2f  PPM=%.0f\n",
                  gas.raw, gas.voltage, gas.rs, gas.ratio, gas.ppm);
    Serial.printf("Valid=%s\n", gas.valid ? "YES" : "NO");
    
    Serial.println("=================================\n");
}
