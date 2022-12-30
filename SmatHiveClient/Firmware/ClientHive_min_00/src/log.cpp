#include "log.h"
#include "time.h"
void Logg::begin(ESP32Time *rtc)
{
    // init SDCARD
    spi.begin(ESP32_GPIO14_SDSCK, ESP32_GPIO12_SDMISO, ESP32_GPIO13_SDMOSI, ESP32_GPIO26_SDCS);
    if (!SD.begin(ESP32_GPIO26_SDCS, spi, SDCARDSPEED))
    {
        Serial.println("Card Mount Failed");
    }
    else
    {
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
    }
    cur_time=rtc;
}
void Logg::print(const char *format,...)
{
    ESP_LOGI(TAG)
    //Create File
    if(cur_indexinfile==0)
    {   
        uint8_t cardType = SD.cardType();
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
        if(cardType==CARD_NONE || cardType==CARD_UNKNOWN)
        {

            begin(cur_time);
        }
        // cur_filename="Log_"+cur_time->getDateTime();
        tm t=cur_time->getTimeStruct();
        cur_filename="/Log_"+String(t.tm_year+1900)+"-"+
                        String(t.tm_mon+1)+"-"+
                        String(t.tm_mday)+"_"+
                        String(t.tm_hour)+"-"+
                        String(t.tm_min)+"-"+
                        String(t.tm_sec)+".txt";   
        Serial.println("file name:"+cur_filename);
        cur_indexinfile=0;     
    }
    if(!(cur_file=SD.open(cur_filename.c_str(),FILE_APPEND)))
    {
        Serial.println("file coud not be create or open");
        cur_indexinfile=0;
        SD.end();
        spi.end();
    }
    else
    {
        // cur_file.seek(EOF);
        if(cur_indexinfile==0)
        {
            if(!cur_file.println("Time,Date,Inside Temp,Outside Temp,Inside Humidity,Outside Humidity,Heater Current,Heater Percent,Heater Avg Cur,Heater Mode,Fan Current,Fan Percent,Fan Mode,Hive Mode,SetPoint Temp,PID SetPoint,Weight"))
            {
                Serial.println("write failed");
                SD.end();
                spi.end();
            }
        }
        if(!cur_file.println(format))
        {
            Serial.println("write failed");
            cur_indexinfile=0;
            SD.end();
            spi.end();
        }
        else
        {
            cur_indexinfile++;
        }
        if(cur_indexinfile>24*60) //24 hr*60min
        {
            cur_indexinfile=0;
        }
    }
    cur_file.close();
}
