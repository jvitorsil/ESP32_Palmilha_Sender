/**
**************************************************************************************************************
* @file main.cpp
* @author ~~~~~~~~
* @version V0.1.0
* @date 18-Mar-2024
* @brief FAPEMIG Project - Palmilha
*************************************************************************************************************
*/

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Time.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <freertos/task.h>
#include <Preferences.h>

/* Pin numbers -------------------------------------------------------------------------------------------------------*/

#define PIN_READ_SENSOR 36


#define SSID "Erro404"
#define PASSWORD "semsenha"

#define SERVER "192.168.100.13"
#define PORT 62445

/* Instances ---------------------------------------------------------------------------------------------------------*/

IPAddress ip(192, 168, 100, 20); // Endereço IP fixo desejado para o ESP32
IPAddress gateway(192, 168, 100, 1); // Endereço IP do gateway
IPAddress subnet(255, 255, 255, 0); // Máscara de sub-rede


ESP32Time rtc;
Preferences preferences;

WiFiClient client;


/* variables ---------------------------------------------------------------------------------------------------------*/

  char key[20];

typedef struct struct_message{
  uint16_t S1, S2, S3, S4, S5, S6, S7, S8, S9;
  uint8_t hora, min, seg, mSeg;
  uint8_t battery;

} struct_message;

struct_message savePacket;
struct_message getPacket;

uint8_t queuIndex = 0;


void PutDataQueu(struct_message data);
void GetDataQueu();
void UpdateQueuIndex();
void TaskGetData(void *parameter);
void SendTCPData(struct_message data);
void ReadData();

void setup() {
  Serial.begin(115200);

  rtc.setTime(0, 0, 0, 1, 1, 2024);

  WiFi.config(ip, gateway, subnet);

  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  Serial.println("Iniciando Conexão");
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);

  Serial.println("Conectado a rede...");

  analogReadResolution(12);

  preferences.begin("QueuPacket", false);
  queuIndex = preferences.getUInt("queuIndex", 0);

  xTaskCreatePinnedToCore(TaskGetData, "Task", 10000, NULL, 1, NULL, 0);

}

void loop() {
}

void TaskGetData(void *parameter) 
{
  while (true)
   {
    ReadData();
    PutDataQueu(savePacket);
    
    if(queuIndex >= 9)
      GetDataQueu();
    
    UpdateQueuIndex();

    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void ReadData(){
    savePacket.S1 = (analogRead(PIN_READ_SENSOR) << 4) + 1;
    savePacket.S2 = (analogRead(PIN_READ_SENSOR) << 4) + 2;
    savePacket.S3 = (analogRead(PIN_READ_SENSOR) << 4) + 3;
    savePacket.S4 = (analogRead(PIN_READ_SENSOR) << 4) + 4;
    savePacket.S5 = (analogRead(PIN_READ_SENSOR) << 4) + 5;
    savePacket.S6 = (analogRead(PIN_READ_SENSOR) << 4) + 6;
    savePacket.S7 = (analogRead(PIN_READ_SENSOR) << 4) + 7;
    savePacket.S8 = (analogRead(PIN_READ_SENSOR) << 4) + 8;
    savePacket.S9 = (analogRead(PIN_READ_SENSOR) << 4) + 9;

    savePacket.hora = rtc.getHour();
    savePacket.min = rtc.getMinute();
    savePacket.seg = rtc.getSecond();
    savePacket.mSeg = rtc.getSecond();

    savePacket.battery = 250;
}

void PutDataQueu(struct_message data) {
  sprintf(key, "data%d", queuIndex);
  preferences.putBytes(key, &data, sizeof(data));

  Serial.printf("%u-%u-%u | S1: %u, S2: %u... \n\r", data.hora, data.min, data.seg, data.S1, data.S2);
}

void GetDataQueu(){
  for(int x = 0; x < 10; x++){
  
    sprintf(key, "data%d", x);
    preferences.getBytes(key, &getPacket, sizeof(getPacket));

    Serial.printf("Get Data: %u | %u-%u-%u  | S1: %u, S2: %u... \n\r", x ,getPacket.hora, getPacket.min, getPacket.seg, getPacket.S1, getPacket.S2);

    SendTCPData(getPacket);
  }
}

void UpdateQueuIndex() {
  queuIndex = (queuIndex + 1) % 10;
  preferences.putUInt("queuIndex", queuIndex);
}


void SendTCPData(struct_message data) {
  if (!client.connect(SERVER, PORT))
    return;

  client.write((uint8_t*)&data, sizeof(data));
  client.stop();
}