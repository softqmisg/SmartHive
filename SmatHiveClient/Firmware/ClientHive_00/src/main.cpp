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

uint8_t state = 1;
float Voltage;
int AN_Pot1_Result;
uint32_t readADC_Cal(int ADC_Raw);

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
  //////////////////////////////////////////////
  // ledcSetup(HEATERPWMCHANNEL, 1000, 8);
  // ledcAttachPin(ESP32_GPIO19_THERMAL, HEATERPWMCHANNEL); // pwm

  ////////////////////////////////////////
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
  /////////////////////////////
  // init timer0_1s//
  timer0_1s = timerBegin(0, 80, true); // 80M/80=1Mhz
  timerAttachInterrupt(timer0_1s, &onTimer0, true);
  timerAlarmWrite(timer0_1s, 1000000, true); // every 1hz
  timerAlarmEnable(timer0_1s);
  // ///////////////////

  ledcWrite(2, led_brightness); // max duty=2^bits

  //////////////////////////////
  Serial.println(hive.gethiveName());
  Serial.println(hive.getPosition().latitude,6);
  Serial.println(hive.getPosition().longitude,6);
  Serial.println(hive.getblepassword());
  Serial.println(hive.getLight().level);
  Serial.println(hive.getLight().hysteresis);

}

void loop()
{
  // put your main code here, to run repeatedly:

    // ledcWrite(HEATERPWMCHANNEL,1);
    // AN_Pot1_Result = analogRead(ESP32_GPIO36_THERMALFEEDBACK);
    // Voltage = readADC_Cal(AN_Pot1_Result);
    // Serial.println(AN_Pot1_Result);
    // Serial.println(Voltage/1000.0); // Print Voltage (in V)
    // Serial.println(Voltage);      // Print Voltage (in mV)
    // Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    delay(500);
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
    // Serial.println(hive.rtcint.getTimeDate());


  }
}
// uint32_t readADC_Cal(int ADC_Raw)
// {
//   esp_adc_cal_characteristics_t adc_chars;
  
//   esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
//   return(esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
// }