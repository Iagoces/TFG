#include <Wire.h>
#include <DHT.h>

//Define o pino em que o sensor esta conectado e o tipo do sensor
#define DHTPIN 8
#define DHTTYPE    DHT11

//Cria as variaveis globais
int  temp=0;

//Inicial o dht no pino e no tipo do sensor
DHT dht(DHTPIN, DHTTYPE);

void setup(){
  //Inicia o I2C no endereco 0x0B
  Wire.begin(11);
  dht.begin();
  Wire.onRequest(Read_Temp);
}

//Funcao para enviar a temperatura lida
void Read_Temp(){
   delay(2000);
   Wire.write(temp); 
}


void loop() {
   delay(2000);
  //Faz leitura do sensor
   temp= dht.readTemperature();
}
