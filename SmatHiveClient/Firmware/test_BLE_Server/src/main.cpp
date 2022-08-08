#include <Arduino.h>
#include <WiFi.h>
#include "ESP32Time.h"
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLESecurity.h>
unsigned long lastTime = 0;
/**
 * @brief other variables
 * 
 */

ESP32Time rtc_int(0);//3600*3+1800
tm cur_Time,prev_Time;
typedef struct {
  uint16_t year;
  uint8_t mon;
  uint8_t day;
  uint8_t hr;
  uint8_t min;
  uint8_t sec;
} mytm_t;
/**
 * @brief BLR variables
 * 
 */
#define bleServerName "RTC_ESP32"
bool deviceConnected = false;
/*define UUID for service and charactristic*/
static const BLEUUID RTC_SERVICE_UUID((uint16_t)0x1805);
static const BLEUUID RTC_CHARACTRISTIC_UUID((uint16_t)0x2A2B);

static const BLEUUID Thermometer_SERVICE_UUID("581c15b6-ae8f-4f55-b14d-bbe8b298db28");
static const BLEUUID Thermometer1_CHARACTRISTIC_UUID((uint16_t)0x2a6e);//
static const BLEUUID Thermometer2_CHARACTRISTIC_UUID((uint16_t)0x2a6e);//"f875a791-ccea-4dc5-a661-4db54989067f"


/* define variable for sevice and charactristic*/
BLEServer *hiveServer;

BLEService *rtcService;
BLECharacteristic *rtcCharactristic;
BLEDescriptor rtcDescriptor(BLEUUID((uint16_t)0x2903));

BLEService *thermometerService;
BLECharacteristic *thermometer1Charactristic;
BLEDescriptor thermometer1Descriptor(BLEUUID((uint16_t)0x2903));
BLECharacteristic *thermometer2Charactristic;
BLEDescriptor thermometer2Descriptor(BLEUUID((uint16_t)0x2903));
/**
 * @brief Server callbacks
 * 
 */
class hiveServerCallbacks:public BLEServerCallbacks
{
  void onConnect(BLEServer *server)
  {
    deviceConnected=true;
    log_i("Device connected");
    hiveServer->getAdvertising()->stop();
  }
  void onDisconnect(BLEServer *server)
  {
    deviceConnected=false;
    log_i("Device disconnected");
      // rtcServer->startAdvertising();
    hiveServer->getAdvertising()->start();
  }
};
/**
 * @brief rtc charactristic callbacks
 * 
 */
class rtcCharactristicCallbacks:public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic* pCharacteristic)
  {
    cur_Time=rtc_int.getTimeStruct();
    tm *now=&cur_Time;
    uint8_t data[10] = {0};
    data[0] = (1900 + now->tm_year) & 0xff;
    data[1] = ((1900 + now->tm_year) >> 8) & 0xff;
    data[2] = now->tm_mon + 1;
    data[3] = now->tm_mday;
    data[4] = now->tm_hour;
    data[5] = now->tm_min;
    data[6] = now->tm_sec;
    data[7] = now->tm_wday + 1;
    data[8] = 0;//(chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count() % 1000) * 256 / 1000;
    pCharacteristic->setValue(data,sizeof(data));
    log_i("respond with new timr:%d-%d-%d,%d-%d-%d,%d",(data[1]<<8)+data[0],data[2],data[3],data[4],data[5],data[6],data[7]);
  }
  void onWrite(BLECharacteristic* pCharacteristic)
  {
     cur_Time=rtc_int.getTimeStruct();
    uint8_t data[7];

    memcpy(data,pCharacteristic->getData(),sizeof(data));
    cur_Time.tm_year=((uint16_t)(data[1]<<8)&0xff00)+(uint16_t)(data[0]&0xff)-(uint16_t)1900;
    cur_Time.tm_mon=data[2]-1;
    cur_Time.tm_mday=data[3];
    cur_Time.tm_hour=data[4];
    cur_Time.tm_min=data[5];
    cur_Time.tm_sec=data[6];

  
    rtc_int.setTimeStruct(cur_Time);
    log_i("onWrite is rtime in Charactristic:%d-%d-%d,%d-%d-%d",(data[1]<<8)+data[0],data[2],data[3],data[4],data[5],data[6]);
    log_i("onWrite is tmp_t in Charactristic:%d-%d-%d,%d-%d-%d",cur_Time.tm_year,cur_Time.tm_mon,cur_Time.tm_mday,cur_Time.tm_hour,cur_Time.tm_min,cur_Time.tm_sec);

  }

};

class rtcDescriptionCallbacks: public BLEDescriptorCallbacks
{
  void onWrite(BLEDescriptor* pDescriptor)
  {
    log_i("Data is Received in Description");
  }
};
/**
 * @brief thermometer1 charactristic
 * 
 */
class thermometer1CharactristicCallbacks:public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic* pCharacteristic)
  {
    double temp1=36.8;
    // static char temperatureCTemp[6];
    // dtostrf(temp1, 6, 2, temperatureCTemp);
    // pCharacteristic->setValue(temperatureCTemp);
    int temp1_i=(int)(3680);
    pCharacteristic->setValue(temp1_i);
    log_i("respond temp1:%f,%d",temp1,temp1_i);
  }
};
/**
 * @brief thermometer2 charactristic
 * 
 */
class thermometer2CharactristicCallbacks:public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic* pCharacteristic)
  {
    double temp2=46.6;
    //  static char temperatureCTemp[6];
    // dtostrf(temp2, 6, 2, temperatureCTemp);
    // pCharacteristic->setValue(temperatureCTemp);
    // log_i("respond temp1:%f",temp2);
        int temp1_i=(int)((double)temp2*100.0);
    pCharacteristic->setValue(temp1_i);
    log_i("respond temp2:%f,%d",temp2,temp1_i);
  }
};
/**
 * @brief 
 *  * Timer ISR for very 1 second:
 * reading sensors for example
  */
hw_timer_t *timer0_1s=NULL;
portMUX_TYPE timer0Mux=portMUX_INITIALIZER_UNLOCKED;
volatile bool timer0_f=false;
void IRAM_ATTR onTimer0()
{
  portENTER_CRITICAL_ISR(&timer0Mux);
  timer0_f=true;
  portEXIT_CRITICAL_ISR(&timer0Mux);
}
/**
 * @brief 
 * 
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(ESP.getChipCores());
  Serial.println(ESP.getChipModel());
  Serial.println(ESP.getChipRevision());
  Serial.println(ESP.getCpuFreqMHz());
  Serial.println(ESP.getFlashChipSize());
  Serial.println(ESP.getFlashChipSpeed());
  Serial.println(ESP.getFreeSketchSpace());
  rtc_int.setTime(30, 24, 15, 26, 7, 2022); 
  // init timer0_1s//
  timer0_1s=timerBegin(0,80,true);//80M/80=1Mhz
  timerAttachInterrupt(timer0_1s,&onTimer0,true);
  timerAlarmWrite(timer0_1s,1000000,true);//every 1hz
  timerAlarmEnable(timer0_1s);  
  //start BLE
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  BLEDevice::init(bleServerName);
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);  

  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setStaticPIN(123456); 
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_NO_BOND	);
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK); 
  //  uint32_t passkey = 123456;
  // esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  hiveServer = BLEDevice::createServer();
  hiveServer->setCallbacks(new hiveServerCallbacks());
  /******define rtc service********/
  rtcService = hiveServer->createService(RTC_SERVICE_UUID);
  rtcCharactristic= rtcService->createCharacteristic(RTC_CHARACTRISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY
                                                                        | BLECharacteristic::PROPERTY_READ 
                                                                         | BLECharacteristic::PROPERTY_WRITE
                                                                         //|BLECharacteristic::PROPERTY_WRITE_NR
                                                                                            );
  rtcCharactristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);  
  rtcCharactristic->setCallbacks(new rtcCharactristicCallbacks());
  rtcDescriptor.setValue("rtc internal Date");
  rtcDescriptor.setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED|ESP_GATT_PERM_WRITE_ENCRYPTED);
  rtcCharactristic->addDescriptor(&rtcDescriptor);
  rtcService->start();
      
  /******define thermo service********/
  thermometerService = hiveServer->createService(Thermometer_SERVICE_UUID);

  thermometer1Charactristic= thermometerService->createCharacteristic(Thermometer1_CHARACTRISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY
                                                                        | BLECharacteristic::PROPERTY_READ 
                                                                        //  | BLECharacteristic::PROPERTY_WRITE
                                                                         //|BLECharacteristic::PROPERTY_WRITE_NR
                                                                                            );
  thermometer1Charactristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED|ESP_GATT_PERM_WRITE_ENCRYPTED);
  thermometer1Charactristic->setCallbacks(new thermometer1CharactristicCallbacks());
  thermometer1Descriptor.setValue("thermo1");
  thermometer1Descriptor.setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED|ESP_GATT_PERM_WRITE_ENCRYPTED);
  thermometer1Charactristic->addDescriptor(&thermometer1Descriptor);

  thermometer2Charactristic= thermometerService->createCharacteristic(Thermometer2_CHARACTRISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY
                                                                        | BLECharacteristic::PROPERTY_READ 
                                                                        //  | BLECharacteristic::PROPERTY_WRITE
                                                                         //|BLECharacteristic::PROPERTY_WRITE_NR
                                                                                            );
  thermometer2Charactristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED|ESP_GATT_PERM_WRITE_ENCRYPTED);
  thermometer2Charactristic->setCallbacks(new thermometer2CharactristicCallbacks());
  thermometer2Descriptor.setValue("thermo2");
  thermometer2Descriptor.setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED|ESP_GATT_PERM_WRITE_ENCRYPTED);
  thermometer2Charactristic->addDescriptor(&thermometer2Descriptor);  

  thermometerService->start();
  /*****start advertising*********/                                                                                        
  BLEAdvertising *hiveAdvertising = BLEDevice::getAdvertising();
  hiveAdvertising->addServiceUUID(RTC_SERVICE_UUID);
  // hiveAdvertising->addServiceUUID(Thermometer_SERVICE_UUID);
  hiveAdvertising->setScanResponse(true);
  hiveAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  hiveAdvertising->setMinPreferred(0x12);
  
  hiveServer->getAdvertising()->start(); //BLEDevice::startAdvertising();  rtcServer->startAdvertising();
  /*******************/
    const char *macAddress_ = BLEDevice::getAddress().toString().c_str();
  Serial.println(macAddress_);
   log_i("Characteristic defined! Now you can read it in your phone!");
  prev_Time=rtc_int.getTimeStruct();
  tm *now=&prev_Time;
  uint8_t data[10] = {0};
  data[0] = (1900 + now->tm_year) & 0xff;
  data[1] = ((1900 + now->tm_year) >> 8) & 0xff;
  data[2] = now->tm_mon + 1;
  data[3] = now->tm_mday;
  data[4] = now->tm_hour;
  data[5] = now->tm_min;
  data[6] = now->tm_sec;
  data[7] = now->tm_wday + 1;
  data[8] = 0;//(chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count() % 1000) * 256 / 1000;
  rtcCharactristic->setValue(data,sizeof(data));       
  rtcCharactristic->notify(true);
  
  lastTime=millis();
}
/**
 * @brief 
 * 
 */

void loop() {
  // put your main code here, to run repeatedly:

    // if((millis()-lastTime)>1000)
    if(timer0_f)
    {
      lastTime=millis();
      portENTER_CRITICAL(&timer0Mux);
      timer0_f=false;
      portEXIT_CRITICAL(&timer0Mux);   
      cur_Time=rtc_int.getTimeStruct();    
      Serial.println(rtc_int.getDateTime(true));
      if(deviceConnected)
      {
        if(prev_Time.tm_sec!=cur_Time.tm_sec)
        {
          tm *now=&cur_Time;
          uint8_t data[10] = {0};
          data[0] = (1900 + now->tm_year) & 0xff;
          data[1] = ((1900 + now->tm_year) >> 8) & 0xff;
          data[2] = now->tm_mon + 1;
          data[3] = now->tm_mday;
          data[4] = now->tm_hour;
          data[5] = now->tm_min;
          data[6] = now->tm_sec;
          data[7] = now->tm_wday + 1;
          data[8] = 0;//(chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count() % 1000) * 256 / 1000;
          rtcCharactristic->setValue(data,sizeof(data));       
          rtcCharactristic->notify(true);
          //  log_i("onNotify with new :%d-%d-%d,%d-%d-%d,%d",(data[1]<<8)+data[0],data[2],data[3],data[4],data[5],data[6],data[7]);

        }
        memccpy(&prev_Time,&cur_Time,1,sizeof(cur_Time));
      }    
  } 

}