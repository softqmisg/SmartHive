#include "hive.h"
#include "UserBLE.h"
// #include "buildTime.h"

TwoWire I2C_1(0);
TwoWire I2C_2(1);
SoftWire I2C_3(ESP32_GPIO27_RTC_SDA, ESP32_GPIO32_RTC_SCL);

DateTime BUILD_DATETIME(__DATE__,__TIME__);

const AUTOProg_t DEFAULT_AUTOPROG={
    .temperature=10.0,
    .fanhumidity=60.0,
    .fantemperature=40,
    .fanhysthumidity=2.0,
    .fanhysttemperature=1.0,
};
const STERILProg_t DEFAULT_STERILPROG={
    .temperature=40,
    .duration=TimeSpan(0,0,10,0),
};
const PIDCoeff_t DEFAULT_PID={
    .kp=2.0,
    .ki=5.0,
    .kd=1.0,
};
/**
 * @brief Key press detction interrupt
 *
 */
unsigned long button_time = 0;
unsigned long last_button_time = 0;
volatile bool keypress_f = false;
portMUX_TYPE keyMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onKeyPress(void)
{
    button_time = millis();
    if (button_time - last_button_time > 300)
    {
        portENTER_CRITICAL_ISR(&keyMux);
        keypress_f = true;
        portEXIT_CRITICAL_ISR(&keyMux);
    }
    last_button_time = button_time;
}
/**
 * @brief readadc calib
 * 
 */
uint32_t Hive::readADC_Cal(uint8_t ch)
{
  esp_adc_cal_characteristics_t adc_chars;
  int ADC_Raw=analogRead(ch);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  return(esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}
/**
 * @brief load default values from EEPROM
 *
 */
void Hive::loaddefaultvalues(void)
{
    if (((uint8_t)ee.readByte(EE_ADD_FIRSTTIME) != 0xaa)|| (digitalRead(ESP32_GPIO15_KEYDOWN)==0))
    { // save paramters in EEPROM
        Serial.println("Writing for first time in EEPROM");
        ee.writeByte(EE_ADD_FIRSTTIME,0xaa);
        ee.writeDouble(EE_ADD_LAT,DEFAULT_LAT);
        ee.writeDouble(EE_ADD_LNG,DEFAULT_LNG);
        ee.writeString(EE_ADD_HIVE_NAME,DEFAULT_NAME);
        ee.writeLong(EE_ADD_BLEPASS,DEFAULT_BLEPASS);
        ee.put(EE_ADD_PIDCOEFF,DEFAULT_PID);
        ee.put(EE_ADD_AUTOPROG,DEFAULT_AUTOPROG);
        ee.put(EE_ADD_STERILPROG,DEFAULT_STERILPROG);
        ee.commit();
    }
    Serial.println("reading from  EEPROM");
    setPosition(ee.readDouble(EE_ADD_LAT),ee.readDouble(EE_ADD_LNG),false);
    sethiveName(ee.readString(EE_ADD_HIVE_NAME),false);
    setblepassword(ee.readLong(EE_ADD_BLEPASS),false);
    setPID(ee.readAll(EE_ADD_PIDCOEFF,DEFAULT_PID),false);
    setAutoProg(ee.readAll(EE_ADD_AUTOPROG,DEFAULT_AUTOPROG),false);
    setSterilProg(ee.readAll(EE_ADD_STERILPROG,DEFAULT_STERILPROG),false);
}
/**
 * @brief Hive class start
 *
 */
Hive *Hive::sHive = 0;
void Hive::begin(void)
{
    sHive = this;
    if (ee.begin(EEPROM_SZ))
    {
        Serial.println("EEPROM OK");
    }
    else
    {
        Serial.println("EEPROM FAIL");
    }
    loaddefaultvalues();
    // Set IO mods
    ledcSetup(HEATERPWMCHANNEL, 1000, 8);
    ledcAttachPin(ESP32_GPIO19_THERMAL, HEATERPWMCHANNEL); // pwm
    ledcSetup(FANPWMCHANNEL, 5000, 8);
    ledcAttachPin(ESP32_GPIO18_FAN, FANPWMCHANNEL); // pwm
    ledcSetup(LEDPWMCHANNEL, 1000, 8);
    ledcAttachPin(ESP32_GPIO5_LED0, LEDPWMCHANNEL);            // pwm: freq=80M/2^bits
    pinMode(ESP32_GPIO4_PUMP, OUTPUT);             // gpio
    // pinMode(ESP32_GPIO39_PHOTOCELL, ANALOG);       // analog input
    // pinMode(ESP32_GPIO33_FANFEEDBACK, ANALOG);     // analog input
    // pinMode(ESP32_GPIO36_THERMALFEEDBACK, ANALOG); // analog input
    pinMode(ESP32_GPIO23_REED, INPUT);             // intterupt input
    pinMode(ESP32_GPIO15_KEYDOWN, INPUT); // interrupt input
    attachInterrupt(digitalPinToInterrupt(ESP32_GPIO15_KEYDOWN), onKeyPress, FALLING);

    ///init  i2c & sht20
    I2C_1.begin(ESP32_GPIO21_SDA1, ESP32_GPIO22_SCL1);
    I2C_1.setTimeOut(1000);
    I2C_2.begin(ESP32_GPIO16_SDA2, ESP32_GPIO17_SCL2);
    I2C_2.setTimeOut(1000);
    sht20Inside.begin(&I2C_1);
    sht20Outside.begin(&I2C_2);
    //init scale
    scale.begin(ESP32_GPIO35_HX711_DT, ESP32_GPIO34_HX711_CLK);  
    scale.power_down();  
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
        rtcext.adjust(BUILD_DATETIME);
        rtcneedAdjust = true;
        Serial.println("Write new nvram ds1307 ");
        Serial.println(String(BUILD_DATETIME.year())+"-"+
                        String(BUILD_DATETIME.month())+"-"+
                        String(BUILD_DATETIME.day())+","+
                        String(BUILD_DATETIME.hour())+":"+
                        String(BUILD_DATETIME.minute())+":"+
                        String(BUILD_DATETIME.second()));
        DateTime tt=rtcext.now();
        Serial.println(tt.year());
        Serial.println(tt.month());
        Serial.println(tt.day());
        Serial.println(tt.hour());
        Serial.println(tt.minute());
        Serial.println(tt.second());

    }
    else
    {
        rtcneedAdjust = false;
        Serial.println("Read nvram ds1307 was ok");
    }
    // /////////////////////////////////
    if(rtcext.isrunning())
        rtcint.setTime(rtcext.now().unixtime());
    else
        rtcint.setTime(0);
    ////////////////////////////////
    logg.begin(&rtcint);
    ///////////////set PID/////////////
    heater.SetSampleTime(1000);
    heater.SetMode(AUTOMATIC);
    /////////////////////////////
    setFanMode(FANOFF);
    setHeaterMode(HEATEROFF);
}
void Hive::update(void)
{
    //read adcs
    avg_heater_cnt++;
    heater_current=(float)readADC_Cal(ESP32_GPIO36_THERMALFEEDBACK)*10.0;
    sum_heater_current+=heater_current;
    if(avg_heater_cnt==3600)
    {
        avg_heater_current=(float)sum_heater_current/3600.0;
        sum_heater_current=0;
        avg_heater_cnt=0;
    }

    fan_current=readADC_Cal(ESP32_GPIO33_FANFEEDBACK)*10.0;
    //read sht20s
    if(sht20Inside.isConnected())
    {
        
        sht20Inside.read();
        if(isDeviceConnected())
        {
            bleNotify(READONLY_InsideTemperature);
            bleNotify(READONLY_InsideHumidity);
        }
        
    }
    if(sht20Outside.isConnected())
    {
        sht20Outside.read();
        if(isDeviceConnected())
        {
            bleNotify(READONLY_OutsideTemperature);
            bleNotify(READONLY_OutsideHumidity);
        }
    }
    //read hx711
    if(scale.is_ready())
    {
        bleNotify(READONLY_HiveWeight);
    }
    ///check time ,sun and day

    DateTime now=rtcext.now();
    hiveposition.calculateSun(now);
    if(rtcext.now().second()==0 && isDeviceConnected())
    {
       bleNotify(READONLY_Sun);
    }
    //check status
    //run control algorithm
    if(hive_state==HIVEAUTO)
    {
        controlhiveauto();
    }
    else if(hive_state==HIVESTERIL)
    {
        controlhivesteril();
    }
    else
    {
        controlhiveoff();
    }

    ////logging datas
    tik_counter++;
    if(tik_counter>=1*20)
    {
        tik_counter=0;
        String format=rtcint.getTime("%F") +"\n"+
                    rtcint.getTime("%T") +"\n"+        
                      "h cur:"+String(getHeaterCurrent())+"\n"+
                      "h avg cur:"+String(getHeaterAverageCurrent())+"\n"+
                      " h percent:"+String(getHeater())+"\n"+
                      " pid output:"+String(heaterOutput)+"\n"+
                      "pid setpoint:"+String(heaterSetpoint)+"\n"+
                      "in temp:"+String(getInsideTemperature())+"\n"+
                      "in hum:"+String(getInsideHumidity())+"\n"+
                      "fan cur:"+String(fan_current);
        Serial.println(format);
        Serial.println("================================");
        format=rtcint.getTime("%F")+","+
               rtcint.getTime("%T")+","+
                      String(getHeaterCurrent())+","+
                      String(getHeaterAverageCurrent())+","+
                      String(getHeater())+","+
                      String(heaterOutput)+","+
                      String(heaterSetpoint)+","+
                      String(getInsideTemperature())+","+
                      String(getInsideHumidity())+","+
                      String(fan_current);        
        logg.print(format.c_str());
    }
}
float Hive::getHeaterAverageCurrent(){ 
    if(avg_heater_cnt>0)
    {
        return (sum_heater_current/avg_heater_cnt*3600.0+avg_heater_current)/(3601.0);
    }
    else
    {
        return avg_heater_current;
    }
};
void Hive::setHiveState(_hivestate_t state){
    hive_state=state;
    if(state==HIVEAUTO)
        timestamp=rtcint.getEpoch();
};
void Hive::setFanMode(_fanmode_t mode)
{
    fan_mode=mode;
    if(mode==FANAUTO)
        ;
    else if(mode==FANOFF)
        setFan(0);
    else 
        setFan(255);

    bleNotify(READWRITE_FanMode+100);
}
void Hive::setHeaterMode(_heatermode_t mode)
{
    if(mode==HEATERAUTO)
        ;
    else if(mode==HEATEROFF)
        setHeater(0);
    else 
        setHeater(255);    

    bleNotify(READWRITE_HeaterMode+100);
}
void Hive::controlhiveauto()
{
        controlfan();
        controlheater(auto_prog.temperature);
}
void Hive::controlhivesteril()
{
    if(rtcint.getEpoch()-timestamp<(long)steril_prog.duration.totalseconds())
    {
        controlheater(steril_prog.temperature);
    }
    else
    {
        setHiveState(HIVEAUTO);
        setHeaterMode(HEATERAUTO) ;
        setFanMode(FANAUTO) ;
    }
}
void Hive::controlhiveoff()
{
    setHeaterMode(HEATEROFF) ;
    setFanMode(FANOFF) ;
}
void Hive::controlfan(void)
{
    if(fan_mode==FANAUTO)
    {
        if(sht20Inside.isConnected())
        {        
            float temp_t=getInsideTemperature(),temp_h=getInsideHumidity();
            if(temp_h>auto_prog.fanhumidity || temp_t >auto_prog.fantemperature) 
            {
                setFan(255);
            }
            else if(temp_h< (auto_prog.fanhumidity-auto_prog.fanhysthumidity) &&
                    temp_t < (auto_prog.fantemperature-auto_prog.fanhysttemperature)
                    )
            {
                setFan(0);
            }
        }
        else
        {
            setFan(255);
        }
    }
}
void Hive::controlheater(double targettemp)
{
    if(heater_mode==HEATERAUTO)
    {
        if(sht20Inside.isConnected())
        {
            heaterInput = sht20Inside.getTemperature();
            heaterSetpoint=targettemp;
            heater.Compute();
            setHeater(heaterOutput);
        }
        else
        {
            setHeater(0);
        }
    }
}
void Hive::setPosition(double lat, double lng,bool save)
{
    if(save){
        ee.writeDouble(EE_ADD_LAT,lat);
        ee.writeDouble(EE_ADD_LNG,lng);
        ee.commit();
    }
    hiveposition.setPosition(lat, lng);
};
void Hive::sethiveName(String name,bool save){
    if(save){
        ee.writeString(EE_ADD_HIVE_NAME,name);
        ee.commit();        
    }
    bleServerName=name;
};
void Hive::setblepassword(long pass,bool save){
    if(save)
    {
        ee.writeLong(EE_ADD_BLEPASS,pass);
        ee.commit();
    }
    bleStaticPIN=pass;
}
void Hive::setPID(double kp,double ki,double kd,bool save)
{
    PIDCoeff_t pid=
    {
        .kp=kp,
        .ki=ki,
        .kd=kd
    };
     setPID(pid,save);
}
void Hive::setPID(PIDCoeff_t pid,bool save)
{
    if(save)
    {
        ee.put(EE_ADD_PIDCOEFF,pid);
        ee.commit();        
    }
    memcpy(&pidCoeff,&pid,sizeof(PID_COEFF_UUID));
    heater.SetTunings(pidCoeff.kp,pidCoeff.ki,pidCoeff.kd);    
}
void Hive::setAutoProg(AUTOProg_t prog,bool save)
{
    if(save)
    {
        ee.put(EE_ADD_AUTOPROG,prog);
        ee.commit();
    }
    memcpy(&auto_prog,&prog,sizeof(AUTOProg_t));
}
void Hive::setSterilProg(STERILProg_t prog,bool save)
{
    if(save)
    {
        ee.put(EE_ADD_STERILPROG,prog);
        ee.commit();        
    }
    memcpy(&steril_prog,&prog,sizeof(STERILProg_t));

}
void Hive::setSterilProg(double temp,TimeSpan duration,bool save)
{
    STERILProg_t sprog=
    {
        .temperature=temp,
        .duration=duration
    };
    setSterilProg(sprog,save);
}

_mode_t Hive::checkHeater(){
    uint8_t p=getHeater();
    setHeater(255);
    delay(100);
    heater_current=readADC_Cal(ESP32_GPIO36_THERMALFEEDBACK)*10.0;
    setHeater(p);
    if(heater_current>0.03)
        return MODEOK;
    else
        return MODEFAIL;

}
_mode_t Hive::checkFan()
{
    uint8_t s=getFan();
    setFan(255);
    delay(500);
    fan_current=readADC_Cal(ESP32_GPIO33_FANFEEDBACK)*10.0;
    setFan(s);
    if(fan_current>0.00)
        return MODEOK;
    else
        return MODEFAIL;
}
_sensorstate_t Hive::checkSensorInside(){
    if(sht20Inside.isConnected())
        return CONNECTOK;
    else
        return DISCONNECT;
}
_sensorstate_t Hive::checkSensorOutside(){
    if(sht20Outside.isConnected())
        return CONNECTOK;
    else
        return DISCONNECT;
};
_sensorstate_t Hive::checkSensorScale()
{
    if(scale.is_ready())
        return CONNECTOK;
    else
        return DISCONNECT;
}
float Hive::getHiveWeight()
{
    float w;
    scale.power_up();
    w=scale.get_units(5);
    scale.power_down(); 
    return w;
}
void Hive::setDateTime(DateTime t){
    if(rtcext.isrunning())
    {
        rtcext.adjust(t);
        rtcint.setTime(rtcext.now().unixtime());  
    }
    else
    {
        rtcint.setTime(t.unixtime());
    }      
}
