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
#include "position.h"
#include "EEPROM.h"
#include "log.h"
#include "esp_adc_cal.h"


#define DS1307_NVRAM_CHECK_BAT 2

#define HEATERPWMCHANNEL   0
#define FANPWMCHANNEL   2
#define LEDPWMCHANNEL   4

enum  _state_t {HIVEAUTO,HIVESTERIL,HIVEMANUAL};
enum _mode_t {MODEFAIL,MODEOK};
enum _sensorstate_t {DISCONNECT=0,CONNECTOK,CONNECTFAIL};
typedef struct {
  double kp;
  double ki;
  double kd;
} PID_Coeff_t;

typedef struct {
    bool active;
    uint8_t id;
    double TemperatureTarget;
    double fanOnTemperature;
    double fanTHysteresis;
    double fanOnHumidity;
    double fanHHysteresis;
}   PROG_t;

class EEClass:public EEPROMClass{
    public:
    EEClass():EEPROMClass(){};
    template<typename T>
    T &get(int address, T &t) {
      if (address < 0 || address + sizeof(T) > _size)
        return t;

      memcpy((uint8_t*) &t, _data + address, sizeof(T));
      return t;
    };

    template<typename T>
    const T &put(int address, const T &t) {
      if (address < 0 || address + sizeof(T) > _size)
        return t;

      memcpy(_data + address, (const uint8_t*) &t, sizeof(T));
      _dirty = true;
      return t;
    };
    template <class T> T writeAll (int address, const T &value)
    {
        if (address < 0 || address + sizeof(T) > _size)
            return value;

        memcpy(_data + address, (const uint8_t*) &value, sizeof(T));
        _dirty = true;

        return sizeof (value);
    }  ;  
    template <class T> T readAll (int address, T &value)
    {
        if (address < 0 || address + sizeof(T) > _size)
            return value;

        memcpy((uint8_t*) &value, _data + address, sizeof(T));
        return value;
    };    

};
class Hive
{

private:
    /* data */
    static Hive* sHive;
    char swTxBuffer[16];
    char swRxBuffer[16];
    bool rtcneedAdjust = false;
    double heaterSetpoint=37.0, heaterInput, heaterOutput;
    bool deviceConnected=false;
    bool    doorState;
    uint8_t fan_percent=0;
    uint8_t heater_percent=0;
    float fan_current;
    float heater_current;
    float avg_heater_current;
    float sum_heater_current;
    uint16_t avg_heater_cnt=0;
    _state_t  hive_state=HIVEAUTO;
    String bleServerName;
    Position hiveposition;
    long bleStaticPIN;
    EEClass ee;
    PID_Coeff_t pidCoeff;
    PROG_t autoprog;
    PROG_t sterilprog;    
    _sensorstate_t sht20inside_state;
    _sensorstate_t sht20outside_state;
    uint32_t tik_counter=0;
    uint32_t readADC_Cal(uint8_t ch);
    int led_brightness = 0;
    int led_direction=1;  
    bool rtcextrunning=false;
public:
    SHT2x sht20Inside;
    SHT2x sht20Outside;
    RTC_DS1307 rtcext;
    PID heater;
    Logg logg;
    ESP32Time rtcint;
    
    Hive() : 
        heater_current(0.0),
        avg_heater_current(0.0),
        sum_heater_current(0.0),
        bleServerName(DEFAULT_NAME),
        hiveposition(DEFAULT_LAT,DEFAULT_LNG),
        bleStaticPIN(DEFAULT_BLEPASS),
        pidCoeff{DEFAULT_KPCOEFF,DEFAULT_KICOEFF,DEFAULT_KDCOEFF},
        heater(&heaterInput, &heaterOutput, &heaterSetpoint, DEFAULT_KPCOEFF,DEFAULT_KICOEFF,DEFAULT_KDCOEFF, DIRECT),
        rtcint(0)
        {};
    ~Hive(){};
    void begin(void);
    void update();
    void loaddefaultvalues(uint8_t load);

    void setFan(uint8_t percent) {ledcWrite(FANPWMCHANNEL, 256 - percent); fan_percent=percent;};
    uint8_t getFan() {return fan_percent;};
    void setHeater(uint8_t percent) {ledcWrite(HEATERPWMCHANNEL,256-percent); heater_percent=percent;};
    uint8_t getHeater(){return heater_percent;};
    void setHiveState(_state_t state){
        hive_state=state;
    };
    _state_t getHiveState(){return hive_state;};
    _mode_t checkHeater();
    _mode_t checkFan();
    _mode_t checkPump();
    _sensorstate_t checkSensorInside();
    _sensorstate_t checkSensorOutside();
    void setAutoProg(PROG_t prog,bool save);
    PROG_t getAutoProg(void){return autoprog;};    
    void setSterilProg(PROG_t prog,bool save);
    PROG_t getSterilProg(void){return sterilprog;};

    void setPID(double kp,double ki,double kd,bool save);
    PID_Coeff_t getPID(){return pidCoeff;};
    void setblepassword(long pass,bool save);
    long getblepassword(){return bleStaticPIN;}
    void setPosition(position_t pos,bool save) { setPosition(pos.latitude,pos.longitude,save);};
    void setPosition(double lat,double lng,bool save);
    position_t getPosition() {return hiveposition.getPosition();};
    DateTime getSunrise() {return hiveposition.getSunrise();};
    DateTime getSunset() {return hiveposition.getSunset();};
    DateTime getNoon() {return hiveposition.getNoon();};
    void setDateTime(DateTime t){
            rtcext.adjust(t);
            rtcint.setTime(rtcext.now().unixtime());        
        };
    DateTime getDateTime(){
        return rtcext.now();
    }
    String gethiveName(){return bleServerName;};
    void sethiveName(String name,bool save);
    void setTargetTemperature(double temp){ heaterSetpoint=temp;};
    double getTargetTemperature(void){ return heaterSetpoint;};
    float getInsideTemperature(){ return sht20Inside.getTemperature();};
    float getInsideHumidity(){ return sht20Inside.getHumidity();};
    float getOutsideTemperature(){ return sht20Outside.getTemperature();};
    float getOutsideHumidity(){ return sht20Outside.getHumidity();};
    void controlHeater(PROG_t prog);
    bool getRTCStatus(){return rtcneedAdjust;};
    bool isDeviceConnected(){return deviceConnected;};
    bool getConnectionState(){return deviceConnected;};
    void setConnectionState(bool state){deviceConnected=state;};
    int readPhotoCell(){return 0; /*analogRead(ESP32_GPIO39_PHOTOCELL);*/};
    bool readDoorState(){return doorState;};
    float getHeaterCurrent(){return heater_current;};
    float getHeaterAverageCurrent();
    bool getFanStatus(){return (fan_current>0?true:false);};
};
extern Hive hive;

#endif