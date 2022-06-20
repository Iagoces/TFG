//Inclui as bibliotecas necessarias
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <SparkFun_MMA8452Q.h>
#include <WiFi.h> 
#include <RTClib.h>

//Cria os prototipos das fuceos
Adafruit_VL53L0X s_distance = Adafruit_VL53L0X();
MMA8452Q s_ace;  
RTC_DS1307 rtc;

//Define a rede Wi-fi
const char* ssid = "ESP32-AP"; //Nome
const char* pass = "12345678"; //Senha
WiFiServer sv(80); //Cria um servidor na porta 80

//Cria as tarefas
TaskHandle_t Address_Finder;
TaskHandle_t Read_Temperature;
TaskHandle_t Read_Luminosity;
TaskHandle_t Read_Distance;
TaskHandle_t Read_Accelerometer;
TaskHandle_t Read_Date;
TaskHandle_t Wifi;

//Crias as variaveis globias
int nDevices, error, address, temp, lum, dist;
int out, dia, mes, ano, hora, minu;
float x, y, z;

void setup(){
  //Inicial a serial e o LED
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  delay(10);
  
  //Inicia o i2c, o sensor de destancia e o acelerometro
  Wire.begin();
  s_distance.begin();
  s_ace.begin();

  //Inicia o RTC e define uma hora inicial
  rtc.begin();
  rtc.adjust(DateTime(2022, 6, 3, 2, 2, 00));

  //Inicia o wifi e cria uma tarefa para enviar os dados
  WiFi.softAP(ssid, pass);
  IPAddress ip = WiFi.softAPIP();
  sv.begin();
  xTaskCreatePinnedToCore(Wifi_Code,"Wifi",8000,NULL,1,&Wifi,0); 
}

//Tarefa para encontrar os enderecos
void Address_Finder_Code( void * parameter ){
  Serial.print("Scanning...\n");
  nDevices = 0;
  for(address = 1; address < 127; address++ ){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if(error == 0){
      Serial.print("I2C device found at address 0x");
      if(address<16){Serial.print("0");}
      Serial.println(address,HEX);
      nDevices++;
    }
    else if(error==4){
      Serial.print("Unknow error at address 0x");
      if (address<16){Serial.print("0");}
      Serial.println(address,HEX);
    }   
  }
  if(nDevices == 0){
    Serial.print("No I2C devices found\n");
  }
  else{Serial.print("Done\n");}
  vTaskDelete( NULL );
}

//Tarefa para ler a temperatura
void Read_Temperature_Code( void * parameter ){
    Wire.requestFrom(0xB, 1);
    temp = Wire.read();
    Serial.print("Temperatura(°C):");
    Serial.println(temp);
    vTaskDelete( NULL );
}

//Tarefa para ler a luminosidade
void Read_Luminosity_Code( void * parameter ){
    Wire.requestFrom(0xA, 2, 1);
    lum = Wire.read();
    Serial.print("Luminosidade(Lux):");
    Serial.println(lum);
    vTaskDelete( NULL );
}

//Tarefa para ler a distancia
void Read_Distance_Code( void * parameter ){
    VL53L0X_RangingMeasurementData_t measure;
    s_distance.rangingTest(&measure, false);
    if (measure.RangeStatus != 4){
    dist = measure.RangeMilliMeter;  
    Serial.print("Distance (mm):"); 
    Serial.println(dist);
    out = 0;
  } else{
    Serial.println(" out of range ");
    out = 1;
  }
    vTaskDelete( NULL );
}

//Tarefa para ler a posicao
void Read_Accelerometer_Code( void * parameter ){
    while(s_ace.available() != 1){}
    x = s_ace.getCalculatedX();
    y = s_ace.getCalculatedY();
    z = s_ace.getCalculatedZ();
    Serial.print("X:");Serial.print(x, 3);
    Serial.print("\t");
    Serial.print("Y:");Serial.print(y, 3);
    Serial.print("\t");
    Serial.print("Z:");Serial.print(z, 3);
    Serial.println("\t");
    vTaskDelete( NULL );
}

//Tarefa para ler a data
void Read_Date_Code(void *parameter){
  DateTime complete_data = rtc.now();  
  dia = complete_data.day();
  mes = complete_data.month();
  ano = complete_data.year();
  hora = complete_data.hour();
  minu = complete_data.minute();
  Serial.print("Dia:"); 
  if(dia < 10){
    Serial.print("0");Serial.print(dia);
  } else{Serial.print(dia);} 
  Serial.print('/');
  if(mes < 10){
    Serial.print("0");Serial.print(mes);
  }else{Serial.print(mes);}
  Serial.print('/'); 
  Serial.println(ano); 
  
  Serial.print("Horario:");
  if(hora < 10){
    Serial.print("0");Serial.print(hora);
  } else{Serial.print(hora);}
  Serial.print(':');
  if(minu < 10){
    Serial.print("0");Serial.println(minu);
  } else{Serial.println(minu);} 
  vTaskDelete( NULL );
}


//Tarefa para enviar os dados via Wi-fi
void Wifi_Code( void * parameter ){
  delay(10000); 
  for(;;){
  delay(1000);
  WiFiClient client = sv.available();
 if(client){
  String line = "";
  while(client.connected()){
    if (client.available()){
      char c = client.read(); 
      if (c == '\n'){
          if (line.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            
            client.print("<body bgcolor='#836FFF'>");
            
            client.print("<h1><font color='#F8F8FF'> MODULO DE i2c </font></h1>");
            
            client.print("Temperatura(&degC):"); 
            client.print(temp);
            
            client.print("<br>Luminosidade(Lux):"); 
            client.print(lum);

            if(out == 0){
              client.print("<br>Distancia (mm):"); 
              client.print(dist);
            } else if(out == 1){
              client.print("<br>Distancia (mm): out of range"); 
            }

            client.print("<br>X:"); 
            client.print(x);
            client.print("     Y:");
            client.print(y);
            client.print("     Z:");
            client.print(z);
            
            client.print("<br>Ultima atualizacao as:");
            if(hora < 10){
              client.print("0");
              client.print(hora);
            }else{client.print(hora);}
            client.print(":"); 
            if(minu < 10){
              client.print("0");
              client.print(minu);
            }else{client.print(minu);}
            client.print("<br>Do dia ");
            if(dia < 10){
              client.print("0");
              client.print(dia);
            }else{client.print(dia);}
            client.print("/"); 
            if(mes < 10){
              client.print("0");
              client.print(mes);
            }else{client.print(mes);} 
            client.print("/"); client.print(ano);
            client.print("<meta http-equiv='refresh' content='6'>");
            break;
  }else{line = "";}
  }else if (c != '\r') {line += c;}
}}client.stop();}}}

//Loop para leitura
void loop(){
  
  //ENCONTRA O ENDEREÇO
  xTaskCreatePinnedToCore(Address_Finder_Code,"Address_Finder",10000,NULL,1,&Address_Finder,1);                         
  vTaskDelay(2000/portTICK_PERIOD_MS);

  //LEITURA DE TEMPERATURA
  xTaskCreatePinnedToCore(Read_Temperature_Code,"Read_Temperature",10000,NULL,1,&Read_Temperature,1);
  vTaskDelay(2000/portTICK_PERIOD_MS);

  //LEITURA DE LUMINOSIDADE
  xTaskCreatePinnedToCore(Read_Luminosity_Code,"Read_Luminosity",10000,NULL,1,&Read_Luminosity,1);
  vTaskDelay(1000/portTICK_PERIOD_MS);  

  //LEITURA DE DISTANCIA
  xTaskCreatePinnedToCore(Read_Distance_Code,"Read_Distance",10000,NULL,1,&Read_Distance,1);
  vTaskDelay(2000/portTICK_PERIOD_MS); 

  //LEITYRA DE VELOCIDADE
  xTaskCreatePinnedToCore(Read_Accelerometer_Code,"Read_Accelerometer",10000,NULL,1,&Read_Accelerometer,1);
  vTaskDelay(2000/portTICK_PERIOD_MS); 

  //LEITURA DA DATA
  xTaskCreatePinnedToCore(Read_Date_Code,"Read_Date",10000,NULL,1,&Read_Date,1);
  vTaskDelay(2000/portTICK_PERIOD_MS);
}
