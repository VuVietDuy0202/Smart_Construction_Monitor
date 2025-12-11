#include "network_module.h"
#include "ota_module.h"
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
bool buzzerRemoteControl = false;
unsigned long lastReconnectAttempt = 0;
//Callback MQTT
void networkMqttCallback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    Serial.printf("[MQTT] %s → %s\n", topic, message);
    if (strcmp(topic, TOPIC_ALARM_CONTROL) == 0) {
        buzzerRemoteControl = (strcmp(message, "ON") == 0);
    }
    if (strcmp(topic, TOPIC_OTA_UPDATE) == 0) {
        if (length > 0 && strlen(message) > 0) {
            if (strcmp(message, OTA_UPDATE_KEY) == 0) { // So sánh với key bảo mật
                Serial.println("[MQTT] Key hợp lệ → Chuyển về Factory...");
                mqttClient.publish(TOPIC_STATUS, "OTA_RESTARTING", true);
                delay(500);
                switchToFactory();
            } else {
                Serial.println("[MQTT] Key OTA sai!");
            }
        } else {
            Serial.println("[MQTT] Bỏ qua retained hoặc message rỗng!");
        }
    }
}
void setupWiFi() {
    Serial.printf("WiFi connecting to %s ", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("connected!");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
}
void networkInit() {
    setupWiFi();
    #ifdef ROOT_CA_CERT
        wifiClient.setCACert(ROOT_CA_CERT);
        Serial.println("[TLS] Using certificate validation");
    #else
        wifiClient.setInsecure();
        Serial.println("[TLS] WARNING: Insecure mode (no cert validation)");
    #endif
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setKeepAlive(5);
    mqttClient.setCallback(networkMqttCallback);
    
    Serial.println("[NETWORK] Init OK");
}
void networkReconnect() {
    if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
        if (millis() - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = millis();
            Serial.print("MQTT connecting...");
            
          if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                                   TOPIC_STATUS, 1, true, "OFFLINE")) {
                Serial.println(" CONNECTED!");
                Serial.printf("   Client: %s\n", MQTT_CLIENT_ID);
                Serial.printf("   Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
                mqttClient.subscribe(TOPIC_ALARM_CONTROL);
                mqttClient.subscribe(TOPIC_OTA_UPDATE);
                mqttClient.publish(TOPIC_STATUS, "ONLINE", true);
                Serial.println("   Subscribed topics OK");
            } else {
                int rc = mqttClient.state();
                Serial.printf(" FAILED (rc=%d)\n", rc);
                
                switch(rc) {
                    case -4: Serial.println("   → Connection timeout"); break;
                    case -3: Serial.println("   → Connection lost"); break;
                    case -2: Serial.println("   → Connect failed"); break;
                    case -1: Serial.println("   → Disconnected"); break;
                    case  1: Serial.println("   → Bad protocol"); break;
                    case  2: Serial.println("   → Bad client ID"); break;
                    case  3: Serial.println("   → Server unavailable"); break;
                    case  4: Serial.println("   → Bad credentials"); break;
                    case  5: Serial.println("   → Not authorized"); break;
                }
            }
        }
    }
}
void networkMaintain() {
    networkReconnect();
    mqttClient.loop();
}
void networkPublish(const SensorData& data, AlertLevel alert, float vibration,float totalTilt) {
    if (!mqttClient.connected()) return;
    
    mqttClient.publish(TOPIC_TEMPERATURE, String(data.dht.temperature, 1).c_str());
    mqttClient.publish(TOPIC_HUMIDITY, String(data.dht.humidity, 0).c_str());
    mqttClient.publish(TOPIC_VIBRATION, String(vibration, 2).c_str());
    mqttClient.publish(TOPIC_GAS_PPM, String(data.gas.ppm-150, 1).c_str());
    mqttClient.publish(TOPIC_TOTAL_TILT, String(totalTilt, 1).c_str());
    mqttClient.publish(TOPIC_ROLL, String(data.mpu.roll, 1).c_str());
    mqttClient.publish(TOPIC_PITCH, String(data.mpu.pitch, 1).c_str());
    
    if (alert >= ALERT_WARNING) {
        mqttClient.publish(TOPIC_ALERT, alertLevelToString(alert)); 
    }
    mqttClient.publish("blackbox/event", eventTypeToString(data.eventType));
    Serial.println("[MQTT] Published");
}

// Kiểm tra kết nối
bool networkIsConnected() {
    bool isConnected = mqttClient.connected();
    static bool lastStatus = false;
    
    if (isConnected != lastStatus) {
        if (isConnected) {
            Serial.println(" MQTT: CONNECTED");
        } else {
            Serial.println("MQTT: DISCONNECTED");
        }
        lastStatus = isConnected;
    }
    return isConnected;
}