#include "hive.h"
#include "buildTime.h"

TwoWire I2C_1(0);
TwoWire I2C_2(1);
SoftWire I2C_3(ESP32_GPIO27_RTC_SDA, ESP32_GPIO32_RTC_SCL);

/**
 * @brief Key press detction interrupt
 * 
 */
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 
volatile bool keypress_f=false;
portMUX_TYPE keyMux=portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onKeyPress(void)
{
  button_time=millis();
  if(button_time-last_button_time>300)
  {
    portENTER_CRITICAL_ISR(&keyMux);
    keypress_f=true;
    portEXIT_CRITICAL_ISR(&keyMux);
  }
    last_button_time=button_time;

}
/**
 * @brief Interrupt for Door openning with redrelay
 * 
 */
void  Hive::onReedrelayISR(void)
{
    if(sHive!=0)
        sHive->setDoorState(digitalRead(ESP32_GPIO23_REED));
}
/**
 * @brief Hive class start
 * 
 */
Hive* Hive::sHive=0;
void Hive::begin(void)
{
    sHive=this;
    // Set IO mods
    ledcSetup(HEATERPWMCHANNEL, 1000, 8);
    ledcAttachPin(ESP32_GPIO19_THERMAL, 0); // pwm
    ledcSetup(FANPWMCHANNEL, 5000, 8);
    ledcAttachPin(ESP32_GPIO18_FAN, 1); // pwm
    ledcSetup(LEDPWMCHANNEL, 1000, 8);
    ledcAttachPin(ESP32_GPIO5_LED0, 2);            // pwm: freq=80M/2^bits
    pinMode(ESP32_GPIO4_PUMP, OUTPUT);             // gpio
    pinMode(ESP32_GPIO39_PHOTOCELL, ANALOG);       // analog input
    pinMode(ESP32_GPIO33_FANFEEDBACK, ANALOG);     // analog input
    pinMode(ESP32_GPIO36_THERMALFEEDBACK, ANALOG); // analog input
    pinMode(ESP32_GPIO23_REED, INPUT);             // intterupt input
    attachInterrupt(digitalPinToInterrupt(ESP32_GPIO23_REED), Hive::onReedrelayISR, CHANGE);
    pinMode(ESP32_GPIO15_KEYDOWN, INPUT); // interrupt input
    attachInterrupt(digitalPinToInterrupt(ESP32_GPIO15_KEYDOWN), onKeyPress, FALLING);

    setDoorState(digitalRead(ESP32_GPIO23_REED));
    ///
    I2C_1.begin(ESP32_GPIO21_SDA1, ESP32_GPIO22_SCL1);
    I2C_1.setTimeOut(1000);
    I2C_2.begin(ESP32_GPIO16_SDA2, ESP32_GPIO17_SCL2);
    I2C_2.setTimeOut(1000);
    sht20Inside.begin(&I2C_1);
    sht20Outside.begin(&I2C_2);
    // init rtc_ext
    I2C_3.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
    I2C_3.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
    I2C_3.setDelay_us(5);
    I2C_3.setTimeout(1000);

    if (!rtcext.begin(&I2C_3))
    {
        Serial.println("fail init ds1307");
    }
    else
    {
        Serial.println("init ds1307");
    }
    if (rtcext.readnvram(DS1307_NVRAM_CHECK_BAT) != 0xAA)
    {
        rtcext.writenvram(DS1307_NVRAM_CHECK_BAT, 0xAA);
        
        rtcext.adjust(DateTime(BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN, BUILD_SEC));
        rtcneedAdjust = true;
        Serial.println("Write new nvram ds1307 ");
    }
    else
    {
        rtcneedAdjust = false;
        Serial.println("Read nvram ds1307 was ok");
    }
    rtcint.setTime(rtcext.now().unixtime());
    // /////////////////////////////////
    // // init SDCARD
    // SPIClass spi(HSPI);
    // spi.begin(ESP32_GPIO14_SDSCK, ESP32_GPIO12_SDMISO, ESP32_GPIO13_SDMOSI, ESP32_GPIO26_SDCS);
    // if (!SD.begin(ESP32_GPIO26_SDCS, spi, SDCARDSPEED))
    // {
    //     Serial.println("Card Mount Failed");
    // }
    // uint8_t cardType = SD.cardType();
    // if (cardType == CARD_NONE)
    // {
    //     Serial.println("No SD card attached");
    // }
    // else
    // {
    //     Serial.print("SD Card Type: ");
    //     if (cardType == CARD_MMC)
    //     {
    //         Serial.println("MMC");
    //     }
    //     else if (cardType == CARD_SD)
    //     {
    //         Serial.println("SDSC");
    //     }
    //     else if (cardType == CARD_SDHC)
    //     {
    //         Serial.println("SDHC");
    //     }
    //     else
    //     {
    //         Serial.println("UNKNOWN");
    //     }
    //     uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    //     Serial.printf("SD Card Size: %lluMB\n", cardSize);
    // }
    ////////////////////////////
    heaterInput = sht20Inside.getTemperature();
    heater.SetSampleTime(1000);
    heater.SetMode(AUTOMATIC);
    /////////////////////////////
    FreqCountESP.begin(ESP32_GPIO2_WATERLEVEL, 1000, 1); // frequncy input
}
void Hive::controlHeater(void)
{
    heaterInput = sht20Inside.getTemperature();
    heater.Compute();
    ledcWrite(HEATERPWMCHANNEL, 255-heaterOutput);
}