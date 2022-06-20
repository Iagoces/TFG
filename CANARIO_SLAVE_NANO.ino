#include <Wire.h>

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
int sensorValue = 0;

void setup(){
  Wire.begin(10);
  Serial.begin(9600);
  Wire.onRequest(Read_Lum);
}

void Read_Lum(){
   delay(2000);
   Wire.write(sensorValue);
}

void loop(){
   delay(2000);
   sensorValue = analogRead(analogInPin)/4;
}
