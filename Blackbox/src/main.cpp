#include <Arduino.h>
#include <math.h>
#include "modules/sensor_module.h"
#include "modules/network_module.h"
#include "modules/ota_module.h"
extern "C" {
    #include "esp_ota_ops.h"
    #include "esp_partition.h"
}

#define VIBRATION_BUFFER_SIZE 20
float vibrationBuffer[VIBRATION_BUFFER_SIZE];
int bufferIndex = 0;

// Baseline
struct { float accelX, accelY, accelZ; bool isCalibrated; } baseline = {0, 0, 1, false};
unsigned long lastSensorRead = 0;
unsigned long lastPublish = 0;
SensorData currentData;
AlertLevel currentAlert = ALERT_NONE;
float currentVibration = 0.0;
float currentVibrationP2P = 0.0;
float totalTilt = 0.0;
void analyzeSensorData() {
    float ax = currentData.mpu.accelX;
    float ay = currentData.mpu.accelY;
    float az = currentData.mpu.accelZ;
    float mag = sqrt(ax*ax + ay*ay + az*az) - 1.0;
    float accelTotal = sqrt(ax*ax + ay*ay + az*az); 
    vibrationBuffer[bufferIndex++] = mag;
    if (bufferIndex >= VIBRATION_BUFFER_SIZE) bufferIndex = 0;
    float sumSq = 0;
    float maxVal = -1e9;
    float minVal = 1e9;
    for (int i = 0; i < VIBRATION_BUFFER_SIZE; i++) {
        sumSq += vibrationBuffer[i] * vibrationBuffer[i];
        if (vibrationBuffer[i] > maxVal) maxVal = vibrationBuffer[i];
        if (vibrationBuffer[i] < minVal) minVal = vibrationBuffer[i];
    }
    
    currentVibration = sqrt(sumSq / VIBRATION_BUFFER_SIZE);
    currentVibrationP2P = maxVal - minVal;
    
    // 4. Roll & Pitch
    currentData.mpu.roll = sensorGetRoll();
    currentData.mpu.pitch = sensorGetPitch();
    totalTilt = sqrt(currentData.mpu.roll*currentData.mpu.roll + currentData.mpu.pitch*currentData.mpu.pitch); 
    float gasPPM = currentData.gas.ppm-150;
    bool isGasCritical = (gasPPM >= GAS_CRITICAL_PPM);
    bool isGasWarning = (gasPPM >= GAS_WARNING_PPM);
    bool isShock = (accelTotal > 3.0f);
    bool isVibrationCritical = (currentVibration >= VIBRATION_CRITICAL);
    bool isVibrationWarning = (currentVibration >= VIBRATION_WARNING);
    bool isTiltDanger = (totalTilt >= TILT_DANGER_ANGLE);
    bool isTiltWarning = (totalTilt >= TILT_WARNING_ANGLE);
    bool isTempAbnormal = (currentData.dht.temperature > TEMP_MAX_CONCRETE);
    bool isHumidityLow = (currentData.dht.humidity < HUMIDITY_MIN_CONCRETE);
    if (isGasCritical) {
        currentAlert = ALERT_CRITICAL;
        currentData.eventType = EVENT_GAS_CRITICAL;
    }
    else if (isShock) {
        currentAlert = ALERT_CRITICAL;
        currentData.eventType = EVENT_CRASH;
    }
    else if (isVibrationCritical) {
        currentAlert = ALERT_CRITICAL;
        currentData.eventType = EVENT_CRASH;
    }
    else if (isTiltDanger) {
        currentAlert = ALERT_CRITICAL;
        currentData.eventType = EVENT_TILT_DANGER;
    }
    // === WARNING ===
    else if (isGasWarning) {
        currentAlert = ALERT_WARNING;
        currentData.eventType = EVENT_GAS_WARNING;
    }
    else if (isVibrationWarning) {
        currentAlert = ALERT_WARNING;
        currentData.eventType = EVENT_VIBRATION_HIGH;
    }
    else if (isTiltWarning) {
        currentAlert = ALERT_WARNING;
        currentData.eventType = EVENT_TILT_WARNING;
    }
    else if (isTempAbnormal) {
        currentAlert = ALERT_WARNING;
        currentData.eventType = EVENT_TEMP_ABNORMAL;
    }
    else if (isHumidityLow) {
        currentAlert = ALERT_WARNING;
        currentData.eventType = EVENT_HUMIDITY_LOW;
    }
    // === NORMAL ===
    else {
        currentAlert = ALERT_NONE;
        currentData.eventType = EVENT_NORMAL;
    }
}
void ledControl() {
    if (currentAlert == ALERT_CRITICAL) {
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, LOW);
    }  else {
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, HIGH);
    }
}
void controlBuzzer() {
    if (buzzerRemoteControl) {
        digitalWrite(PIN_BUZZER, LOW);
    } else {
        if (currentAlert == ALERT_CRITICAL) {
            digitalWrite(PIN_BUZZER, LOW); 
        } else {
            digitalWrite(PIN_BUZZER, HIGH);  
        }
    }
}
void TaskBootButton(void *pvParameters) {
    pinMode(BOOT_PIN, INPUT_PULLUP);
    uint16_t pressTime = 0;
    const uint16_t PRESS_THRESHOLD = 600; // 600 * 10ms = 6000ms

    while (1) {
        if (digitalRead(BOOT_PIN) == LOW) {
            pressTime++;
            if (pressTime >= PRESS_THRESHOLD) {
                Serial.println("[BOOT] Nhấn nút >6s → Factory!");
                switchToFactory();
                pressTime = 0;
            }
        } else {
            pressTime = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    digitalWrite(PIN_BUZZER, HIGH);  
    for (int i = 0; i < VIBRATION_BUFFER_SIZE; i++) {
        vibrationBuffer[i] = 0.0;
    }
    sensorInit();
    Serial.println("Calibrating... Giữ yên 3s");
    delay(3000);
    SensorData cal = sensorReadAll();
    baseline.accelX = cal.mpu.accelX;
    baseline.accelY = cal.mpu.accelY;
    baseline.accelZ = cal.mpu.accelZ;
    baseline.isCalibrated = true;
    networkInit();
    xTaskCreate(TaskBootButton, "BootButton", 4096, NULL, 1, NULL);
    
    Serial.println("READY\n");
    xTaskCreate(TaskBootButton, "BootButtonTask", 2048, NULL, 1, NULL);
}
void loop() {
    static unsigned long lastMPURead = 0;        
    static unsigned long lastSlowRead = 0;       
    static unsigned long lastPrint = 0;          
    unsigned long now = millis();
    networkMaintain();
    //sensorTest();
    if (now - lastMPURead >= 4) {
        lastMPURead = now;
        currentData.mpu = sensorReadMPU(); 
        analyzeSensorData();  
        ledControl();
        controlBuzzer();
        if (now - lastPrint >= 100) {
            lastPrint = now;
            Serial.printf("[DATA] R=%.1f° P=%.1f° RMS=%.2f Gas=%.0fppm → %s | %s\n",
                          currentData.mpu.roll, currentData.mpu.pitch, 
                          currentVibration, currentData.gas.ppm,
                          alertLevelToString(currentAlert),
                          eventTypeToString(currentData.eventType));
        }
    }
    if (now - lastSlowRead >= 1000) {
        lastSlowRead = now;
        currentData.dht = sensorReadDHT();    
        currentData.gas = sensorReadGas();    
        currentData.timestamp = millis();
    }
    if (networkIsConnected() && (now - lastPublish >= MQTT_PUBLISH_INTERVAL)) {
        lastPublish = now;
        networkPublish(currentData, currentAlert, currentVibration, totalTilt);
    }
}