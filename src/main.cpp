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
#include <WiFiManager.h>

/* Pin numbers -------------------------------------------------------------------------------------------------------*/
#define PIN_READ_SENSOR 36


/*#define SSID "iPhone"
#define PASSWORD "senhasenha"*/

#define SERVER "172.20.10.5"
#define PORT 62445

#define QUEU_SIZE 10

/* Instances ---------------------------------------------------------------------------------------------------------*/

/*IPAddress ip(172, 20, 10, 6); // Endereço IP fixo desejado para o ESP32
IPAddress gateway(172, 20, 10, 1); // Endereço IP do gateway
IPAddress subnet(255, 255, 255, 240); // Máscara de sub-rede
*/
ESP32Time rtc;
Preferences preferences;

WiFiClient client;


/* variables ---------------------------------------------------------------------------------------------------------*/

char key[20];

typedef struct struct_sender_packet{
  uint16_t S1, S2, S3, S4, S5, S6, S7, S8, S9;
  uint8_t hora, min, seg, mSeg;
  uint8_t battery;

} struct_sender_packet;

struct_sender_packet savePacket;
struct_sender_packet getPacket;

uint8_t queuIndex = 0;

/* Private functions -------------------------------------------------------------------------------------------------*/

void ReadData();
void SendDataPacket();
void UpdateQueuIndex();
void TaskGetData(void *parameter);
void PutDataQueu(struct_sender_packet data);

/* Main Application --------------------------------------------------------------------------------------------------*/

void setup() 
{
  Serial.begin(115200);

  rtc.setTime(0, 0, 0, 1, 1, 2024);

  WiFiManager wifiManager;
  wifiManager.setSTAStaticIPConfig(IPAddress(172, 20, 10, 6), IPAddress(172, 20, 10, 1), IPAddress(255, 255, 255, 240)); // Use esta linha para configurar um IP estático
  wifiManager.autoConnect("AP_NAME", "AP_PASSWORD"); // Use esta linha para criar um ponto de acesso e configurar a rede

  Serial.println("Iniciando Conexão");
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);

  client.connect(SERVER, PORT);

  Serial.println("Conectado a rede...");

  analogReadResolution(12);

  preferences.begin("QueuPacket", false);
  queuIndex = preferences.getUInt("QueuIndex", 0);

  xTaskCreatePinnedToCore(TaskGetData, "Task", 10000, NULL, 1, NULL, 0);

}

void loop() 
{
  if (!client.connect(SERVER, PORT))
    return;
    
  while (client.available()) {
    String response = client.readStringUntil('\r');
    Serial.println("Received from server: " + response);
  }
}

void TaskGetData(void *parameter) 
{
  while (true)
   {
    ReadData();
    PutDataQueu(savePacket);
    
    if(queuIndex >= (QUEU_SIZE - 1))
      SendDataPacket();
    
    UpdateQueuIndex();

    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void ReadData()
{
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

void PutDataQueu(struct_sender_packet data) 
{
  sprintf(key, "data%d", queuIndex);
  preferences.putBytes(key, &data, sizeof(data));
}

void SendDataPacket()
{
  for(int x = 0; x < QUEU_SIZE; x++)
  {
    if (!client.connect(SERVER, PORT))
      return;

    sprintf(key, "data%d", x);
    preferences.getBytes(key, &getPacket, sizeof(getPacket));

    client.write((uint8_t*)&getPacket, sizeof(getPacket));
  
    Serial.printf("Get Data: %u | %u-%u-%u  | S1: %u, S2: %u... \n\r", x ,getPacket.hora, getPacket.min, getPacket.seg, getPacket.S1, getPacket.S2);
  
  }
  client.stop();
}

void UpdateQueuIndex() 
{
  queuIndex = (queuIndex + 1) % QUEU_SIZE;
  preferences.putUInt("QueuIndex", queuIndex);
}
