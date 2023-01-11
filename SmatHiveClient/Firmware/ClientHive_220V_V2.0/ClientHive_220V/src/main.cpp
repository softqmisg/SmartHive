#include <Arduino.h>
#include <Wifi.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLESecurity.h>
#include "main.h"
#include "hive.h"
#include "UserBLE.h"
#include "math.h"
#include "esp_adc_cal.h"

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "ESP32Time.h"


char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


Hive hive;
///////////////////////////////////////////////////////////////////
hw_timer_t *timer0_1s = NULL;
portMUX_TYPE timer0Mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool timer0_f = false;
void IRAM_ATTR onTimer0()
{
  portENTER_CRITICAL_ISR(&timer0Mux);
  timer0_f = true;
  portEXIT_CRITICAL_ISR(&timer0Mux);
}

uint8_t state = 1;
float Voltage;
int AN_Pot1_Result;
uint32_t readADC_Cal(int ADC_Raw);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  setCpuFrequencyMhz(80);
  Serial.println(__FILE__);
  Serial.print("SHT20 LIB Version:");
  Serial.println(SHT2x_LIB_VERSION);
  Serial.println("Chip cores:" + String(ESP.getChipCores()));
  Serial.println("Chip Model:" + String(ESP.getChipModel()));
  Serial.println("Chip Revision:" + String(ESP.getChipRevision()));
  Serial.println("CPU MHz:" + String(ESP.getCpuFreqMHz()));
  Serial.println("Chip Flash size:" + String(ESP.getFlashChipSize()));
  Serial.println("Chip Flahs Speed:" + String(ESP.getFlashChipSpeed()));
  Serial.println("Chip Free Sketch Space:" + String(ESP.getFreeSketchSpace()));
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  //////////////////////////////////////////////
  // ledcSetup(HEATERPWMCHANNEL, 50, 8);
  // ledcAttachPin(ESP32_GPIO19_THERMAL, HEATERPWMCHANNEL); // pwm
  // ledcWrite(HEATERPWMCHANNEL,64);
  // while(1);
  ////////////////////////////////////////
  hive.begin();
  ////////////////////
  bleInit();
  ///////////////////
  uint8_t heater_level, stat;
  if (hive.sht20Inside.isConnected())
  {
    stat = hive.sht20Inside.getStatus();
    Serial.println(stat, HEX);
    hive.sht20Inside.getHeaterLevel(heater_level);
    Serial.println(heater_level);
  }
  if (hive.sht20Outside.isConnected())
  {
    stat = hive.sht20Outside.getStatus();
    Serial.println(stat, HEX);
    hive.sht20Outside.getHeaterLevel(heater_level);
    Serial.println(heater_level);
    Serial.println();
  }
  /////////////////////////////
  // init timer0_1s//
  timer0_1s = timerBegin(0, 80, true); // 80M/80=1Mhz
  timerAttachInterrupt(timer0_1s, &onTimer0, true);
  timerAlarmWrite(timer0_1s, 1000000, true); // every 1hz
  timerAlarmEnable(timer0_1s);
  // ///////////////////
  Serial.println(hive.gethiveName());
  Serial.println(hive.getPosition().latitude,6);
  Serial.println(hive.getPosition().longitude,6);
  Serial.println(hive.getblepassword());
  
}
long mili=0;
void loop()
{
  // put your main code here, to run repeatedly:

  if (timer0_f)
  {
    portENTER_CRITICAL(&timer0Mux);
    timer0_f = false;
    portEXIT_CRITICAL(&timer0Mux);
    hive.update();

   
  }
}
