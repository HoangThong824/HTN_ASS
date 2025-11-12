
#include "light_monitor.h"

void light_monitor(void* pvParameters){
    pinMode(LIGHT_SENSOR, INPUT);

    while(1){
        int sensorValue = analogRead(LIGHT_SENSOR);

        glob_light = sensorValue;
        // In ra gia tri anh sang
        Serial.print("Light: ");
        Serial.println(sensorValue); // In voi 2 chu so thap phan
        
        glob_light = sensorValue;
        vTaskDelay(5000); 
    }
}