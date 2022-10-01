#ifndef __USER_BLE_H__
#define __USER_BLE_H__
#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLESecurity.h>
#include "hive.h"
#define ENABLE_PAIR_ENCRYPTION 0

#define SerialNumberString "H0000001T"
#define SoftwareVersionString "1.1"
#define HardwareVersionString "2.0"

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
class hiveSecurityCallbacks : public BLESecurityCallbacks
{

  uint32_t onPassKeyRequest()
  {
    return hive.getblepassword();
  }

  void onPassKeyNotify(uint32_t pass_key) {}

  bool onConfirmPIN(uint32_t pass_key)
  {
    Serial.println("   - SecurityCallback - onConfirmPIN:" + String(pass_key));
    vTaskDelay(5000);
    return true;
  }

  bool onSecurityRequest()
  {
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
  {
    if (cmpl.success)
    {
      Serial.println("   - SecurityCallback - Authentication Success");
      hive.setConnectionState(true);
    }
    else
    {
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
#define READWRITE_CHARACTRISTIC_NUMBER 11

#define READWRITE_Position          0
#define READWRITE_DeviceName        1
#define READWRITE_Time              2
#define READWRITE_Date              3
#define READWRITE_BlePassword       4
#define READWRITE_PIDCoeff          5
#define READWRITE_FanMode           6
#define READWRITE_HeaterMode        7
#define READWRITE_AutoProg          8
#define READWRITE_SterilProg        9
#define READWRITE_HiveState         10

#define POSITION_UUID               BLEUUID("bcd72712-7705-49c5-b2a8-bbc0eb72a988")
#define DEVICE_NAME_UUID            BLEUUID("dc4a6c5f-8d4a-4e4a-9dbe-2f7aad811f80")
#define TIME_UUID                   BLEUUID("fb75341e-8ab1-4355-9b69-39471f6be25d")
#define DATE_UUID                   BLEUUID("ae928490-34aa-4e9c-be83-0ca678f5eda2")
#define BLE_PASSWORD_UUID           BLEUUID("f1120eac-310b-4ffe-93c3-1d8aba7c58f5")
#define PID_COEFF_UUID              BLEUUID("e887f503-36fd-463d-b4d6-77f024094347")
#define FANMODE_UUID                BLEUUID("dc59a1c7-6536-4cff-a24b-4e0c3a28108b")
#define HEATERMODE_UUID             BLEUUID("fdcbcf24-2aff-4ab4-a6bb-a656ce0dce0a")
#define AUTO_PROG_UUID              BLEUUID("7b91fb80-a20d-4a97-b2ad-48728db985f8")
#define STRILPROG_UUID              BLEUUID("7b91fb80-a20d-4a97-b2ad-48728db985f8")
#define HIVE_STATE_UUID             BLEUUID("c89076e1-7f68-4f6d-8a55-c4956c8da8e1")



/**
 * @brief ReadWrite Charactristic Callbacks
 *
 */
class readwriteCharactristicCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    String tmp_str;
    if (pCharacteristic->getUUID().equals(POSITION_UUID))
    {
      position_t tmp_pos = hive.getPosition();
      tmp_str = String(tmp_pos.latitude,6) + "," + String(tmp_pos.longitude,6);
      pCharacteristic->setValue(tmp_str.c_str());
    }
    if (pCharacteristic->getUUID().equals(DEVICE_NAME_UUID))
    {
      tmp_str=hive.gethiveName();
      pCharacteristic->setValue(tmp_str.c_str());
    }
    if (pCharacteristic->getUUID().equals(TIME_UUID))
    {
      DateTime tmp_time =hive.getDateTime();
      tmp_str=String(tmp_time.hour())+":"+
              String(tmp_time.minute())+":"+
              String(tmp_time.second());//+","+
      pCharacteristic->setValue(tmp_str.c_str());
    }
    if (pCharacteristic->getUUID().equals(DATE_UUID))
    {
      DateTime tmp_time =hive.getDateTime();
      tmp_str=String(tmp_time.year())+"-"+
              String(tmp_time.month())+"-"+
              String(tmp_time.day());
      pCharacteristic->setValue(tmp_str.c_str());
    }
    if (pCharacteristic->getUUID().equals(PID_COEFF_UUID))
    {
      PIDCoeff_t tmp_pid =hive.getPID();
      tmp_str=String(tmp_pid.kp,2)+","+
              String(tmp_pid.ki,2)+","+
              String(tmp_pid.kd,2);
      pCharacteristic->setValue(tmp_str.c_str());
    }
    if (pCharacteristic->getUUID().equals(FANMODE_UUID))
    {
      _fanmode_t fanmode=hive.getFanMode();
      if(fanmode==FANAUTO)
        tmp_str="AUTO";
      else if(fanmode==FANOFF)
        tmp_str="OFF";
      else
        tmp_str="ON";
      pCharacteristic->setValue(tmp_str.c_str());

    }
    if (pCharacteristic->getUUID().equals(HEATERMODE_UUID))
    {
      _heatermode_t heatermode=hive.getHeaterMode();
      if(heatermode==HEATERAUTO)
        tmp_str="AUTO";
      else if(heatermode==HEATEROFF)
        tmp_str="OFF";
      else
        tmp_str="ON";
      pCharacteristic->setValue(tmp_str.c_str());

    }    
    if (pCharacteristic->getUUID().equals(AUTO_PROG_UUID)) 
    {
      AUTOProg_t tmp_prog=hive.getAutoProg();
      tmp_str=String(tmp_prog.temperature,1)+","+
              String(tmp_prog.fanhumidity,0)+","+
              String(tmp_prog.fantemperature,1)+","+
              String(tmp_prog.fanhysthumidity,0)+","+
              String(tmp_prog.fanhysttemperature,1);
             pCharacteristic->setValue(tmp_str.c_str());
    }    
    if (pCharacteristic->getUUID().equals(STRILPROG_UUID)) 
    {
      STERILProg_t tmp_prog=hive.getSterilProg();
      tmp_str=String(tmp_prog.temperature,1)+","+
              String(tmp_prog.duration.hours())+":"+
              String(tmp_prog.duration.minutes())+":"+ 
              String(tmp_prog.duration.seconds());
             pCharacteristic->setValue(tmp_str.c_str());
    } 
    if (pCharacteristic->getUUID().equals(HIVE_STATE_UUID))
    {
      _hivestate_t st=hive.getHiveState();
      if(st==HIVEAUTO)
        tmp_str = "AUTO";
      else if(st==HIVESTERIL)
        tmp_str="STERIL";
      else
        tmp_str="OFF";
      pCharacteristic->setValue(tmp_str.c_str());
    }    

  };
  void onWrite(BLECharacteristic *pCharacteristic)
  {

    String value_str;
    value_str = pCharacteristic->getValue().c_str();
    Serial.println(value_str);
    /// split spring with comma
    int pos_deliminator[100];
    // Apply value to paramters
    if (value_str.length() > 0)
    {
      if (pCharacteristic->getUUID().equals(POSITION_UUID))
      {
        String lat, lng;
        pos_deliminator[0] = value_str.indexOf(",");
        pos_deliminator[1] = value_str.indexOf(",", pos_deliminator[0] + 1);
        lat = value_str.substring(0, pos_deliminator[0]);
        lng = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]);
        Serial.println("******" + lat + "," + lng);
        hive.setPosition(atof(lat.c_str()), atof(lng.c_str()), true);
      }
      if (pCharacteristic->getUUID().equals(DEVICE_NAME_UUID))
      {
        if (value_str.length() > 0)
        {
          hiveServer->getAdvertising()->stop();
          hive.sethiveName(value_str, true);
          esp_ble_gap_set_device_name(value_str.c_str());
          hiveServer->getAdvertising()->start(); // hiveServer->startAdvertising();
          ESP.restart();
        }
      }
      if (pCharacteristic->getUUID().equals(TIME_UUID))
      {
        String h, m, s,y,mm,d;
        pos_deliminator[0] = value_str.indexOf(":");
        pos_deliminator[1] = value_str.indexOf(":", pos_deliminator[0] + 1);
        pos_deliminator[2] = value_str.indexOf(":", pos_deliminator[1] + 1);
        h = value_str.substring(0, pos_deliminator[0]);
        m = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]);
        s = value_str.substring(pos_deliminator[1] + 1, pos_deliminator[2]);
        DateTime tmp_time = hive.getDateTime();
        hive.setDateTime(DateTime(tmp_time.year(),
                                  tmp_time.month(),
                                  tmp_time.day(),
                                  h.toInt(),
                                  m.toInt(),
                                  s.toInt()));

      }
      if (pCharacteristic->getUUID().equals(DATE_UUID))
      {
        String year, month, day;
        pos_deliminator[0] = value_str.indexOf("-");
        pos_deliminator[1] = value_str.indexOf("-", pos_deliminator[0] + 1);
        pos_deliminator[2] = value_str.indexOf("-", pos_deliminator[1] + 1);
        year = value_str.substring(0, pos_deliminator[0]);
        month = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]);
        day = value_str.substring(pos_deliminator[1] + 1, pos_deliminator[2]);
        DateTime tmp_time = hive.getDateTime();
        hive.setDateTime(DateTime(year.toInt(),
                                  month.toInt(),
                                  day.toInt(),
                                  tmp_time.hour(),
                                  tmp_time.minute(),
                                  tmp_time.second()));
      }
      if (pCharacteristic->getUUID().equals(BLE_PASSWORD_UUID))
      {
        hive.setblepassword(value_str.toInt(), true);
        hiveServer->getAdvertising()->stop();
        hiveServer->getAdvertising()->start();
        ESP.restart();
      }
      if (pCharacteristic->getUUID().equals(PID_COEFF_UUID))
      {
        String kp, ki, kd;
        pos_deliminator[0] = value_str.indexOf(",");
        pos_deliminator[1] = value_str.indexOf(",", pos_deliminator[0] + 1);
        pos_deliminator[2] = value_str.indexOf(",", pos_deliminator[1] + 1);
        kp = value_str.substring(0, pos_deliminator[0]);
        ki = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]);
        kd = value_str.substring(pos_deliminator[1] + 1, pos_deliminator[2]);
        hive.setPID(kp.toDouble(), ki.toDouble(), kd.toDouble(), true);
      }
      if (pCharacteristic->getUUID().equals(FANMODE_UUID))
      {
        value_str.toUpperCase();
        if(value_str=="AUTO")
          hive.setFanMode(FANAUTO);
        else if(value_str=="OFF")
          hive.setFanMode(FANOFF);
        else if(value_str=="ON")
          hive.setFanMode(FANON);
      }
      if (pCharacteristic->getUUID().equals(HEATERMODE_UUID))
      {
        value_str.toUpperCase();
        if(value_str=="AUTO")
          hive.setHeaterMode(HEATERAUTO);
        else if(value_str=="OFF")
          hive.setHeaterMode(HEATEROFF);
        else if(value_str=="ON")
          hive.setHeaterMode(HEATERON);
      }      
      if (pCharacteristic->getUUID().equals(AUTO_PROG_UUID))
      {
        AUTOProg_t prog;
        pos_deliminator[0] = value_str.indexOf(",");
        pos_deliminator[1] = value_str.indexOf(",", pos_deliminator[0] + 1);
        pos_deliminator[2] = value_str.indexOf(",", pos_deliminator[1] + 1);
        pos_deliminator[3] = value_str.indexOf(",", pos_deliminator[2] + 1);
        pos_deliminator[4] = value_str.indexOf(",", pos_deliminator[3] + 1);

        prog.temperature=value_str.substring(0, pos_deliminator[0]).toDouble();
        prog.fanhumidity = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]).toDouble();
        prog.fantemperature = value_str.substring(pos_deliminator[1] + 1, pos_deliminator[2]).toDouble();
        prog.fanhysthumidity = value_str.substring(pos_deliminator[2] + 1, pos_deliminator[3]).toDouble();
        prog.fanhysttemperature = value_str.substring(pos_deliminator[3] + 1, pos_deliminator[4]).toDouble();
        hive.setAutoProg(prog, true);
      }
      if (pCharacteristic->getUUID().equals(STRILPROG_UUID))
      {
        STERILProg_t prog;
        pos_deliminator[0] = value_str.indexOf(",");
        pos_deliminator[1] = value_str.indexOf(":", pos_deliminator[0] + 1);
        pos_deliminator[2] = value_str.indexOf(":", pos_deliminator[1] + 1);
        pos_deliminator[3] = value_str.indexOf(":", pos_deliminator[2] + 1);


        prog.temperature=value_str.substring(0, pos_deliminator[0]).toDouble();
        prog.duration = TimeSpan(0,
                                value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]).toInt(),
                                value_str.substring(pos_deliminator[1] + 1, pos_deliminator[2]).toInt(),
                                value_str.substring(pos_deliminator[2] + 1, pos_deliminator[3]).toInt()
                                ); 
        hive.setSterilProg(prog, true);
      }
      if (pCharacteristic->getUUID().equals(HIVE_STATE_UUID))
      {
        value_str.toUpperCase();
        if(value_str.equals("AUTO"))
          hive.setHiveState(HIVEAUTO);
        else if(value_str.equals("STERIL"))
          hive.setHiveState(HIVESTERIL);
        else if(value_str.equals("OFF"))
          hive.setHiveState(HIVEOFF);
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
#define READONLY_HiveWeight           7
#define READONLY_HeaterCurrent        8
#define READONLY_HeaterAverageCurrent 9
#define READONLY_FanStatus            10
#define READONLY_RTCStatus            11
#define READONLY_CheckHeater          12
#define READONLY_CheckFan             13
#define READONLY_Checksht20Inside     14
#define READONLY_Checksht20Outside    15
#define READONLY_CheckScale           16
#define READONLY_Sun                  17
#define READONLY_SDCARD               18

#define SERIAL_NUMBER_UUID      BLEUUID("734447d3-2cc6-4c0d-8eb6-ad942a931d01")
#define SOFTWARE_VERSION_UUID   BLEUUID("a628a3d6-51b4-44c2-b2d1-37c9b8be6972")
#define HARDWARE_VERSION_UUID   BLEUUID("00ce255b-d0ca-42ec-8ff2-7df81cfa3f52")
#define TEMP_INSIDE_UUID        BLEUUID("735d97aa-84e4-49b8-9c1c-8f9c47f0045f")
#define HUM_INSIDE_UUID         BLEUUID("c8abadb3-d645-4b16-a851-d113857bce4e")
#define TEMP_OUTSIDE_UUID       BLEUUID("9a170546-0e58-433d-8057-8478b95c0a18")
#define HUM_OUTSIDE_UUID        BLEUUID("fdda7dfd-79fa-41b6-80d8-ef01f6007360")
#define HIVE_WEIGHT_UUID        BLEUUID("92ae03b0-87dc-482d-8322-5ff2d12b6fe7")
#define HEATER_CURRENT_UUID     BLEUUID("94981336-ac09-48c7-8a62-86ba780bd734")
#define HEATER_AVG_CURRENT_UUID BLEUUID("fbf5e2d6-9d02-42c2-a33c-0b5bd6c22727")
#define FAN_STATUS_UUID         BLEUUID("b713687e-b84d-4ab8-b94b-af04c7b7b0ed")
#define RTC_STATUS_UUID         BLEUUID("edc7e652-5627-42e1-b9aa-68db0ed33568")
#define HIVE_CHECKHEATER_UUID       BLEUUID("12cf5269-61e8-4201-a165-a5adeae5f83f")
#define HIVE_CHECKFAN_UUID          BLEUUID("f4fb3847-3532-4ad3-a58d-14b991b71fdd")
#define HIVE_CHECKST20INSIDE_UUID   BLEUUID("e02f6d38-6c4a-4389-8c9c-e980d522a491")
#define HIVE_CHECKST20OUTSIDE_UUID  BLEUUID("85d9ed33-04a8-4b97-ac32-1a046ee871bc")
#define HIVE_CHECKSCALE_UUID         BLEUUID("4291737c-4e6e-481c-b0c7-e55fa3084414")
#define SUN_TIME_UUID               BLEUUID("ededa9b6-a8f3-4f1b-8d20-bc3903499a91")
#define SDCARD_UUID                 BLEUUID("d5cc63cf-0f29-4fef-bf2c-94334d1a7aa3")
/**
 * @brief Readonly Charactristic Callbacks
 *
 */
class readonlyCharactristicCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharactristic)
  {
    String strTemp;
    if (pCharactristic->getUUID().equals(SERIAL_NUMBER_UUID))
    {
      pCharactristic->setValue(SerialNumberString);
    }
    if (pCharactristic->getUUID().equals(SOFTWARE_VERSION_UUID))
    {
      pCharactristic->setValue(SoftwareVersionString);
    }
    if (pCharactristic->getUUID().equals(HARDWARE_VERSION_UUID))
    {
      pCharactristic->setValue(HardwareVersionString);
    }
    if (pCharactristic->getUUID().equals(TEMP_INSIDE_UUID))
    {
      strTemp = (hive.checkSensorInside()==CONNECTOK) ? String(hive.getInsideTemperature(),1) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HUM_INSIDE_UUID))
    {
      strTemp = (hive.checkSensorInside()==CONNECTOK) ? String(hive.getInsideHumidity(),0) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(TEMP_OUTSIDE_UUID))
    {
      strTemp = (hive.checkSensorOutside()==CONNECTOK) ? String(hive.getOutsideTemperature(),1) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HUM_OUTSIDE_UUID))
    {
      strTemp = (hive.checkSensorOutside()==CONNECTOK) ? String(hive.getOutsideHumidity(),0) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_WEIGHT_UUID))
    {
      strTemp = (hive.checkSensorScale()==CONNECTOK) ? String(hive.getHiveWeight()) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HEATER_CURRENT_UUID))
    {
      strTemp = String(hive.getHeaterCurrent());
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HEATER_AVG_CURRENT_UUID))
    {
      strTemp = String(hive.getHeaterAverageCurrent());
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(FAN_STATUS_UUID))
    {
      strTemp = (hive.getFanStatus() ? "ON" : "OFF");
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(RTC_STATUS_UUID))
    {
      strTemp = hive.getRTCStatus() ? "Adjust" : "Correct";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_CHECKHEATER_UUID))
    {
        strTemp=(hive.checkHeater()==MODEFAIL?"BROKEN":"HEALTHY");
        pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_CHECKFAN_UUID))
    {
        strTemp=(hive.checkFan()==MODEFAIL?"BROKEN":"HEALTHY");
        pCharactristic->setValue(strTemp.c_str());
    }    
    if (pCharactristic->getUUID().equals(HIVE_CHECKST20INSIDE_UUID))
    {
      _sensorstate_t st=hive.checkSensorInside();
        if(st==DISCONNECT)
        {
          strTemp="Disconnect";
        }
        else if(st==CONNECTOK)
        {
          strTemp="Connect OK";
        }
        else 
        {
          strTemp="Connect Fail";
        }        
        pCharactristic->setValue(strTemp.c_str());
    }        
    if (pCharactristic->getUUID().equals(HIVE_CHECKST20OUTSIDE_UUID))
    {
        _sensorstate_t st=hive.checkSensorOutside();
        if(st==DISCONNECT)
        {
          strTemp="Disconnect";
        }
        else if(st==CONNECTOK)
        {
          strTemp="Connect OK";
        }
        else 
        {
          strTemp="Connect Fail";
        }        
        pCharactristic->setValue(strTemp.c_str());
    }  
    if (pCharactristic->getUUID().equals(HIVE_CHECKSCALE_UUID))
    {
        _sensorstate_t st=hive.checkSensorScale();
        if(st==DISCONNECT)
        {
          strTemp="Disconnect";
        }
        else if(st==CONNECTOK)
        {
          strTemp="Connect OK";
        }
        else 
        {
          strTemp="Connect Fail";
        }        
        pCharactristic->setValue(strTemp.c_str());
    }           
    if (pCharactristic->getUUID().equals(SUN_TIME_UUID))
    {
      DateTime sr=hive.getSunrise();
      DateTime nn=hive.getNoon();
      DateTime ss=hive.getSunset();
      strTemp=String(sr.hour())+":"+
              String(sr.minute())+":"+
              String(sr.second())+","+
              String(nn.hour())+":"+
              String(nn.minute())+":"+
              String(nn.second())+","+ 
              String(ss.hour())+":"+
              String(ss.minute())+":"+
              String(ss.second());                          
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(SDCARD_UUID))
    {
      if(SD.cardType()==CARD_NONE)
      {
        strTemp=="---/---";
      }
      else
      {
        uint64_t cardSize = SD.totalBytes() / (1024 * 1024);
        uint64_t cardfree=SD.usedBytes() / (1024 * 1024);
        strTemp=String((uint16_t)cardfree)+"/"+String((uint16_t)cardSize);
      }
      pCharactristic->setValue(strTemp.c_str());

    }

  };
};

void bleInit();
void bleNotify(uint8_t code);
#endif