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

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

uint8_t led_brightness = 0;
Hive hive;
bool sendInitValuesToClient = false;
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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
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
  // ////////////////////////////////////////
  //   ledcSetup(FANPWMCHANNEL, 1000, 8);
  //   ledcAttachPin(ESP32_GPIO18_FAN, FANPWMCHANNEL); // pwm
  //   //digitalWrite(ESP32_GPIO18_FAN,LOW);
  //    ledcWrite(FANPWMCHANNEL, 256 - 0);
  // Serial.println(__DATE__);
  // Serial.println(__TIME__);
  // DateTime now(__DATE__,__TIME__);
  // Serial.println(now.year());
  // Serial.println(now.unixtime());
  // Position pos(35.7,51.42);
  // unsigned long prev=millis();
  // pos.calculateSun(DateTime(2022,9,9,0,0,0));
  // unsigned long next=millis();
  // Serial.println(next-prev);

  // DateTime sun=pos.getSunrise();
  // Serial.println(
  //   String(sun.hour())+":"+
  //   String(sun.minute())+":"+
  //   String(sun.second())
  // );
  // sun=pos.getSunset();
  // Serial.println(
  //   String(sun.hour())+":"+
  //   String(sun.minute())+":"+
  //   String(sun.second())
  // );  
  // sun=pos.getNoon();
  // Serial.println(
  //   String(sun.hour())+":"+
  //   String(sun.minute())+":"+
  //   String(sun.second())
  // );    
  // while(1);
  // /////////////////////
  hive.begin();
  ////////////////////
  // SerialBT.begin("SmartHive");
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
  // /////////////////////////////
  // init timer0_1s//
  timer0_1s = timerBegin(0, 80, true); // 80M/80=1Mhz
  timerAttachInterrupt(timer0_1s, &onTimer0, true);
  timerAlarmWrite(timer0_1s, 1000000, true); // every 1hz
  timerAlarmEnable(timer0_1s);
  // ///////////////////

  ledcWrite(2, led_brightness); // max duty=2^bits

  ////////////////////////////////
  Serial.println(hive.gethiveName());
  Serial.println(hive.getPosition().latitude,6);
  Serial.println(hive.getPosition().longitude,6);
  Serial.println(hive.getblepassword());
  Serial.println(hive.getLight().level);
  Serial.println(hive.getLight().hysteresis);
  
}
uint8_t state = 1;
void loop()
{
  // put your main code here, to run repeatedly:

  if (keypress_f)
  {
    portENTER_CRITICAL_ISR(&keyMux);
    keypress_f = false;
    portEXIT_CRITICAL_ISR(&keyMux);
    Serial.println("Key pressed");
  }
  if (timer0_f)
  {
    portENTER_CRITICAL(&timer0Mux);
    timer0_f = false;
    portEXIT_CRITICAL(&timer0Mux);
    hive.update();

    if (hive.isDeviceConnected())
    {
      if (sendInitValuesToClient == false)
      {
        sendInitValuesToClient = true;
        bleNotify(READONLY_CHARACTRISTIC_NUMBER);
      }
    }
    else
    {
      sendInitValuesToClient = false;
    }


    led_brightness += 10;
    ledcWrite(LEDPWMCHANNEL, led_brightness);       // max duty=2^bits

  //   Serial.println("Brightness:" + String(led_brightness));
  //   if (hive.rtcext.isrunning())
  //   {
  //     DateTime now = hive.rtcext.now();
  //     Serial.print(now.year(), DEC);
  //     Serial.print('/');
  //     Serial.print(now.month(), DEC);
  //     Serial.print('/');
  //     Serial.print(now.day(), DEC);
  //     Serial.print(" (");
  //     Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  //     Serial.print(") ");
  //     Serial.print(now.hour(), DEC);
  //     Serial.print(':');
  //     Serial.print(now.minute(), DEC);
  //     Serial.print(':');
  //     Serial.print(now.second(), DEC);
  //     Serial.println();
  //   }
  //   if (hive.sht20Inside.isConnected())
  //   {
  //     Serial.print("SHT inside:");
  //     Serial.print(hive.getInsideTemperature(), 1);
  //     Serial.print(",");
  //     Serial.println(hive.getInsideHumidity(), 0);
  //   }
  //   else
  //   {
  //     Serial.println("SHT inside disconnected");
  //   }
  //   if (hive.sht20Outside.isConnected())
  //   {
  //     Serial.print("SHT Outside:");
  //     Serial.print(hive.getOutsideTemperature(), 1);
  //     Serial.print(",");
  //     Serial.println(hive.getOutsideHumidity(), 0);
  //   }
  //   else
  //   {
  //     Serial.println("SHT outside disconnected");
  //   }
  //   Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  }
}