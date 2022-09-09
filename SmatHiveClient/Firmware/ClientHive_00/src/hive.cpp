#include "hive.h"
#include "UserBLE.h"
// #include "buildTime.h"

TwoWire I2C_1(0);
TwoWire I2C_2(1);
SoftWire I2C_3(ESP32_GPIO27_RTC_SDA, ESP32_GPIO32_RTC_SCL);

DateTime BUILD_DATETIME(__DATE__,__TIME__);
const PUMP_t DEFAULT_PUMP={
    .turnonlevel=HALF,
    .controlmode=LEVEL,
    .turnontime=TimeSpan(0,0,1,0)
};
const PROG_t DEFAULT_PROG_1={
    .active=true,
    .id=0,
    .start=DateTime(BUILD_DATETIME.year(),3,21,0,0,0),
    .stop=DateTime(BUILD_DATETIME.year(),6,21,23,59,59),
    .Sunrise=TimeSpan(0,0,0,0),
    .Sunset=TimeSpan(0,0,0,0),
    .minNormalTemperatureDay=17.0,
    .minNormalTemperatureNight=8,
    .dayTemperatureTarget=20.0,
    .nightTemperatureTarget=12.0,
    .emergencyTemperatureTarget=12.0,
    .fanOnTemperature=35.0,
    .fanOnHumidity=60.0,
    .fanOnHysteresis=2.0,
    .pumpOnDay=false,
    .pumpOnNight=false,
    .fanOnDay=true,
    .fanOnNight=true,
    .heaterOnDay=true,
    .heaterOnNight=true
    };
const PROG_t DEFAULT_PROG_2={
    .active=true,
    .id=1,
    .start=DateTime(BUILD_DATETIME.year(),6,22,0,0,0),
    .stop=DateTime(BUILD_DATETIME.year(),9,22,23,59,59),
    .Sunrise=TimeSpan(0,0,0,0),
    .Sunset=TimeSpan(0,0,0,0),
    .minNormalTemperatureDay=20.0,
    .minNormalTemperatureNight=12.0,
    .dayTemperatureTarget=20.0,
    .nightTemperatureTarget=12.0,
    .emergencyTemperatureTarget=12.0,
    .fanOnTemperature=35.0,
    .fanOnHumidity=60.0,
    .fanOnHysteresis=2.0,
    .pumpOnDay=false,
    .pumpOnNight=false,
    .fanOnDay=true,
    .fanOnNight=true,
    .heaterOnDay=true,
    .heaterOnNight=true
    };
const PROG_t DEFAULT_PROG_3={
    .active=true,
    .id=2,
    .start=DateTime(BUILD_DATETIME.year(),9,23,0,0,0),
    .stop=DateTime(BUILD_DATETIME.year(),12,21,23,59,59),
    .Sunrise=TimeSpan(0,0,0,0),
    .Sunset=TimeSpan(0,0,0,0),
    .minNormalTemperatureDay=10.0,
    .minNormalTemperatureNight=-5.0,
    .dayTemperatureTarget=15.0,
    .nightTemperatureTarget=12.0,
    .emergencyTemperatureTarget=12.0,
    .fanOnTemperature=35.0,
    .fanOnHumidity=60.0,
    .fanOnHysteresis=2.0,
    .pumpOnDay=true,
    .pumpOnNight=true,
    .fanOnDay=true,
    .fanOnNight=false,
    .heaterOnDay=true,
    .heaterOnNight=true
    };
const PROG_t DEFAULT_PROG_4={
    .active=true,
    .id=3,
    .start=DateTime(BUILD_DATETIME.year(),12,22,0,0,0),
    .stop=DateTime(BUILD_DATETIME.year(),6,21,23,59,59),
    .Sunrise=TimeSpan(0,0,0,0),
    .Sunset=TimeSpan(0,0,0,0),
    .minNormalTemperatureDay=0.0,
    .minNormalTemperatureNight=-5.0,
    .dayTemperatureTarget=15.0,
    .nightTemperatureTarget=12.0,
    .emergencyTemperatureTarget=12.0,
    .fanOnTemperature=35.0,
    .fanOnHumidity=60.0,
    .fanOnHysteresis=2.0,
    .pumpOnDay=false,
    .pumpOnNight=false,
    .fanOnDay=true,
    .fanOnNight=false,
    .heaterOnDay=true,
    .heaterOnNight=true
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
 * @brief Interrupt for Door openning with redrelay
 *
 */
void Hive::onReedrelayISR(void)
{
    if (sHive != 0)
        sHive->setDoorState(digitalRead(ESP32_GPIO23_REED));
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
        ee.writeDouble(EE_ADD_KPCOEFF,DEFAULT_KPCOEFF);
        ee.writeDouble(EE_ADD_KICOEFF,DEFAULT_KICOEFF);
        ee.writeDouble(EE_ADD_KDCOEFF,DEFAULT_KDCOEFF);
        ee.writeUShort(EE_ADD_LIGHTNIGHT,DEFAULT_LIGHT_NIGHT);
        ee.writeUShort(EE_ADD_LIGHTHYSTERESIS,DEFAULT_LIGHT_HSYTERESIS);        
        ee.put(EE_ADD_HIVEPROGS_START+0*EE_SZ_HIVEPROGS,DEFAULT_PROG_1);
        ee.put(EE_ADD_HIVEPROGS_START+1*EE_SZ_HIVEPROGS,DEFAULT_PROG_2);
        ee.put(EE_ADD_HIVEPROGS_START+2*EE_SZ_HIVEPROGS,DEFAULT_PROG_3);
        ee.put(EE_ADD_HIVEPROGS_START+3*EE_SZ_HIVEPROGS,DEFAULT_PROG_4);                
        ee.put(EE_ADD_PUMP,DEFAULT_PUMP);
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
    setLight(ee.readUShort(EE_ADD_LIGHTNIGHT),ee.readUShort(EE_ADD_LIGHTHYSTERESIS),false);
    setProg(ee.readAll(EE_ADD_HIVEPROGS_START+0*EE_SZ_HIVEPROGS,DEFAULT_PROG_1),false);
    setProg(ee.readAll(EE_ADD_HIVEPROGS_START+1*EE_SZ_HIVEPROGS,DEFAULT_PROG_2),false);
    setProg(ee.readAll(EE_ADD_HIVEPROGS_START+2*EE_SZ_HIVEPROGS,DEFAULT_PROG_3),false);
    setProg(ee.readAll(EE_ADD_HIVEPROGS_START+3*EE_SZ_HIVEPROGS,DEFAULT_PROG_4),false);        
    setPump(ee.readAll(EE_ADD_PUMP,DEFAULT_PUMP),false);
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
    pinMode(ESP32_GPIO39_PHOTOCELL, ANALOG);       // analog input
    pinMode(ESP32_GPIO33_FANFEEDBACK, ANALOG);     // analog input
    pinMode(ESP32_GPIO36_THERMALFEEDBACK, ANALOG); // analog input
    pinMode(ESP32_GPIO23_REED, INPUT);             // intterupt input
    attachInterrupt(digitalPinToInterrupt(ESP32_GPIO23_REED), Hive::onReedrelayISR, CHANGE);
    pinMode(ESP32_GPIO15_KEYDOWN, INPUT); // interrupt input
    attachInterrupt(digitalPinToInterrupt(ESP32_GPIO15_KEYDOWN), onKeyPress, FALLING);

    setDoorState(digitalRead(ESP32_GPIO23_REED));
    ///
    I2C_1.begin(ESP32_GPIO21_SDA1, ESP32_GPIO22_SCL1);
    I2C_1.setTimeOut(1000);
    I2C_2.begin(ESP32_GPIO16_SDA2, ESP32_GPIO17_SCL2);
    I2C_2.setTimeOut(1000);
    sht20Inside.begin(&I2C_1);
    sht20Outside.begin(&I2C_2);
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
                        String(BUILD_DATETIME.day())+" "+
                        String(BUILD_DATETIME.hour())+":"+
                        String(BUILD_DATETIME.minute())+":"+
                        String(BUILD_DATETIME.second())
        );
    }
    else
    {
        rtcneedAdjust = false;
        Serial.println("Read nvram ds1307 was ok");
    }
    rtcint.setTime(rtcext.now().unixtime());
    // /////////////////////////////////
    // // init SDCARD
    // SPIClass spi(HSPI);
    // spi.begin(ESP32_GPIO14_SDSCK, ESP32_GPIO12_SDMISO, ESP32_GPIO13_SDMOSI, ESP32_GPIO26_SDCS);
    // if (!SD.begin(ESP32_GPIO26_SDCS, spi, SDCARDSPEED))
    // {
    //     Serial.println("Card Mount Failed");
    // }
    // uint8_t cardType = SD.cardType();
    // if (cardType == CARD_NONE)
    // {
    //     Serial.println("No SD card attached");
    // }
    // else
    // {
    //     Serial.print("SD Card Type: ");
    //     if (cardType == CARD_MMC)
    //     {
    //         Serial.println("MMC");
    //     }
    //     else if (cardType == CARD_SD)
    //     {
    //         Serial.println("SDSC");
    //     }
    //     else if (cardType == CARD_SDHC)
    //     {
    //         Serial.println("SDHC");
    //     }
    //     else
    //     {
    //         Serial.println("UNKNOWN");
    //     }
    //     uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    //     Serial.printf("SD Card Size: %lluMB\n", cardSize);
    // }
    ////////////////////////////
    heaterInput = sht20Inside.getTemperature();
    heaterSetpoint = 37.0;
    heater.SetSampleTime(1000);
    heater.SetMode(AUTOMATIC);

    /////////////////////////////
    FreqCountESP.begin(ESP32_GPIO2_WATERLEVEL, 1000, 1); // frequncy input
    FreqCountESP.start();

    setFan(0);
    setHeater(0);
    setPumpStatus(false);
}
void Hive::update(void)
{
    while(!FreqCountESP.available());
    uint32_t freq_r = FreqCountESP.read();
    double cap = 1.44 / (3 * freq_r) * 1e6 - CAP_OFFSET;  //pico farad      
    if(cap<=CAP_LEVEL_EMPTY)
        feeding_level=EMPETY;
    else if(cap<=CAP_LEVEL_HALF)
        feeding_level=HALF;
    else
        feeding_level=FULL;
    //read adcs
    avg_heater_cnt++;
    heater_current=(float)analogRead(ESP32_GPIO36_THERMALFEEDBACK)*33.0/1024;
    sum_heater_current+=heater_current;
    if(avg_heater_cnt==3600)
    {
        avg_heater_current=(float)sum_heater_current/3600.0;
        sum_heater_current=0;
        avg_heater_cnt=0;
    }

    fan_current=analogRead(ESP32_GPIO33_FANFEEDBACK)*33.0/1024.0;

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
    ///check time ,sun and day

    DateTime now=rtcext.now();
    hiveposition.calculateSun(now);
    if(rtcext.now().second()==0 && isDeviceConnected())
    {
       bleNotify(READONLY_Sun);
    }
    if((now.unixtime()>getSunrise().unixtime() )&&
        (now.unixtime()<getSunset().unixtime()))
    {
        if(diurnal_status==NIGHT)
        {
            diurnal_status=DAY;
            if(isDeviceConnected())
            {
                bleNotify(READONLY_HiveDiurnal);
            }
        }
    }
    else
    {
        if(diurnal_status==DAY)
        {
            diurnal_status=NIGHT;
            if(isDeviceConnected())
            {
                bleNotify(READONLY_HiveDiurnal);
            }            
        }
    }
    //check status
    //run control algorithm
    if(getHiveState()==HIVEAUTO)
    {
      controlHeater();
    }
    else if(getHiveState()==HIVESTERIL)
    {

    }
    else if(getHiveState()==HIVEEMERGENCY)
    {

    }
    else
    {
        setFan(0);
        setHeater(0);
        setPumpStatus(false);
    }

}
float Hive::getHeaterAverageCurrent(){ 
    if(avg_heater_cnt>0)
    {
        return (sum_heater_current/avg_heater_cnt*3600.0+avg_heater_current)/(avg_heater_cnt+1);
    }
    else
    {
        return avg_heater_current;
    }
};

void Hive::controlHeater(void)
{
    if(sht20Inside.isConnected())
    {
        heaterInput = sht20Inside.getTemperature();
        heater.Compute();
        setHeater(heaterOutput);
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
void Hive::setLight(uint16_t levelNight, uint16_t levelHysteresis,bool save)
{
    if(save)
    {
        ee.writeUShort(EE_ADD_LIGHTNIGHT,levelNight);
        ee.writeUShort(EE_ADD_LIGHTHYSTERESIS,levelHysteresis);
        ee.commit();        
    }
    lightlevelNight.level=levelNight;
    lightlevelNight.hysteresis=levelHysteresis;
}
void Hive::setProg(PROG_t prog,bool save)
{
    if(save)
    {
        ee.put(EE_ADD_HIVEPROGS_START+prog.id*EE_SZ_HIVEPROGS,prog);
        ee.commit();
    }
    memcpy(&progs[prog.id],&prog,sizeof(PROG_t));
}
void Hive::setPump(PUMP_t p,bool save)
{
    if(save)
    {
        ee.put(EE_ADD_PUMP,p);
        ee.commit();        
    }
    memcpy(&pump,&p,sizeof(PUMP_t));
}
_mode_t Hive::checkHeater(){
    setHeater(255);
    delay(100);
    heater_current=analogRead(ESP32_GPIO36_THERMALFEEDBACK)*33.0/1024.0;
    setHeater(heaterOutput);
    if(heater_current>0.03)
        return MODEOK;
    else
        return MODEFAIL;

}
_mode_t Hive::checkFan()
{
    setFan(255);
    delay(100);
    fan_current=analogRead(ESP32_GPIO33_FANFEEDBACK)*33.0/1024.0;
    setFan(fan_speed);
    if(fan_current>0.03)
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
