#include <Wire.h>

//Pino em que o sensor esta conectado
const int analogInPin = A0;

//Variaveis Globais
int sensorValue = 0;

void setup(){
  //Inicial o Slave no Endreço 0x0A
  Wire.begin(10);
  //Cria a funcao para quando o mestre pede uma leitura
  Wire.onRequest(Read_Lum);
}

//Funcao para enviar o valor lido
void Read_Lum(){
   delay(2000);
   Wire.write(sensorValue);
}

void loop(){
   delay(2000);
  //Le o valor do sensor e divide por 4 para poder enviar um de 8 bits que é o suportado pelo I2C
   sensorValue = analogRead(analogInPin)/4;
}
