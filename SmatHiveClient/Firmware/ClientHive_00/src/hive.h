#ifndef __HIVE_H__
#define __HIVE_H__
#include <Arduino.h>
#include "main.h"
#include <PID_v1.h>
#include <Wire.h>
#include <SoftWire.h>
#include <SHT2x.h>
#include "RTClib_Tiny.h"
#include "ESP32Time.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "FreqCountESP.h"

#define DS1307_NVRAM_CHECK_BAT 1
#define SDCARDSPEED 5000000
#define HEATERCHANNEL   0
class Hive
{

private:
    /* data */

    char swTxBuffer[16];
    char swRxBuffer[16];
    bool need_rtcadjust = false;
    ESP32Time rtcint;
    double heaterSetpoint=37.0, heaterInput, heaterOutput;


public:
    SHT2x sht20Inside;
    SHT2x sht20Outside;
    RTC_DS1307 rtcext;
    PID heater;
    _FreqCountESP FreqCountESP;

    Hive(double _kp = 2, double _ki = 5, double _kd = 1) : rtcint(0),
                       heater(&heaterInput, &heaterOutput, &heaterSetpoint, _kp, _ki, _kd, DIRECT){};
    ~Hive(){};
    void begin(void);
    float getInsideTemperature(){ return sht20Inside.getTemperature();};
    float getInsideHumidity(){ return sht20Inside.getHumidity();};
    float getOutsideTemperature(){ return sht20Outside.getTemperature();};
    float getOutsideHumidity(){ return sht20Outside.getHumidity();};
    void controlHeater(void);

};

#endif