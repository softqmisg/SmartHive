#include <Arduino.h>
#include "Wire.h"
#include "SHT2x.h"
#include "main.h"
#include "FreqCountESP.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "ESP32Time.h"
#include "PID_v1.h"
#include "softWire.h"
#include "RTClib_Tiny.h"
// #include "BluetoothSerial.h"
  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

TwoWire I2C_1= TwoWire(0);
TwoWire I2C_2=TwoWire(1);
SoftWire I2C_3(ESP32_GPIO27_RTC_SDA,ESP32_GPIO32_RTC_SCL);
AsyncDelay readInterval;
char swTxBuffer[16];
char swRxBuffer[16];

SHT2x SHT20_Inside;
SHT2x SHT20_Outside;


ESP32Time rtc_int;
RTC_DS1307 rtc_ext;

// BluetoothSerial SerialBT;
double Heater_Setpoint, Heater_Input, Heater_Output;
//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;
PID Heater_PID(&Heater_Input, &Heater_Output, &Heater_Setpoint, Kp, Ki, Kd, DIRECT);
///////////////////////////////////////////////////////////////////
hw_timer_t *timer0_1s=NULL;
portMUX_TYPE timer0Mux=portMUX_INITIALIZER_UNLOCKED;
volatile bool timer0_f=false;
void IRAM_ATTR onTimer0()
{
  portENTER_CRITICAL_ISR(&timer0Mux);
  timer0_f=true;
  portEXIT_CRITICAL_ISR(&timer0Mux);
}
void IRAM_ATTR  onreedRelay(void)
{
  int doorState=digitalRead(ESP32_GPIO23_REED);
}
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 
volatile bool keypress_f=false;
portMUX_TYPE keyMux=portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onKeyPress(void)
{
  button_time=millis();
  if(button_time-last_button_time>250)
  {
    portENTER_CRITICAL_ISR(&keyMux);
    keypress_f=true;
    portEXIT_CRITICAL_ISR(&keyMux);
    last_button_time=button_time;
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("SHT20 LIB Version:");
  Serial.println(SHT2x_LIB_VERSION);
  Serial.println("CPU MHz:"+String(ESP.getCpuFreqMHz()));
  // Set IO mods
  ledcSetup(0,1000,8);ledcAttachPin(ESP32_GPIO19_THERMAL,0);//pwm
  ledcSetup(1,1000,8);ledcAttachPin(ESP32_GPIO18_FAN,1);//pwm  
  ledcSetup(2,1000,8);ledcAttachPin(ESP32_GPIO5_LED0,2);//pwm
  pinMode(ESP32_GPIO4_PUMP,OUTPUT);//gpio
  pinMode(ESP32_GPIO39_PHOTOCELL,ANALOG);//analog input
  pinMode(ESP32_GPIO33_FANFEEDBACK,ANALOG);//analog input
  pinMode(ESP32_GPIO36_THERMALFEEDBACK,ANALOG);//analog input
  pinMode(ESP32_GPIO23_REED,INPUT);//intterupt input
  attachInterrupt(digitalPinToInterrupt(ESP32_GPIO23_REED),onreedRelay,CHANGE);
  pinMode(ESP32_GPIO15_KEYDOWN,INPUT); //interrupt input
  attachInterrupt(digitalPinToInterrupt(ESP32_GPIO15_KEYDOWN),onKeyPress,FALLING);
  FreqCountESP.begin(ESP32_GPIO2_WATERLEVEL,1000,1);//frquncy input
   //init i2c&SHT20//
  I2C_1.begin(ESP32_GPIO21_SDA1,ESP32_GPIO22_SCL1);
  I2C_2.begin(ESP32_GPIO16_SDA2,ESP32_GPIO17_SCL2);
  SHT20_Inside.begin(&I2C_1);
  SHT20_Outside.begin(&I2C_2);
  //init rtc_ext
  I2C_3.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
  I2C_3.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
  I2C_3.setDelay_us(5);
  I2C_3.setTimeout(1000);
  if(!rtc_ext.begin())
  {
    Serial.println("fail init ds1307");
  }
  else
  {
    Serial.println("init ds1307");
  }
  if(rtc_ext.readnvram(2)!=0xAA)
  {
    Serial.println("Write new nvram ds1307 ");
    rtc_ext.adjust(DateTime(2021, 06, 21, 8, 45, 0));
    rtc_ext.writenvram(2,0xAA);
  }
  else
  {
    Serial.println("Read nvram ds1307 was ok");
  }
  //set rtc_int
   DateTime now=rtc_ext.now();
  rtc_int.setTime(now.unixtime());  
  //init SDCARD
  SPIClass spi(HSPI);
  spi.begin(ESP32_GPIO14_SDSCK,ESP32_GPIO12_SDMISO,ESP32_GPIO13_SDMOSI,ESP32_GPIO26_SDCS);
  if(!SD.begin(ESP32_GPIO26_SDCS,spi,80000000))
  {
    Serial.println("Card Mount Failed");
  }
  // init timer0_1s//
  timer0_1s=timerBegin(0,80,true);//80M/80=1Mhz
  timerAttachInterrupt(timer0_1s,&onTimer0,true);
  timerAlarmWrite(timer0_1s,1000000,true);//every 1hz
  timerAlarmEnable(timer0_1s);
  ///////////////////
  // SerialBT.begin("SmartHive");
  ///////////////////
  uint8_t heater_level;
  uint8_t stat = SHT20_Inside.getStatus();
  Serial.println(stat, HEX);
  SHT20_Inside.getHeaterLevel(heater_level);
  Serial.println(heater_level);
   Serial.println();
  
  stat = SHT20_Outside.getStatus();
  Serial.println(stat, HEX);
  SHT20_Outside.getHeaterLevel(heater_level);
  Serial.println(heater_level);
  Serial.println();
  Serial.println();
  /////////////////////////////
  Heater_Setpoint=37.0;
  Heater_Input=(double)SHT20_Inside.getTemperature();
  Heater_PID.SetMode(AUTOMATIC);
}
uint8_t state=1;
void loop() {
  // put your main code here, to run repeatedly:
  if(keypress_f)
  {
    portENTER_CRITICAL_ISR(&keyMux);
    keypress_f=false;
    portEXIT_CRITICAL_ISR(&keyMux);
    Serial.println("Key pressed");
  }
  if(timer0_f)
  {
    portENTER_CRITICAL(&timer0Mux);
    timer0_f=false;
    portEXIT_CRITICAL(&timer0Mux);   

    DateTime now=rtc_ext.now();
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

    switch(state)
    {
      case 1:
        state=1;
        SHT20_Inside.read();
        if(SHT20_Inside.isConnected())
        {
          Serial.print("SHT inside:"); 
          Serial.print(SHT20_Inside.getTemperature(),1);
          Serial.print(",");
          Serial.println(SHT20_Inside.getHumidity(),1);
          Heater_Input=(double)SHT20_Inside.getTemperature();
          Heater_PID.Compute();
          ledcWrite(0,Heater_Output);

        }
        else
        {
          Serial.println ("SHT inside disconnected");
        }
        SHT20_Outside.read();
        if(SHT20_Outside.isConnected())
        {
          Serial.print("SHT Outside:"); 
          Serial.print(SHT20_Outside.getTemperature(),1);
          Serial.print(",");
          Serial.println(SHT20_Outside.getHumidity(),1);
        }
        else
        {
          Serial.println ("SHT outside disconnected");
        }


      break;
      case 2:
        state=1;
        SHT20_Outside.read();
        if(SHT20_Outside.isConnected())
        {
          Serial.print("SHT Outside:"); 
          Serial.print(SHT20_Outside.getTemperature(),1);
          Serial.print(",");
          Serial.println(SHT20_Outside.getHumidity(),1);
        }
        else
        {
          Serial.println ("SHT outside disconnected");
        }

      break;
    }

  }

}