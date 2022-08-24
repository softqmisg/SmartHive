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
#define CAP_OFFSET  11.66
#define RES_555     1e6
#define CAP_LEVEL_EMPTY 20 //pf
#define CAP_LEVEL_HALF 40 //pf
#define CAP_LEVEL_FULL 80 //pf
enum _diurnal_t {DAY,NIGHT};
enum  _state_t {NORMAL,EMERGENCY};

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
    bool hiveWeightState=false;
    uint8_t fan_speed=0;
    float fan_current;
    bool pump_status=false;
    _diurnal_t diurnal_status=DAY;
    _state_t  hive_state=NORMAL;
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
    bool getConnectionState(){return deviceConnected;};
    void setConnectionState(bool state){deviceConnected=state;};
    int readPhotoCell(){return analogRead(ESP32_GPIO39_PHOTOCELL);};
    bool readDoorState(){return doorState;};
    bool getHiveWeightState(){ return hiveWeightState;};
    float readWeight(){ return 25.0;};
    int getHiveFeedingLevel(){
        FreqCountESP.start();
        while(!FreqCountESP.available());
        FreqCountESP.stop();
        uint32_t freq_r = FreqCountESP.read();
        double cap = 1.44 / (3 * freq_r) * 1e6 - CAP_OFFSET;  //pico farad      
        if(cap<=CAP_LEVEL_EMPTY)
            return 0;
        else if(cap<=CAP_LEVEL_HALF)
            return 1;
        else
            return 2;
    };
    float getHeaterAverageCurrent(){ return 0.568;};
    bool getFanStatus(){
        ledcWrite(FANPWMCHANNEL,255-127);
        delay(100);
        fan_current=analogRead(ESP32_GPIO18_FAN)*33.0/1024.0;
        ledcWrite(FANPWMCHANNEL, 255-fan_speed);
        if(fan_current>0.03)
            return true;
        else
            return false;
    };
    bool getPumpStatus(){ return pump_status;};
    _diurnal_t getDiurnalStatus(){return diurnal_status;};
    _state_t getHiveState(){return hive_state;}
};
extern volatile bool keypress_f;
extern portMUX_TYPE keyMux;
extern Hive hive;

#endif