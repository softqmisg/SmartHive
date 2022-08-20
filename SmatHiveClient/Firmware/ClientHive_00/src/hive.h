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

#define DS1307_NVRAM_CHECK_BAT 2
#define SDCARDSPEED 5000000
#define HEATERPWMCHANNEL   0
#define FANPWMCHANNEL   1
#define LEDPWMCHANNEL   2

class Hive
{

private:
    /* data */
    static Hive* sHive;
    char swTxBuffer[16];
    char swRxBuffer[16];
    bool rtcneedAdjust = false;
    ESP32Time rtcint;
    double heaterSetpoint=37.0, heaterInput, heaterOutput;
    bool deviceConnected=false;
    bool    doorState;
    static void onReedrelayISR();
    void setDoorState(int value){doorState=(value)?true:false;};

public:
    SHT2x sht20Inside;
    SHT2x sht20Outside;
    RTC_DS1307 rtcext;
    PID heater;
    _FreqCountESP FreqCountESP;

    enum {StateDay,StateNight} hiveDiurnalState;
    enum {EmergencyState,NormalState} hiveOperationalState;
    Hive(double _kp = 2, double _ki = 5, double _kd = 1) : rtcint(0),
                       heater(&heaterInput, &heaterOutput, &heaterSetpoint, _kp, _ki, _kd, DIRECT){};
    ~Hive(){};
    void begin(void);
    float getInsideTemperature(){ return sht20Inside.getTemperature();};
    float getInsideHumidity(){ return sht20Inside.getHumidity();};
    float getOutsideTemperature(){ return sht20Outside.getTemperature();};
    float getOutsideHumidity(){ return sht20Outside.getHumidity();};
    void controlHeater(void);
    bool getRTCStatus(){return rtcneedAdjust;};
    bool isDeviceConnected(){return deviceConnected;};
    void setConnectionState(bool state){deviceConnected=state;};
    int readPhotoCell(){return analogRead(ESP32_GPIO39_PHOTOCELL);};
    bool readDoorState(){return doorState;};
};
extern volatile bool keypress_f;
extern portMUX_TYPE keyMux;
extern Hive hive;

#endif