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
#include "FreqCountESP.h"
#include "position.h"
#include "EEPROM.h"
#include "log.h"
#include "esp_adc_cal.h"


#define DS1307_NVRAM_CHECK_BAT 2
#define HEATERPWMCHANNEL   0
#define FANPWMCHANNEL   1
#define LEDPWMCHANNEL   2
#define CAP_OFFSET  11.66
#define RES_555     1e6
#define CAP_LEVEL_EMPTY 20 //pf
#define CAP_LEVEL_HALF 40 //pf
#define CAP_LEVEL_FULL 80 //pf

enum _diurnal_t {DAY,NIGHT};
enum  _state_t {HIVEAUTO,HIVEEMERGENCY,HIVESTERIL,HIVEOFF};
enum _mode_t {MODEFAIL,MODEOK};
enum _feeding_t {EMPETY=0,HALF,FULL};
enum _pumpcontrol_t {TIME=0,LEVEL=1};
enum _sensorstate_t {DISCONNECT=0,CONNECTOK,CONNECTFAIL};
typedef struct {
  double kp;
  double ki;
  double kd;
} PID_Coeff_t;

typedef struct {
    uint16_t level;
    uint16_t hysteresis;

} LIGHT_Level_t;

typedef struct {
    bool active;
    uint8_t id;
    DateTime start;
    DateTime stop;
    TimeSpan  Sunrise;
    TimeSpan Sunset;
    double minNormalTemperatureDay;
    double minNormalTemperatureNight;
    double dayTemperatureTarget;
    double nightTemperatureTarget;
    double emergencyTemperatureTarget;
    double fanOnTemperature;
    double fanOnHumidity;
    double fanOnHysteresis;
    bool pumpOnDay;
    bool pumpOnNight;
    bool fanOnDay;
    bool fanOnNight;
    bool heaterOnDay;
    bool heaterOnNight;
}   PROG_t;

typedef struct{
_feeding_t turnonlevel;
_pumpcontrol_t controlmode;
TimeSpan turnontime;
} PUMP_t;

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
    static void onReedrelayISR();
    void setDoorState(int value){doorState=(value)?true:false;};
    bool hiveWeightState=false;
    uint8_t fan_percent=0;
    uint8_t heater_percent=0;
    float fan_current;
    float heater_current;
    float avg_heater_current;
    float sum_heater_current;
    uint16_t avg_heater_cnt=0;
    _feeding_t feeding_level=EMPETY;
    _diurnal_t diurnal_status=DAY;
    _state_t  hive_state=HIVEAUTO;
    String bleServerName;
    Position hiveposition;
    long bleStaticPIN;
    EEClass ee;
    PID_Coeff_t pidCoeff;
    LIGHT_Level_t lightlevelNight;
    PROG_t progs[HIVE_MAX_PROG];
    bool pump_status;
    PUMP_t pump;
    _sensorstate_t sht20inside_state;
    _sensorstate_t sht20outside_state;
    uint32_t tik_counter=0;
    uint32_t readADC_Cal(uint8_t ch);
public:
    SHT2x sht20Inside;
    SHT2x sht20Outside;
    RTC_DS1307 rtcext;
    PID heater;
    _FreqCountESP FreqCountESP;
    Logg logg;
    ESP32Time rtcint;
    
    enum {StateDay,StateNight} hiveDiurnalState;
    enum {EmergencyState,NormalState} hiveOperationalState;
    Hive() : 
        heater_current(0.0),
        avg_heater_current(0.0),
        sum_heater_current(0.0),
        bleServerName(DEFAULT_NAME),
        hiveposition(DEFAULT_LAT,DEFAULT_LNG),
        bleStaticPIN(DEFAULT_BLEPASS),
        pidCoeff{DEFAULT_KPCOEFF,DEFAULT_KICOEFF,DEFAULT_KDCOEFF},
        lightlevelNight{DEFAULT_LIGHT_NIGHT,DEFAULT_LIGHT_HSYTERESIS},
        pump_status(false),
        heater(&heaterInput, &heaterOutput, &heaterSetpoint, DEFAULT_KPCOEFF,DEFAULT_KICOEFF,DEFAULT_KDCOEFF, DIRECT),
        rtcint(0)
        {};
    ~Hive(){};
    void begin(void);
    void update();
    void loaddefaultvalues();

    void setFan(uint8_t percent) {ledcWrite(FANPWMCHANNEL, 256 - percent); fan_percent=percent;};
    uint8_t getFan() {return fan_percent;};
    void setHeater(uint8_t percent) {ledcWrite(HEATERPWMCHANNEL, 256 - percent); heater_percent=percent;};
    uint8_t getHeater(){return heater_percent;};
    void setPumpStatus(bool state){ 
        pump_status=state;
        if(state)
        digitalWrite(ESP32_GPIO4_PUMP,LOW);
        else
        digitalWrite(ESP32_GPIO4_PUMP,HIGH);
    }
    void setHiveState(_state_t state){
        hive_state=state;
    };
    _state_t getHiveState(){return hive_state;};
    _mode_t checkHeater();
    _mode_t checkFan();
    _mode_t checkPump();
    _sensorstate_t checkSensorInside();
    _sensorstate_t checkSensorOutside();
    void setPump(PUMP_t pump,bool save);
    PUMP_t getPump(){return pump;};
    void setProg(PROG_t prog,bool save);
    PROG_t getProg(uint8_t num){return progs[num];};
    void setLight(uint16_t levelNight, uint16_t levelHysteresis,bool save);
    LIGHT_Level_t getLight(){return (lightlevelNight);}
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
    void setInsideTemperature(double temp){ heaterSetpoint=temp;};
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
    float readWeight(){ return 0.0;};
    _feeding_t getHiveFeedingLevel(){return feeding_level;};
    float getHeaterCurrent(){return heater_current;}
    float getHeaterAverageCurrent();
    bool getFanStatus(){return (fan_percent>0?true:false);};
    bool getPumpStatus(){ return (bool)pump_status;};
    _diurnal_t getDiurnalStatus(){return diurnal_status;};
};
extern volatile bool keypress_f;
extern portMUX_TYPE keyMux;
extern Hive hive;

#endif