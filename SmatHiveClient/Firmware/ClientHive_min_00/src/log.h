#ifndef __LOG__H__
#define __LOG__H__
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "main.h"
#include "ESP32Time.h"
#define SDCARDSPEED 4000000
class Logg
{
    private:
        uint8_t cardType;
        uint64_t cardSize;
        SPIClass spi;
        File cur_file;
        String cur_filename;
        uint32_t cur_indexinfile=0;
        ESP32Time *cur_time;
    public:
    Logg(){spi=SPIClass(HSPI); };
    ~Logg(){};
    void begin(ESP32Time *rtc);
    void print(const char *format,...);
};
#endif