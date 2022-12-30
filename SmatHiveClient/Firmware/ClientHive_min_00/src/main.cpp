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

/**
 * @brief Key press detction interrupt
 *
 */
unsigned long button_time = 0;
unsigned long last_button_time = 0;
volatile bool keypress_f = false;
portMUX_TYPE keyMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onKeyPress(void)
{
    button_time = millis();
    if (button_time - last_button_time > 300)
    {
        portENTER_CRITICAL_ISR(&keyMux);
        keypress_f = true;
        portEXIT_CRITICAL_ISR(&keyMux);
    }
    last_button_time = button_time;
}
//////////////////////////////////////////////////

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("*****************************");
  Serial.println("* SMART HIVE,Sanjeh Novin   *");
  Serial.println("*****************************");
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
  //   ledcSetup(HEATERPWMCHANNEL, 100, 8);
  //   ledcAttachPin(ESP32_GPIO19_THERMAL,HEATERPWMCHANNEL ); // pwm
  //   ledcSetup(FANPWMCHANNEL, 1000, 8);
  //   ledcAttachPin(ESP32_GPIO18_FAN, FANPWMCHANNEL); // pwm
  
  //   ledcWrite(HEATERPWMCHANNEL, 256 - 255);//50/256*100
  //   ledcWrite(FANPWMCHANNEL, 256 - 255);//0=off ,255=on

  // esp_adc_cal_characteristics_t adc_chars;
  // uint32_t ADC_Raw1,ADC_Raw2,x;
  // x=esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, 1093, &adc_chars);
  // Serial.println(adc_chars.adc_num);
  // Serial.println(adc_chars.atten);
  // Serial.println(adc_chars.bit_width);
  // Serial.println(adc_chars.coeff_a);
  // Serial.println(adc_chars.coeff_b);
  // Serial.println(adc_chars.vref);

  // while(1)
  // {
  // x=esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, 1093, &adc_chars);

  //   ADC_Raw1=0;
  //   for(uint8_t i=0;i<100;i++)
  //   {
  //     //ADC_Raw1+=adc1_get_raw(ESP32_THERMALFEEDBACK_CH);
  //     ADC_Raw1+=(uint32_t)analogRead(ESP32_GPIO36_THERMALFEEDBACK);
  //     delay(1);
  //   }
  //   ADC_Raw1/=100;
  //   Serial.println(String(x)+":Heater:"+String(ADC_Raw1)+":"+String(esp_adc_cal_raw_to_voltage(ADC_Raw1, &adc_chars)));
  //   ADC_Raw2=0;
  //   ADC_Raw2=analogRead(ESP32_GPIO33_FANFEEDBACK);
  //   Serial.println(String(x)+":FAN:"+String(ADC_Raw2)+":"+String(esp_adc_cal_raw_to_voltage(ADC_Raw2, &adc_chars)));

  //   x=esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1093, &adc_chars);
  //   ADC_Raw2=0;
  //   ADC_Raw2=adc1_get_raw(ESP32_PHOTOCELL_CH);
  //   Serial.println(":Photocell:"+String(ADC_Raw2)+":"+String(esp_adc_cal_raw_to_voltage(ADC_Raw2, &adc_chars)));


  //   Serial.println("--------------------------------");
  //   delay(1000);
  // }
  ////////////////////////////////////////
  bleInit();
 hive.begin();
  ////////////////////
  // uint8_t heater_level, stat;
  // if (hive.sht20Inside.isConnected())
  // {
  //   stat = hive.sht20Inside.getStatus();
  //   Serial.println(stat, HEX);
  //   hive.sht20Inside.getHeaterLevel(heater_level);
  //   Serial.println(heater_level);
  // }
  // if (hive.sht20Outside.isConnected())
  // {
  //   stat = hive.sht20Outside.getStatus();
  //   Serial.println(stat, HEX);
  //   hive.sht20Outside.getHeaterLevel(heater_level);
  //   Serial.println(heater_level);
  //   Serial.println();
  // }
  /////////////////////////////
  // init timer0_1s//
  timer0_1s = timerBegin(0, 80, true); // 80M/80=1Mhz
  timerAttachInterrupt(timer0_1s, &onTimer0, true);
  timerAlarmWrite(timer0_1s, 1000000, true); // every 1hz
  timerAlarmEnable(timer0_1s);
  // ///////////////////
   ledcSetup(LEDPWMCHANNEL, 1000, 8);
  ledcAttachPin(ESP32_GPIO5_LED0, LEDPWMCHANNEL);            // pwm: freq=80M/2^bits
  ledcWrite(LEDPWMCHANNEL, led_brightness); // max duty=2^bits
    attachInterrupt(digitalPinToInterrupt(ESP32_GPIO15_KEYDOWN), onKeyPress, FALLING);

  //////////////////////////////
  Serial.println(hive.gethiveName());
  Serial.println(hive.getPosition().latitude,6);
  Serial.println(hive.getPosition().longitude,6);
  Serial.println(hive.getblepassword());

}

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
    // Serial.println(hive.rtcint.getTimeDate());
  }
}
