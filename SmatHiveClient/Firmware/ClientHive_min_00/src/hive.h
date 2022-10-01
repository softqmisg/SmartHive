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
#include  "HX711.h"

#define DS1307_NVRAM_CHECK_BAT 2
#define HEATERPWMCHANNEL   0
#define FANPWMCHANNEL   1
#define LEDPWMCHANNEL   2
#define CAP_OFFSET  11.66
#define RES_555     1e6
#define CAP_LEVEL_EMPTY 20 //pf
#define CAP_LEVEL_HALF 40 //pf
#define CAP_LEVEL_FULL 80 //pf

enum  _hivestate_t {HIVEAUTO,HIVESTERIL,HIVEOFF};
enum _mode_t {MODEFAIL,MODEOK};
enum _sensorstate_t {DISCONNECT=0,CONNECTOK,CONNECTFAIL};
enum _fanmode_t {FANAUTO,FANOFF,FANON};
enum _heatermode_t {HEATERAUTO,HEATEROFF,HEATERON};
typedef struct {
  double kp;
  double ki;
  double kd;
} PIDCoeff_t;


typedef struct {
    double temperature;
    TimeSpan duration;

}   STERILProg_t;

typedef struct {
    double temperature;
    double fanhumidity;
    double fantemperature;
    double fanhysthumidity;
    double fanhysttemperature;
}   AUTOProg_t;

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
    bool hiveWeightState=false;
    uint8_t fan_percent=0;
    uint8_t heater_percent=0;
    float fan_current;
    float heater_current;
    float avg_heater_current;
    float sum_heater_current;
    uint16_t avg_heater_cnt=0;
    _hivestate_t  hive_state=HIVEAUTO;
    _fanmode_t fan_mode=FANOFF;
    _heatermode_t heater_mode=HEATEROFF;
    String bleServerName;
    Position hiveposition;
    long bleStaticPIN;
    EEClass ee;
    PIDCoeff_t pidCoeff;
    AUTOProg_t  auto_prog;
    STERILProg_t steril_prog;
    _sensorstate_t sht20inside_state;
    _sensorstate_t sht20outside_state;
    uint32_t tik_counter=0;
    long timestamp;
    uint32_t readADC_Cal(uint8_t ch);
    void setFan(uint8_t percent) {ledcWrite(FANPWMCHANNEL, 256 - percent); fan_percent=percent;};
    uint8_t getFan() {return fan_percent;};
    void setHeater(uint8_t percent) {ledcWrite(HEATERPWMCHANNEL, 256 - percent); heater_percent=percent;};
    uint8_t getHeater(){return heater_percent;};
    void loaddefaultvalues();
    void controlheater(double targettemp);
    void controlfan(void);
    void controlhiveauto();
    void controlhivesteril();
    void controlhiveoff();

public:
    SHT2x sht20Inside;
    SHT2x sht20Outside;
    RTC_DS1307 rtcext;
    PID heater;
    Logg logg;
    ESP32Time rtcint;
    HX711 scale;
    Hive() : 
        heater_current(0.0),
        avg_heater_current(0.0),
        sum_heater_current(0.0),
        bleServerName(DEFAULT_NAME),
        hiveposition(DEFAULT_LAT,DEFAULT_LNG),
        bleStaticPIN(DEFAULT_BLEPASS),
        heater(&heaterInput, &heaterOutput, &heaterSetpoint, pidCoeff.kp,pidCoeff.ki,pidCoeff.kd, DIRECT),
        rtcint(0)
        {};
    ~Hive(){};
    void begin(void);
    void update();

    _mode_t checkHeater();
    _mode_t checkFan();
    _sensorstate_t checkSensorInside();
    _sensorstate_t checkSensorOutside();
    _sensorstate_t checkSensorScale();

    void setHiveState(_hivestate_t state);
    _hivestate_t getHiveState(){return hive_state;};
    void setFanMode(_fanmode_t mode);
    _fanmode_t getFanMode () {return fan_mode;};
    void setHeaterMode(_heatermode_t mode);
    _heatermode_t getHeaterMode () {return heater_mode;}    
    void setSterilProg(STERILProg_t prog,bool save);
    void setSterilProg(double temp,TimeSpan duration,bool save);
    STERILProg_t getSterilProg(){return steril_prog;};
    void setAutoProg(AUTOProg_t prog,bool save);
    AUTOProg_t getAutoProg(){return auto_prog;};
    void setPID(double kp,double ki,double kd,bool save);
    void setPID(PIDCoeff_t pid,bool save);
    PIDCoeff_t getPID(){return pidCoeff;};
    void setblepassword(long pass,bool save);
    long getblepassword(){return bleStaticPIN;}
    void setPosition(position_t pos,bool save) { setPosition(pos.latitude,pos.longitude,save);};
    void setPosition(double lat,double lng,bool save);
    position_t getPosition() {return hiveposition.getPosition();};
    DateTime getSunrise() {return hiveposition.getSunrise();};
    DateTime getSunset() {return hiveposition.getSunset();};
    DateTime getNoon() {return hiveposition.getNoon();};
    void setDateTime(DateTime t);
    DateTime getDateTime(){return  DateTime(rtcint.getEpoch());};
    String gethiveName(){return bleServerName;};
    void sethiveName(String name,bool save);
    float getInsideTemperature(){ return sht20Inside.getTemperature();};
    float getInsideHumidity(){ return sht20Inside.getHumidity();};
    float getOutsideTemperature(){ return sht20Outside.getTemperature();};
    float getOutsideHumidity(){ return sht20Outside.getHumidity();};
    bool getRTCStatus(){return rtcneedAdjust;};
    bool isDeviceConnected(){return deviceConnected;};
    bool getConnectionState(){return deviceConnected;};
    void setConnectionState(bool state){deviceConnected=state;};
    float getHeaterCurrent(){return heater_current;}
    float getHeaterAverageCurrent();
    bool getFanStatus(){return (fan_percent>0?true:false);};
    float getHiveWeight();
};
extern volatile bool keypress_f;
extern portMUX_TYPE keyMux;
extern Hive hive;

#endif