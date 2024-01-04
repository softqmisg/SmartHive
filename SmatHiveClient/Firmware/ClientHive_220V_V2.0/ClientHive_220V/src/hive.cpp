#include "hive.h"
#include "UserBLE.h"
#include "ACS712.h"
// #include "buildTime.h"

TwoWire I2C_1(0);
TwoWire I2C_2(1);
SoftWire I2C_3(ESP32_GPIO27_RTC_SDA, ESP32_GPIO32_RTC_SCL);
// ACS712  ACS(ESP32_GPIO35_THERMALFEEDBACK, 3.3, 1023, 5.0);


DateTime BUILD_DATETIME(__DATE__,__TIME__);

PROG_t DEFAULT_AUTOPROG={
    .active=true,
    .id=0,
    .TemperatureTarget=12.0,
    .fanOnTemperature=35.0,
    .fanTHysteresis=2.0,
    .fanOnHumidity=20.0,
    .fanHHysteresis=2.0,
    };
PROG_t DEFAULT_STERILPROG={
    .active=true,
    .id=0,
    .TemperatureTarget=40.0,
    .fanOnTemperature=45.0,
    .fanTHysteresis=2.0,
    .fanOnHumidity=100.0,
    .fanHHysteresis=2.0,
    };
/**
 * @brief readadc calib
 * 
 */
uint32_t Hive::readADC_Cal(uint8_t ch)
{
  esp_adc_cal_characteristics_t adc_chars;
  int ADC_Raw=analogRead(ch);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  return(esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}
/**
 * @brief load default values from EEPROM
 *
 */
void Hive::loaddefaultvalues(uint8_t load)
{
    if (((uint8_t)ee.readByte(EE_ADD_FIRSTTIME) != 0xaa) || (load==1))//|| (digitalRead(ESP32_GPIO15_KEYDOWN)==0)
    { // save paramters in EEPROM
        Serial.println("Writing for first time in EEPROM");
        ee.writeByte(EE_ADD_FIRSTTIME,0xaa);
        ee.writeDouble(EE_ADD_LAT,DEFAULT_LAT);
        ee.writeDouble(EE_ADD_LNG,DEFAULT_LNG);
        ee.writeString(EE_ADD_HIVE_NAME,DEFAULT_NAME);
        ee.writeLong(EE_ADD_BLEPASS,DEFAULT_BLEPASS);
        ee.writeDouble(EE_ADD_KPCOEFF,DEFAULT_KPCOEFF);
        ee.writeDouble(EE_ADD_KICOEFF,DEFAULT_KICOEFF);
        ee.writeDouble(EE_ADD_KDCOEFF,DEFAULT_KDCOEFF);
        ee.put(EE_ADD_AUTOPROGS_START,DEFAULT_AUTOPROG);
        ee.put(EE_ADD_STERILPROGS_START,DEFAULT_STERILPROG);
        ee.commit();
    }
    Serial.println("reading from  EEPROM");
    setPosition(ee.readDouble(EE_ADD_LAT),ee.readDouble(EE_ADD_LNG),false);
    sethiveName(ee.readString(EE_ADD_HIVE_NAME),false);
    setblepassword(ee.readLong(EE_ADD_BLEPASS),false);
    setPID(ee.readDouble(EE_ADD_KPCOEFF),
           ee.readDouble(EE_ADD_KICOEFF),
           ee.readDouble(EE_ADD_KDCOEFF),false
    );

    PROG_t tmp=ee.readAll(EE_ADD_AUTOPROGS_START,DEFAULT_AUTOPROG);
    setAutoProg(tmp,false);
    tmp=ee.readAll(EE_ADD_STERILPROGS_START,DEFAULT_STERILPROG);
    setSterilProg(tmp,false);

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
    loaddefaultvalues(0);
    // Set IO mods
    ledcSetup(FANPWMCHANNEL, 5000, 8);
    ledcAttachPin(ESP32_GPIO18_FAN, FANPWMCHANNEL); // pwm
    delay(20);
    ledcSetup(LEDPWMCHANNEL, 1000, 8);
    ledcAttachPin(ESP32_GPIO5_LED0, LEDPWMCHANNEL);            // pwm: freq=80M/2^bits
    delay(20);
    ledcSetup(HEATERPWMCHANNEL, 50, 8);
    ledcAttachPin(ESP32_GPIO19_THERMAL, HEATERPWMCHANNEL); // pwm
    delay(20);

    // ledcSetup(6, 100, 8);
    // ledcAttachPin(4, 6); // pwm
    // delay(20);    
    ///
    I2C_1.begin(ESP32_GPIO21_SDA1, ESP32_GPIO22_SCL1);
    I2C_1.setTimeOut(1000);
    I2C_2.begin(ESP32_GPIO16_SDA2, ESP32_GPIO17_SCL2);
    I2C_2.setTimeOut(1000);
    sht20Outside.begin(&I2C_1);
    sht20Inside.begin(&I2C_2);
    //current sensor
    //  ACS.autoMidPoint();
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
    rtcextrunning=false;
    if(rtcext.isrunning()) rtcextrunning=true;

    if (rtcext.readnvram(DS1307_NVRAM_CHECK_BAT) != 0xAA && rtcextrunning)
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
    if(rtcextrunning)
        rtcint.setTime(rtcext.now().unixtime());
    else
        rtcint.setTime(BUILD_DATETIME.unixtime());
    ////////////////////////////////
    logg.begin(&rtcint);
    ////////////////////////////
    heaterInput = sht20Inside.getTemperature();
    setTargetTemperature(autoprog.TemperatureTarget);
    heater.SetSampleTime(1000);
    heater.SetMode(AUTOMATIC);
    // heater.SetOutputLimits(0,1024);
    setHiveState(HIVEAUTO);
    /////////////////////////////
    setFan(0);
    setHeater(0);
    ledcWrite(LEDPWMCHANNEL,led_brightness); // max duty=2^bits  1000hz
    // ledcWrite(FANPWMCHANNEL, 128+64); // max duty=2^bits 100hz
    // ledcWrite(HEATERPWMCHANNEL,64); // max duty=2^bits 50hz
    // ledcWrite(6,128); // max duty=2^bits    5000hz


    // while(1);
}
void Hive::update(void)
{

    led_brightness = led_brightness+10*led_direction;
    ledcWrite(LEDPWMCHANNEL, led_brightness);       // max duty=2^bits
    if(led_brightness>=255)
    {
        led_direction*=-1;
    }     
    // Heater Current update,Average every 1Hr
    avg_heater_cnt++;
    heater_current= 0;//(float)ACS.mA_AC();
    bleNotify(READONLY_HeaterCurrent);    
    sum_heater_current+=heater_current;
    if(avg_heater_cnt==3600)
    {
        avg_heater_current=(float)sum_heater_current/3600.0;
        sum_heater_current=0;
        avg_heater_cnt=0;
        bleNotify(READONLY_HeaterAverageCurrent);
    }

    //fan current mesurment
    fan_current=(float)readADC_Cal(ESP32_GPIO39_FANFEEDBACK)/4.7;
    ///check time ,sun and day
    DateTime now(rtcint.getYear(),rtcint.getMonth(),rtcint.getDay(),
                 rtcint.getHour(),rtcint.getMinute(),rtcint.getSecond());
    hiveposition.calculateSun(now);
    if(rtcint.getSecond()==0)
    {
       bleNotify(READONLY_Sun);
    }
    //check status
    //run control algorithm
    //read sht20s

    if(sht20Outside.isConnected())
    {
        sht20Outside.read();
    }
    bleNotify(READONLY_OutsideTemperature);
    bleNotify(READONLY_OutsideHumidity);
    float temp_t,temp_h;
    if(sht20Inside.isConnected())
    {
        sht20Inside.read();
        temp_t=getInsideTemperature(),temp_h=getInsideHumidity();
        if(getHiveState()==HIVEAUTO)
        {
            controlHeater(autoprog);
            if(temp_t<autoprog.TemperatureTarget)
                setFan(255);
            else
                setFan(0);
            // if(temp_h>autoprog.fanOnHumidity )
            // {
            //     setFan(255);
            // }
            // else if(temp_h< autoprog.fanOnHumidity-autoprog.fanHHysteresis)
            // {
            //     setFan(0);
            // }
            // if(temp_t>autoprog.fanOnTemperature )
            // {
            //     setFan(255);
            // }
            // else if(temp_t< autoprog.fanOnTemperature-autoprog.fanTHysteresis)
            // {
            //     setFan(0);
            // }      
        }
        else if(getHiveState()==HIVESTERIL)
        {
            controlHeater(sterilprog);
            if(temp_t>sterilprog.fanOnTemperature )
            {
                setFan(255);
            }
            else if(temp_t< sterilprog.fanOnTemperature-sterilprog.fanTHysteresis)
            {
                setFan(0);
            }  
        }
        else
        {
            // setFan(0);
            // setHeater(0);
        }
    }
    else
    {
        setHiveState(HIVEMANUAL);
        setFan(0);
        setHeater(0);
    }
    bleNotify(READONLY_InsideTemperature);
    bleNotify(READONLY_InsideHumidity);     
    ////logging datas
    tik_counter++;
    if(tik_counter>=1*20)
    {
        tik_counter=0;
        String format=rtcint.getTime("%F") +"\n"+
                    rtcint.getTime("%T") +"\n"+        
                      "h cur:"+String(getHeaterCurrent())+"\n"+
                      "h avg cur:"+String(getHeaterAverageCurrent())+"\n"+
                      "h percent:"+String(getHeater())+"\n"+
                      "pid output:"+String(heaterOutput)+"\n"+
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

void Hive::controlHeater(PROG_t prog)
{
    if(sht20Inside.isConnected())
    {
        heaterInput = sht20Inside.getTemperature();
        if(getTargetTemperature()!=prog.TemperatureTarget)
            setTargetTemperature(prog.TemperatureTarget);
        heater.Compute();
        setHeater((uint8_t)heaterOutput);//
        bleNotify(READONLY_PIDState);    
    }
    else
    {
        setHeater(0);
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
    if(save)
    {
        ee.writeDouble(EE_ADD_KPCOEFF,kp);
        ee.writeDouble(EE_ADD_KICOEFF,ki);
        ee.writeDouble(EE_ADD_KDCOEFF,kd);
        ee.commit();        
    }
    pidCoeff.kp=kp;
    pidCoeff.ki=ki;
    pidCoeff.kd=kd;
    heater.SetTunings(kp,ki,kd);
}

void Hive::setAutoProg(PROG_t prog,bool save)
{
    if(save)
    {
        ee.put(EE_ADD_AUTOPROGS_START,prog);
        ee.commit();
    }
    memcpy(&autoprog,&prog,sizeof(prog));
}
void Hive::setSterilProg(PROG_t prog,bool save)
{
    if(save)
    {
        ee.put(EE_ADD_STERILPROGS_START,prog);
        ee.commit();
    }
    memcpy(&sterilprog,&prog,sizeof(prog));
}
_mode_t Hive::checkHeater(){
    uint8_t p=getHeater();
    setHeater(255);
    delay(100);
    // heater_current=readADC_Cal(ESP32_GPIO35_THERMALFEEDBACK)*10.0;
    heater_current=0;//ACS.mA_AC();
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
    fan_current=(float)readADC_Cal(ESP32_GPIO39_FANFEEDBACK)/4.7;
    setFan(s);
    if(fan_current>0.02)
        return MODEOK;
    else
        return MODEFAIL;
}
_mode_t Hive::checkPump(){ 
    return MODEOK;
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
