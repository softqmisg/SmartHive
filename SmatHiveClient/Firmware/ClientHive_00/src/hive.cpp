#include "hive.h"
TwoWire I2C_1(0);
TwoWire I2C_2(1);
SoftWire I2C_3(ESP32_GPIO27_RTC_SDA, ESP32_GPIO32_RTC_SCL);

void Hive::begin(void)
{
    
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
        rtcext.adjust(DateTime(2022, 8, 13, 51, 45, 0));
        need_rtcadjust = true;
        Serial.println("Write new nvram ds1307 ");
    }
    else
    {
        need_rtcadjust = false;
        Serial.println("Read nvram ds1307 was ok");
    }
    rtcint.setTime(rtcext.now().unixtime());
    /////////////////////////////////
    // init SDCARD
    SPIClass spi(HSPI);
    spi.begin(ESP32_GPIO14_SDSCK, ESP32_GPIO12_SDMISO, ESP32_GPIO13_SDMOSI, ESP32_GPIO26_SDCS);
    if (!SD.begin(ESP32_GPIO26_SDCS, spi, SDCARDSPEED))
    {
        Serial.println("Card Mount Failed");
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
    }
    else
    {
        Serial.print("SD Card Type: ");
        if (cardType == CARD_MMC)
        {
            Serial.println("MMC");
        }
        else if (cardType == CARD_SD)
        {
            Serial.println("SDSC");
        }
        else if (cardType == CARD_SDHC)
        {
            Serial.println("SDHC");
        }
        else
        {
            Serial.println("UNKNOWN");
        }
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);
    }
    ////////////////////////////
    heaterInput=sht20Inside.getTemperature();
    heater.SetSampleTime(1000);
    heater.SetMode(AUTOMATIC);
    /////////////////////////////
    FreqCountESP.begin(ESP32_GPIO2_WATERLEVEL, 1000, 1); // frequncy input
}
void Hive::controlHeater(void)
{
    heaterInput=sht20Inside.getTemperature();
    heater.Compute();
    ledcWrite(HEATERCHANNEL,heaterOutput);
}