#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <Arduino.h>

enum AlertLevel { // 
    ALERT_NONE = 0,
    ALERT_WARNING = 2,
    ALERT_CRITICAL = 3
};

enum EventType {
    EVENT_NORMAL = 0,
    EVENT_TILT_WARNING = 1,
    EVENT_TILT_DANGER = 2,
    EVENT_VIBRATION_HIGH = 3,
    EVENT_CRASH = 4,
    EVENT_TEMP_ABNORMAL = 5,
    EVENT_HUMIDITY_LOW = 6,
    EVENT_GAS_WARNING = 7,      
    EVENT_GAS_CRITICAL = 8      
};
struct MPU6050Data {
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float pitch, roll;
    bool isValid;
};

struct DHT11Data {
    float temperature;
    float humidity;
    bool isValid;
};

// ← THÊM: Struct cho Gas sensor
struct GasData {
  int   raw;
  float voltage;
  float rs;
  float ratio;
  float ppm;
  bool  valid;
};

struct SensorData {
    unsigned long timestamp;
    MPU6050Data mpu;
    DHT11Data dht;
    GasData gas;        // ← THÊM
    EventType eventType;
};

// Helper functions
inline const char* alertLevelToString(AlertLevel level) { 
    switch(level) {
        case ALERT_NONE: return "✅ Bình thường";
        case ALERT_WARNING: return " ⚠️ Cảnh báo ";
        case ALERT_CRITICAL: return "Nguy hiểm 🚨";
        default: return "UNKNOWN";
    }
}

inline const char* eventTypeToString(EventType type) { 
    switch(type) {
        case EVENT_NORMAL: return "Bình thường ";
        case EVENT_TILT_WARNING: return " ⚠️Cảnh báo nghiêng";
        case EVENT_TILT_DANGER: return "🚨 Nguy hiểm nghiêng";
        case EVENT_VIBRATION_HIGH: return "Rung cao";
        case EVENT_CRASH: return " 💥 Va chạm";
        case EVENT_TEMP_ABNORMAL: return " 🌡️ Nhiệt độ bất thường";
        case EVENT_HUMIDITY_LOW: return " 💧 Độ ẩm thấp";
        case EVENT_GAS_WARNING: return "⚠️ Cảnh báo khí";     
        case EVENT_GAS_CRITICAL: return "🚨 Nguy hiểm khí"; 
        default: return "UNKNOWN";
    }
}

#endif