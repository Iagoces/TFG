#include <Wire.h>
#include <DHT.h>


#define DHTPIN 8
#define DHTTYPE    DHT11
int  temp=0;

DHT dht(DHTPIN, DHTTYPE);

void setup(){
  Wire.begin(11);
  dht.begin();
  Wire.onRequest(Read_Temp);
}

void Read_Temp(){
   delay(2000);
   Wire.write(temp);

   
}
void loop() {
   delay(2000);
   temp= dht.readTemperature();
}
