#ifndef __USER_BLE_H__
#define __USER_BLE_H__
#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLESecurity.h>
#include "hive.h"
#define ENABLE_PAIR_ENCRYPTION  0

#define bleStaticPIN    123456
#define SerialNumberString "H0000001T"
#define SoftwareVersionString  "1.0"
#define HardwareVersionString  "2.0"


extern BLEServer *hiveServer;
extern BLEAdvertising *hiveAdvertising;


/**
 * @brief Server Callbacks
 *
 */
class hiveServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *server)
    {
        // hive.setConnectionState(true);
#if !ENABLE_PAIR_ENCRYPTION
            hive.setConnectionState(true);
#endif
        Serial.println("******Device connected");
        hiveServer->getAdvertising()->stop();
    }
    void onDisconnect(BLEServer *server)
    {
        hive.setConnectionState(false);
        Serial.println("Device disconnected******");
        // rtcServer->startAdvertising();
        hiveServer->getAdvertising()->start();
    }
};
/**
 * @brief Security Callbacks
 * 
 */
class hiveSecurityCallbacks:public BLESecurityCallbacks{

  uint32_t onPassKeyRequest() {
      return bleStaticPIN;
    }

    void onPassKeyNotify(uint32_t pass_key) {}

    bool onConfirmPIN(uint32_t pass_key) {
      Serial.println("   - SecurityCallback - onConfirmPIN:"+String(pass_key));
      vTaskDelay(5000);
      return true;
    }

    bool onSecurityRequest() {
      return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
      if (cmpl.success) {
        Serial.println("   - SecurityCallback - Authentication Success");
        hive.setConnectionState(true);
      } else {
        hive.setConnectionState(false);
        Serial.println("   - SecurityCallback - Authentication Failure*");
        // pServer->removePeerDevice(pServer->getConnId(), true);
      }
    //   BLEDevice::startAdvertising();
    }

};
/**
 * @brief ReadWrite Parameters
 * 
 */
static const BLEUUID READWRITE_SERVICE_UUID("72bf5ea3-89f8-4d3c-86d2-fc4fbe559545");
extern BLEService *readwriteService;
#define READWRITE_CHARACTRISTIC_NUMBER  7

#define READWRITE_Position    0
#define READWRITE_DeviceName  1
#define READWRITE_Time        2
#define READWRITE_Date        3
#define READWRITE_BlePassword 4
#define READWRITE_PIDCoeff    5
#define READWRITE_LightLevel  6

#define POSITION_UUID     BLEUUID("bcd72712-7705-49c5-b2a8-bbc0eb72a988")
#define DEVICE_NAME_UUID  BLEUUID("dc4a6c5f-8d4a-4e4a-9dbe-2f7aad811f80")
#define TIME_UUID         BLEUUID("fb75341e-8ab1-4355-9b69-39471f6be25d")
#define DATE_UUID         BLEUUID("ae928490-34aa-4e9c-be83-0ca678f5eda2")
#define BLE_PASSWORD_UUID BLEUUID("f1120eac-310b-4ffe-93c3-1d8aba7c58f5")
#define PID_COEFF_UUID    BLEUUID("e887f503-36fd-463d-b4d6-77f024094347")
#define LIGHT_LEVEL_UUID  BLEUUID("dc59a1c7-6536-4cff-a24b-4e0c3a28108b")
/**
 * @brief ReadWrite Charactristic Callbacks
 * 
 */
class readwriteCharactristicCallbacks: public BLECharacteristicCallbacks{
  void onRead(BLECharacteristic *pCharactristic){
    if(pCharactristic->getUUID().equals(DEVICE_NAME_UUID)){
      pCharactristic->setValue(hive.gethiveName().c_str());
    }
  };
	void onWrite(BLECharacteristic* pCharacteristic){

    String value_str;
    value_str=pCharacteristic->getValue().c_str();
    Serial.println(value_str);
    ///split spring with comma
    int pos_deliminator[100];
    //Apply value to paramters
    if(pCharacteristic->getUUID().equals(POSITION_UUID)){
      String lat,lng;
      pos_deliminator[0]=value_str.indexOf(",");
      pos_deliminator[1]=value_str.indexOf(",",pos_deliminator[0]+1);
      lat=value_str.substring(0,pos_deliminator[0]);
      lng=value_str.substring(pos_deliminator[0]+1,pos_deliminator[1]);
      hive.sethivePosition(atof(lat.c_str()),atof(lng.c_str()));
    }
    if(pCharacteristic->getUUID().equals(DEVICE_NAME_UUID)){
      if(value_str.length()>0)
      {
        hiveServer->getAdvertising()->stop();
        hive.sethiveName(value_str);
        esp_ble_gap_set_device_name(value_str.c_str());
        hiveServer->getAdvertising()->start(); //hiveServer->startAdvertising();
      }
    }
  };
};
/**
 * @brief Readonly Parameters
 * 
 */
static const BLEUUID READONLY_SERVICE_UUID("581c15b6-ae8f-4f55-b14d-bbe8b298db28");
extern BLEService *readonlyService;
#define READONLY_CHARACTRISTIC_NUMBER 19

#define READONLY_SerialNumber         0
#define READONLY_Softwareversion      1
#define READONLY_Hardwareversion      2
#define READONLY_InsideTemperature    3
#define READONLY_InsideHumidity       4
#define READONLY_OutsideTemperature   5
#define READONLY_OutsideHumidity      6
#define READONLY_OutsideLight         7
#define READONLY_DoorState            8
#define READONLY_HiveWeight           9
#define READONLY_LevelFeeding         10
#define READONLY_AverageCurrent       11
#define READONLY_FanStatus            12
#define READONLY_PumpStatus           13
#define READONLY_InsideSensor         14
#define READONLY_OutsideSensor        15
#define READONLY_HiveDiurnal          16
#define READONLY_RTCStatus            17
#define READONLY_HiveState            18

#define SERIAL_NUMBER_UUID    BLEUUID("734447d3-2cc6-4c0d-8eb6-ad942a931d01")
#define SOFTWARE_VERSION_UUID BLEUUID("a628a3d6-51b4-44c2-b2d1-37c9b8be6972")
#define HARDWARE_VERSION_UUID BLEUUID("00ce255b-d0ca-42ec-8ff2-7df81cfa3f52")
#define TEMP_INSIDE_UUID      BLEUUID("735d97aa-84e4-49b8-9c1c-8f9c47f0045f")
#define HUM_INSIDE_UUID       BLEUUID("c8abadb3-d645-4b16-a851-d113857bce4e")
#define TEMP_OUTSIDE_UUID     BLEUUID("9a170546-0e58-433d-8057-8478b95c0a18")
#define HUM_OUTSIDE_UUID      BLEUUID("fdda7dfd-79fa-41b6-80d8-ef01f6007360")
#define LIGHTNESS_UUID        BLEUUID("bcd72712-7705-49c5-b2a8-bbc0eb72a988")
#define DOOR_STATE_UUID       BLEUUID("29012989-0121-4fdb-b4b6-b1f62ffa15d1")
#define HIVE_WEIGHT_UUID      BLEUUID("92ae03b0-87dc-482d-8322-5ff2d12b6fe7")
#define HIVE_FEEDING_LEVEL    BLEUUID("262ceb5c-19a1-4167-88bb-3a59f95b24ce")
#define AVERAGE_CURRENT_UUID  BLEUUID("fbf5e2d6-9d02-42c2-a33c-0b5bd6c22727")
#define FAN_STATUS_UUID       BLEUUID("b713687e-b84d-4ab8-b94b-af04c7b7b0ed")
#define PUMP_STATUS_UUID      BLEUUID("225f1fd0-10ba-46a9-859f-68e51d9be8f1")
#define SENSOR_INSIDE_UUID    BLEUUID("4dff8580-be85-45b8-bb8c-8d0477a4d3be")
#define SENSOR_OUTSIDE_UUID   BLEUUID("659cbe7c-0017-4625-987d-4c8c7a44a30a")
#define HIVE_DIURNAL_UUID     BLEUUID("aebe1a02-ca7b-4538-a540-ac35cdd1c8e9")
#define RTC_STATUS_UUID       BLEUUID("edc7e652-5627-42e1-b9aa-68db0ed33568")
#define HIVE_STATE_UUID       BLEUUID("c89076e1-7f68-4f6d-8a55-c4956c8da8e1")


/**
 * @brief Readonly Charactristic Callbacks
 * 
 */
class readonlyCharactristicCallbacks:public BLECharacteristicCallbacks{
    void onRead(BLECharacteristic *pCharactristic)
    {
        String strTemp;
        if(pCharactristic->getUUID().equals(SERIAL_NUMBER_UUID))
        {
            pCharactristic->setValue(SerialNumberString);
        }
        if(pCharactristic->getUUID().equals(SOFTWARE_VERSION_UUID))
        {
            pCharactristic->setValue(SoftwareVersionString);
        }
        if(pCharactristic->getUUID().equals(HARDWARE_VERSION_UUID))
        {
            pCharactristic->setValue(HardwareVersionString);
        }        
        if(pCharactristic->getUUID().equals(TEMP_INSIDE_UUID))
        {
             strTemp=hive.sht20Inside.isConnected()?String(hive.sht20Inside.getTemperature()):"---";
            pCharactristic->setValue(strTemp.c_str());
        }
        if(pCharactristic->getUUID().equals(HUM_INSIDE_UUID))
        {
             strTemp=hive.sht20Inside.isConnected()?String(hive.sht20Inside.getHumidity()):"---";
            pCharactristic->setValue(strTemp.c_str());
        }   
        if(pCharactristic->getUUID().equals(TEMP_OUTSIDE_UUID))
        {
             strTemp=hive.sht20Outside.isConnected()?String(hive.sht20Outside.getTemperature()):"---";
            pCharactristic->setValue(strTemp.c_str());
        }
        if(pCharactristic->getUUID().equals(HUM_OUTSIDE_UUID))
        {
             strTemp=hive.sht20Outside.isConnected()?String(hive.sht20Outside.getHumidity()):"---";
            pCharactristic->setValue(strTemp.c_str());
        }  
        if(pCharactristic->getUUID().equals(LIGHTNESS_UUID))
        {
             strTemp=String(hive.readPhotoCell());
            pCharactristic->setValue(strTemp.c_str());
        } 
        if(pCharactristic->getUUID().equals(DOOR_STATE_UUID))
        {
             strTemp=(hive.readDoorState())?"open":"close";
            pCharactristic->setValue(strTemp.c_str());
        } 
        if(pCharactristic->getUUID().equals(HIVE_WEIGHT_UUID))
        {
             strTemp=(hive.getHiveWeightState())?String(hive.readWeight()):"---";
            pCharactristic->setValue(strTemp.c_str());
        } 
        if(pCharactristic->getUUID().equals(HIVE_FEEDING_LEVEL))
        {
            int level=hive.getHiveFeedingLevel();
            if(level==0)
            {
              strTemp="Empty";
            }else if(level==1){
              strTemp="Half";
            }
            else{
              strTemp="Full";
            }

            pCharactristic->setValue(strTemp.c_str());
        }                                 
        if(pCharactristic->getUUID().equals(AVERAGE_CURRENT_UUID))
        {
             strTemp=String(hive.getHeaterAverageCurrent());
            pCharactristic->setValue(strTemp.c_str());
        } 
        if(pCharactristic->getUUID().equals(FAN_STATUS_UUID))
        {
             strTemp=(hive.getFanStatus()?"OK":"FAIL");
            pCharactristic->setValue(strTemp.c_str());
        } 

        if(pCharactristic->getUUID().equals(PUMP_STATUS_UUID))
        {
             strTemp=(hive.getPumpStatus()?"ON":"OFF");
            pCharactristic->setValue(strTemp.c_str());
        } 
        if(pCharactristic->getUUID().equals(SENSOR_INSIDE_UUID))
        {
             strTemp=(hive.sht20Inside.isConnected()?"OK":"FAIL");
            pCharactristic->setValue(strTemp.c_str());
        } 
        if(pCharactristic->getUUID().equals(SENSOR_OUTSIDE_UUID))
        {
             strTemp=(hive.sht20Outside.isConnected()?"OK":"FAIL");
            pCharactristic->setValue(strTemp.c_str());
        } 
        if(pCharactristic->getUUID().equals(HIVE_DIURNAL_UUID))
        {
             strTemp=(hive.getDiurnalStatus()==DAY ?"DAY":"NIGHT");
            pCharactristic->setValue(strTemp.c_str());
        }                 
        if(pCharactristic->getUUID().equals(RTC_STATUS_UUID))
        {
             strTemp=hive.getRTCStatus()?"Adjust":"Correct";
           pCharactristic->setValue(strTemp.c_str());
        }
        if(pCharactristic->getUUID().equals(HIVE_STATE_UUID))
        {
             strTemp=(hive.getHiveState()==NORMAL?"NORMAL":"EMERGENCY");
            pCharactristic->setValue(strTemp.c_str());
        }         
    };
};

 void bleInit();
 void bleNotifyInitializeData();
#endif