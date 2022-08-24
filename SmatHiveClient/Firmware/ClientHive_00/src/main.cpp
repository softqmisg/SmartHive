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

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

uint8_t led_brightness = 0;
Hive hive(2, 5, 1);
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

  /////////////////////
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
    if (hive.isDeviceConnected())
    {
      bleNotifyInitializeData();

      if (sendInitValuesToClient == false)
      {
        sendInitValuesToClient = true;
      }
    }
    else
    {
      sendInitValuesToClient = false;
    }

    led_brightness += 10;
    ledcWrite(LEDPWMCHANNEL, led_brightness); // max duty=2^bits
    ledcWrite(FANPWMCHANNEL, 255-led_brightness); // max duty=2^bits

    Serial.println("Brightness:" + String(led_brightness));
    // hive.FreqCountESP.start();
    // if (hive.FreqCountESP.available())
    // {
    //   hive.FreqCountESP.stop();
    //   uint32_t freq_r = hive.FreqCountESP.read();
    //   Serial.println("Ext PWM Freq:" + String(freq_r));
    //   double cap = 1.44 / (3 * freq_r) * 1e6 - 11.66;
    //   Serial.println("cap(pf):" + String(cap));
    // }
    if (hive.rtcext.isrunning())
    {
      DateTime now = hive.rtcext.now();
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(" (");
      Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
      Serial.print(") ");
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.println();
    }
    if (hive.sht20Inside.isConnected())
    {
      hive.sht20Inside.read();
      Serial.print("SHT inside:");
      Serial.print(hive.sht20Inside.getTemperature(), 1);
      Serial.print(",");
      Serial.println(hive.sht20Inside.getHumidity(), 1);
      // decision on inside temperature && humidity
      hive.controlHeater();
    }
    else
    {
      Serial.println("SHT inside disconnected");
      // decision about PID:
      //  do something ....
    }
    if (hive.sht20Outside.isConnected())
    {
      hive.sht20Outside.read();

      Serial.print("SHT Outside:");
      Serial.print(hive.sht20Outside.getTemperature(), 1);
      Serial.print(",");
      Serial.println(hive.sht20Outside.getHumidity(), 1);
    }
    else
    {
      Serial.println("SHT outside disconnected");
    }
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  }
}