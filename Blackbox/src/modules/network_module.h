#ifndef NETWORK_MODULE_H
#define NETWORK_MODULE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "../../include/app_types.h"
#include "../../include/config.h"
#include "../../include/secrets.h"

extern WiFiClientSecure wifiClient;
extern PubSubClient mqttClient;
extern bool buzzerRemoteControl;

void networkInit();
void networkMaintain();
void networkPublish(const SensorData& data, AlertLevel alert, float vibration, float totalTilt);  
bool networkIsConnected();

#endif