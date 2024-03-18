/**
**************************************************************************************************************
* @file main.cpp
* @author João Vitor Silva <joaovitor_s2015@ufu.br>
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

#define PIN_S1 36
#define PIN_S2 39
#define PIN_S3 34
#define PIN_S4 35
#define PIN_S5 32
#define PIN_S6 33
#define PIN_S7 36
#define PIN_S8 39
#define PIN_S9 34

#define SSID "iPhone"
#define PASSWORD "senhasenha"

/* Instances ---------------------------------------------------------------------------------------------------------*/

IPAddress ip(192, 168, 1, 200); // Endereço IP fixo desejado para o ESP32
IPAddress gateway(192, 168, 1, 1); // Endereço IP do gateway
IPAddress subnet(255, 255, 255, 0); // Máscara de sub-rede

ESP32Time rtc;
Preferences preferences;


/* variables ---------------------------------------------------------------------------------------------------------*/

typedef struct struct_message{
  uint16_t S1, S2, S3, S4, S5, S6, S7, S8, S9;

  uint8_t Battery;
  uint8_t hora;
  uint8_t min;
  uint8_t seg;
  uint8_t mseg;

} struct_message;

struct_message myData;
struct_message getData;

uint8_t writeIndex = 0;


void saveDataToPreferences(struct_message data);
void getDataToPreferences(struct_message data);
void updateWriteIndex();
void TaskGetData(void *parameter);

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

  preferences.begin("myApp", false);
  writeIndex = preferences.getUInt("writeIndex", 0);

  xTaskCreatePinnedToCore(TaskGetData, "Task", 10000, NULL, 1, NULL, 0);

}

void loop() {
}

void TaskGetData(void *parameter) 
{
  while (true)
   {
    myData.S1 = (analogRead(PIN_S1) << 4) + 1;
    myData.S2 = (analogRead(PIN_S2) << 4) + 2;
    myData.S3 = (analogRead(PIN_S3) << 4) + 3;
    myData.S4 = (analogRead(PIN_S4) << 4) + 4;
    myData.S5 = (analogRead(PIN_S5) << 4) + 5;
    myData.S6 = (analogRead(PIN_S6) << 4) + 6;
    myData.S7 = (analogRead(PIN_S7) << 4) + 7;
    myData.S8 = (analogRead(PIN_S8) << 4) + 8;
    myData.S9 = (analogRead(PIN_S9) << 4) + 9;

    myData.hora = rtc.getHour();
    myData.min = rtc.getMinute();
    myData.seg = rtc.getSecond();

    saveDataToPreferences(myData);

    writeIndex = preferences.getUInt("writeIndex", 0);
    if(writeIndex >= 9){
      getDataToPreferences(myData);
    }
    updateWriteIndex();


    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
  // uint16_t Val12bits = myData.S1 >> 4;
  // uint8_t Val4bits = myData.S1 & 0xF;
  // Serial.printf("Valor retornado: %u e %u\n\r", Val12bits, Val4bits);
}

void saveDataToPreferences(struct_message data) {
  char key[20];
  sprintf(key, "data%d", writeIndex);
  preferences.putBytes(key, &data, sizeof(data));

  Serial.println("Set Data: ");
  Serial.printf("%u | %u-%u-%u  | S1: %u, S2: %u, ", writeIndex, data.hora, data.min, data.seg, data.S1, data.S2);
  Serial.println("\n");
}

void getDataToPreferences(struct_message data) {
  for(uint8_t x = 0; x < 10; x++){
    char key[20];
    sprintf(key, "data%d", x);
    preferences.getBytes(key, &getData, sizeof(getData));
    Serial.println("Get Data: ");
    Serial.printf("%u | %u-%u-%u  | S1: %u, S2: %u, ", x , getData.hora, getData.min, getData.seg, getData.S1, getData.S2);
    Serial.println("\n");
  }
}

void updateWriteIndex() {
  writeIndex = (writeIndex + 1) % 10; // Assuming you want to store 10 sets of data
  preferences.putUInt("writeIndex", writeIndex); // Update the write index in Preferences
}