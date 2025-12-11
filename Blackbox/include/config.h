#ifndef CONFIG_H
#define CONFIG_H

// System Info
#define DEVICE_NAME "DUY"
#define FIRMWARE_VERSION "1.0.0"
// Pins
#define LED1 35
#define LED2 36
#define BOOT_PIN 0
#define PIN_DHT 40
#define PIN_MPU_SDA 5
#define PIN_MPU_SCL 7
#define PIN_BUZZER 6
#define PIN_GAS_SENSOR 8 

// MQTT Topics
#define TOPIC_TEMPERATURE "blackbox/temperature"
#define TOPIC_HUMIDITY "blackbox/humidity"
#define TOPIC_VIBRATION "blackbox/vibration"
#define TOPIC_ROLL "blackbox/roll"
#define TOPIC_PITCH "blackbox/pitch"
#define TOPIC_ALERT "blackbox/alert"
#define TOPIC_STATUS "blackbox/status"
#define TOPIC_ALARM_CONTROL "blackbox/alarm/control"
#define TOPIC_OTA_UPDATE "blackbox/ota"    
#define TOPIC_GAS_PPM "blackbox/gas/ppm"  
#define TOPIC_TOTAL_TILT "blackbox/total_tilt"
#define TOPIC_EVENT "blackbox/event"

// Thresholds
#define TEMP_MAX_CONCRETE 35.0
#define HUMIDITY_MIN_CONCRETE 50.0
#define TILT_WARNING_ANGLE 10.0
#define TILT_DANGER_ANGLE 15.0
#define VIBRATION_WARNING 0.5
#define VIBRATION_CRITICAL 1
#define GAS_WARNING_PPM 300      // ← THÊM: Ngưỡng cảnh báo (300 ppm)
#define GAS_CRITICAL_PPM 800  
#define GAS_RL 10              // ← Load resistance (10kΩ)
#define GAS_R0 10   
// Timing
#define SENSOR_READ_INTERVAL 4
#define MQTT_PUBLISH_INTERVAL 1000
#define SERIAL_BAUD_RATE 115200

#endif
