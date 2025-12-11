#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

#include <Arduino.h>
#include <DHT.h>
#include "../../include/app_types.h"
#include "../../include/config.h"

extern DHT dhtSensor;
extern bool sensorInitialized; 
void sensorInit();
DHT11Data sensorReadDHT();
MPU6050Data sensorReadMPU();
SensorData sensorReadAll();
GasData sensorReadGas();  
float sensorGetRoll(); 
float sensorGetPitch();  
bool sensorIsOnline();
void sensorTest();
#endif