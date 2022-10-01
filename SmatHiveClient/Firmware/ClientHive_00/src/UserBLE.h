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
#define SoftwareVersionString "1.0"
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
#define READWRITE_CHARACTRISTIC_NUMBER 21

#define READWRITE_Position          0
#define READWRITE_DeviceName        1
#define READWRITE_Time              2
#define READWRITE_Date              3
#define READWRITE_BlePassword       4
#define READWRITE_PIDCoeff          5
#define READWRITE_LightLevel        6
#define READWRITE_Prog0             7
#define READWRITE_Prog1             8
#define READWRITE_Prog2             9
#define READWRITE_Prog3             10
#define READWRITE_Prog4             11
#define READWRITE_Prog5             12
#define READWRITE_Prog6             13
#define READWRITE_Prog7             14
#define READWRITE_Prog8             15
#define READWRITE_Prog9             16
#define READWRITE_Prog10            17
#define READWRITE_Prog11            18
#define READWRITE_Pump              19
#define READWRITE_HiveState         20

#define POSITION_UUID               BLEUUID("bcd72712-7705-49c5-b2a8-bbc0eb72a988")
#define DEVICE_NAME_UUID            BLEUUID("dc4a6c5f-8d4a-4e4a-9dbe-2f7aad811f80")
#define TIME_UUID                   BLEUUID("fb75341e-8ab1-4355-9b69-39471f6be25d")
#define DATE_UUID                   BLEUUID("ae928490-34aa-4e9c-be83-0ca678f5eda2")
#define BLE_PASSWORD_UUID           BLEUUID("f1120eac-310b-4ffe-93c3-1d8aba7c58f5")
#define PID_COEFF_UUID              BLEUUID("e887f503-36fd-463d-b4d6-77f024094347")
#define LIGHT_LEVEL_UUID            BLEUUID("dc59a1c7-6536-4cff-a24b-4e0c3a28108b")
#define HIVE_PROG0_UUID             BLEUUID("fdcbcf24-2aff-4ab4-a6bb-a656ce0dce0a")
#define HIVE_PROG1_UUID             BLEUUID("7b91fb80-a20d-4a97-b2ad-48728db985f8")
#define HIVE_PROG2_UUID             BLEUUID("03a51ab4-0b51-4db3-8783-f7e350c847a1")
#define HIVE_PROG3_UUID             BLEUUID("aa782ec0-67e4-41de-890b-5f8d08c3d40d")
#define HIVE_PROG4_UUID             BLEUUID("814c5513-465f-45ff-8b5b-8859ec9b14d8")
#define HIVE_PROG5_UUID             BLEUUID("f12703d3-3d10-4b3b-acc9-db696633ee55")
#define HIVE_PROG6_UUID             BLEUUID("a1a7eb92-db3a-4714-82fb-7f953ced4264")
#define HIVE_PROG7_UUID             BLEUUID("04231659-8ce4-4a5e-a246-af8fb502c4b8")
#define HIVE_PROG8_UUID             BLEUUID("3a1e22ec-3a2f-4a19-9758-2285f4001c22")
#define HIVE_PROG9_UUID             BLEUUID("688ee6e2-bcaf-4024-9bcb-cdd5950d3c72")
#define HIVE_PROG10_UUID            BLEUUID("6c3dd7e6-060d-4390-9158-59f61637f408")
#define HIVE_PROG11_UUID            BLEUUID("a4cae9a8-c0ae-4aef-bd17-7183f83f81c7")
#define HIVE_PUMP_UUID              BLEUUID("ed930b8d-6932-4e0a-962d-bbc2eba56d2c")
#define HIVE_STATE_UUID            BLEUUID("c89076e1-7f68-4f6d-8a55-c4956c8da8e1")



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
      // Serial.println("******" + tmp_str);
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
              // String(tmp_time.year())+"-"+
              // String(tmp_time.month())+"-"+
              // String(tmp_time.day());
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
      PID_Coeff_t tmp_pid =hive.getPID();
      tmp_str=String(tmp_pid.kp,2)+","+
              String(tmp_pid.ki,2)+","+
              String(tmp_pid.kd,2);
      pCharacteristic->setValue(tmp_str.c_str());
    }
    if (pCharacteristic->getUUID().equals(LIGHT_LEVEL_UUID))
    {
      LIGHT_Level_t tmp_light_level=hive.getLight();
      tmp_str=String(tmp_light_level.level)+","+
              String(tmp_light_level.hysteresis);
      pCharacteristic->setValue(tmp_str.c_str());

    }
    int numprog=HIVE_MAX_PROG;
    if (pCharacteristic->getUUID().equals(HIVE_PROG0_UUID)) numprog=0;
    if (pCharacteristic->getUUID().equals(HIVE_PROG1_UUID)) numprog=1;
    if (pCharacteristic->getUUID().equals(HIVE_PROG2_UUID)) numprog=2;
    if (pCharacteristic->getUUID().equals(HIVE_PROG3_UUID)) numprog=3;
    if (pCharacteristic->getUUID().equals(HIVE_PROG4_UUID)) numprog=4;
    if (pCharacteristic->getUUID().equals(HIVE_PROG5_UUID)) numprog=5;
    if (pCharacteristic->getUUID().equals(HIVE_PROG6_UUID)) numprog=6;
    if (pCharacteristic->getUUID().equals(HIVE_PROG7_UUID)) numprog=7;
    if (pCharacteristic->getUUID().equals(HIVE_PROG8_UUID)) numprog=8;
    if (pCharacteristic->getUUID().equals(HIVE_PROG9_UUID)) numprog=9;
    if (pCharacteristic->getUUID().equals(HIVE_PROG10_UUID)) numprog=10;
    if (pCharacteristic->getUUID().equals(HIVE_PROG11_UUID)) numprog=11;
    if(numprog<HIVE_MAX_PROG)
    {
      PROG_t tmp_prog=hive.getProg(numprog);
      tmp_str=String(tmp_prog.active)+","+
              String(tmp_prog.id)+","+
              String(tmp_prog.start.year())+"-"+String(tmp_prog.start.month())+"-"+String(tmp_prog.start.day())+","+
              String(tmp_prog.start.hour())+":"+String(tmp_prog.start.minute())+":"+String(tmp_prog.start.second())+","+              
              String(tmp_prog.stop.year())+"-"+String(tmp_prog.stop.month())+"-"+String(tmp_prog.stop.day())+","+
              String(tmp_prog.stop.hour())+":"+String(tmp_prog.stop.minute())+":"+String(tmp_prog.stop.second())+","+              
              String(tmp_prog.Sunrise.days())+":"+String(tmp_prog.Sunrise.hours())+":"+String(tmp_prog.Sunrise.minutes())+":"+String(tmp_prog.Sunrise.seconds())+","+              
              String(tmp_prog.Sunset.days())+":"+String(tmp_prog.Sunset.hours())+":"+String(tmp_prog.Sunset.minutes())+":"+String(tmp_prog.Sunset.seconds())+","+              
              String(tmp_prog.minNormalTemperatureDay)+","+
              String(tmp_prog.minNormalTemperatureNight)+","+              
              String(tmp_prog.dayTemperatureTarget)+","+ 
              String(tmp_prog.nightTemperatureTarget)+","+                           
              String(tmp_prog.emergencyTemperatureTarget)+","+                           
              String(tmp_prog.fanOnTemperature)+","+                           
              String(tmp_prog.fanOnHumidity)+","+                           
              String(tmp_prog.fanOnHysteresis)+","+                           
              String(tmp_prog.pumpOnDay)+","+                           
              String(tmp_prog.pumpOnNight)+","+                           
              String(tmp_prog.fanOnDay)+","+                           
              String(tmp_prog.fanOnNight)+","+                           
              String(tmp_prog.heaterOnDay)+","+                           
              String(tmp_prog.heaterOnNight);
             pCharacteristic->setValue(tmp_str.c_str());
    }    

    if (pCharacteristic->getUUID().equals(HIVE_PUMP_UUID))
    {
      PUMP_t p=hive.getPump();
      tmp_str=String(p.turnonlevel)+","+
              String(p.controlmode)+","+
              String(p.turnontime.days())+":"+
              String(p.turnontime.hours())+":"+ 
              String(p.turnontime.minutes())+":"+
              String(p.turnontime.seconds());
      pCharacteristic->setValue(tmp_str.c_str());
    }
    if (pCharacteristic->getUUID().equals(HIVE_STATE_UUID))
    {
      _state_t st=hive.getHiveState();
      if(st==HIVEAUTO)
        tmp_str = "AUTO";
      else if(st==HIVEEMERGENCY)
        tmp_str="EMERGENCY";
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
        // pos_deliminator[3] = value_str.indexOf("-", pos_deliminator[2] + 1);
        // pos_deliminator[4] = value_str.indexOf("-", pos_deliminator[3] + 1);
        // pos_deliminator[5] = value_str.indexOf("-", pos_deliminator[4] + 1);
        h = value_str.substring(0, pos_deliminator[0]);
        m = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]);
        s = value_str.substring(pos_deliminator[1] + 1, pos_deliminator[2]);
        // y = value_str.substring(pos_deliminator[2] + 1, pos_deliminator[3]);
        // mm = value_str.substring(pos_deliminator[3] + 1, pos_deliminator[4]);
        // d = value_str.substring(pos_deliminator[4] + 1, pos_deliminator[5]);
        DateTime tmp_time = hive.getDateTime();
        hive.setDateTime(DateTime(tmp_time.year(),
                                  tmp_time.month(),
                                  tmp_time.day(),
                                  h.toInt(),
                                  m.toInt(),
                                  s.toInt()));
        // DateTime tmp_time=DateTime(y.toInt(),mm.toInt(),d.toInt(),h.toInt(), m.toInt(), s.toInt());
        // hive.setDateTime(tmp_time);

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
      if (pCharacteristic->getUUID().equals(LIGHT_LEVEL_UUID))
      {
        String LightN, LightH;
        pos_deliminator[0] = value_str.indexOf(",");
        pos_deliminator[1] = value_str.indexOf(",", pos_deliminator[0] + 1);
        LightN = value_str.substring(0, pos_deliminator[0]);
        LightH = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]);
        hive.setLight(LightN.toInt(), LightH.toInt(), true);
      }
      if (pCharacteristic->getUUID().equals(HIVE_PROG0_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG1_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG2_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG3_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG4_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG5_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG6_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG7_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG8_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG9_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG10_UUID) ||
          pCharacteristic->getUUID().equals(HIVE_PROG11_UUID))
      {
        PROG_t prog;
        pos_deliminator[0] = value_str.indexOf(",");
        prog.active=(bool)value_str.substring(0, pos_deliminator[0]).toInt();
        if (prog.active)
        {
          String y,mt,d,h,mn,s;
          pos_deliminator[1] = value_str.indexOf(",", pos_deliminator[0] + 1);
          prog.id = value_str.substring(pos_deliminator[0] + 1, pos_deliminator[1]).toInt();

          pos_deliminator[2] = value_str.indexOf("-", pos_deliminator[1] + 1);
          pos_deliminator[3] = value_str.indexOf("-", pos_deliminator[2] + 1);
          pos_deliminator[4] = value_str.indexOf(",", pos_deliminator[3] + 1);
          pos_deliminator[5] = value_str.indexOf(":", pos_deliminator[4] + 1);
          pos_deliminator[6] = value_str.indexOf(":", pos_deliminator[5] + 1);
          pos_deliminator[7] = value_str.indexOf(",", pos_deliminator[6] + 1);
          y = value_str.substring(pos_deliminator[1] + 1, pos_deliminator[2]);
          mt = value_str.substring(pos_deliminator[2] + 1, pos_deliminator[3]);
          d = value_str.substring(pos_deliminator[3] + 1, pos_deliminator[4]);
          h = value_str.substring(pos_deliminator[4] + 1, pos_deliminator[5]);
          mn = value_str.substring(pos_deliminator[5] + 1, pos_deliminator[6]);
          s = value_str.substring(pos_deliminator[6] + 1, pos_deliminator[7]);
          prog.start=DateTime(y.toInt(),mt.toInt(),d.toInt(),h.toInt(),mn.toInt(),s.toInt());

          pos_deliminator[8] = value_str.indexOf("-", pos_deliminator[7] + 1);
          pos_deliminator[9] = value_str.indexOf("-", pos_deliminator[8] + 1);
          pos_deliminator[10] = value_str.indexOf(",", pos_deliminator[9] + 1);
          pos_deliminator[11] = value_str.indexOf(":", pos_deliminator[10] + 1);
          pos_deliminator[12] = value_str.indexOf(":", pos_deliminator[11] + 1);
          pos_deliminator[13] = value_str.indexOf(",", pos_deliminator[12] + 1);
          y = value_str.substring(pos_deliminator[7] + 1, pos_deliminator[8]);
          mt = value_str.substring(pos_deliminator[8] + 1, pos_deliminator[9]);
          d = value_str.substring(pos_deliminator[9] + 1, pos_deliminator[10]);
          h = value_str.substring(pos_deliminator[10] + 1, pos_deliminator[11]);
          mn = value_str.substring(pos_deliminator[11] + 1, pos_deliminator[12]);
          s = value_str.substring(pos_deliminator[12] + 1, pos_deliminator[13]);
          prog.stop=DateTime(y.toInt(),mt.toInt(),d.toInt(),h.toInt(),mn.toInt(),s.toInt());

          pos_deliminator[14] = value_str.indexOf(":", pos_deliminator[13] + 1);
          pos_deliminator[15] = value_str.indexOf(":", pos_deliminator[14] + 1);
          pos_deliminator[16] = value_str.indexOf(":", pos_deliminator[15] + 1);
          pos_deliminator[17] = value_str.indexOf(",", pos_deliminator[16] + 1);
          d = value_str.substring(pos_deliminator[13] + 1, pos_deliminator[14]);
          h = value_str.substring(pos_deliminator[14] + 1, pos_deliminator[15]);
          mn = value_str.substring(pos_deliminator[15] + 1, pos_deliminator[16]);
          s = value_str.substring(pos_deliminator[16] + 1, pos_deliminator[17]);
          prog.Sunrise=TimeSpan(d.toInt(),h.toInt(),mn.toInt(),s.toInt());

          pos_deliminator[18] = value_str.indexOf(":", pos_deliminator[17] + 1);
          pos_deliminator[19] = value_str.indexOf(":", pos_deliminator[18] + 1);
          pos_deliminator[20] = value_str.indexOf(":", pos_deliminator[19] + 1);
          pos_deliminator[21] = value_str.indexOf(",", pos_deliminator[20] + 1);
          d = value_str.substring(pos_deliminator[17] + 1, pos_deliminator[18]);
          h = value_str.substring(pos_deliminator[18] + 1, pos_deliminator[19]);
          mn = value_str.substring(pos_deliminator[19] + 1, pos_deliminator[20]);
          s = value_str.substring(pos_deliminator[20] + 1, pos_deliminator[21]);
          prog.Sunset=TimeSpan(d.toInt(),h.toInt(),mn.toInt(),s.toInt());

          pos_deliminator[22] = value_str.indexOf(",", pos_deliminator[21] + 1);
          prog.minNormalTemperatureDay = value_str.substring(pos_deliminator[21] + 1, pos_deliminator[22]).toDouble();

          pos_deliminator[23] = value_str.indexOf(",", pos_deliminator[22] + 1);
          prog.minNormalTemperatureNight = value_str.substring(pos_deliminator[22] + 1, pos_deliminator[23]).toDouble();

          pos_deliminator[24] = value_str.indexOf(",", pos_deliminator[23] + 1);
          prog.dayTemperatureTarget = value_str.substring(pos_deliminator[23] + 1, pos_deliminator[24]).toDouble();

          pos_deliminator[25] = value_str.indexOf(",", pos_deliminator[24] + 1);
          prog.nightTemperatureTarget = value_str.substring(pos_deliminator[24] + 1, pos_deliminator[25]).toDouble();

          pos_deliminator[26] = value_str.indexOf(",", pos_deliminator[25] + 1);
          prog.emergencyTemperatureTarget = value_str.substring(pos_deliminator[25] + 1, pos_deliminator[26]).toDouble();

          pos_deliminator[27] = value_str.indexOf(",", pos_deliminator[26] + 1);
          prog.fanOnTemperature = value_str.substring(pos_deliminator[26] + 1, pos_deliminator[27]).toDouble();

          pos_deliminator[28] = value_str.indexOf(",", pos_deliminator[27] + 1);
          prog.fanOnHumidity = value_str.substring(pos_deliminator[27] + 1, pos_deliminator[28]).toDouble();

          pos_deliminator[29] = value_str.indexOf(",", pos_deliminator[28] + 1);
          prog.fanOnHysteresis = value_str.substring(pos_deliminator[28] + 1, pos_deliminator[29]).toDouble();

          pos_deliminator[30] = value_str.indexOf(",", pos_deliminator[29] + 1);
          prog.pumpOnDay = (bool)(value_str.substring(pos_deliminator[29] + 1, pos_deliminator[30]).toInt());

          pos_deliminator[31] = value_str.indexOf(",", pos_deliminator[30] + 1);
          prog.pumpOnNight = (bool)(value_str.substring(pos_deliminator[30] + 1, pos_deliminator[31]).toInt());

          pos_deliminator[32] = value_str.indexOf(",", pos_deliminator[31] + 1);
          prog.fanOnDay = (bool)(value_str.substring(pos_deliminator[31] + 1, pos_deliminator[32]).toInt());

          pos_deliminator[33] = value_str.indexOf(",", pos_deliminator[32] + 1);
          prog.fanOnNight = (bool)(value_str.substring(pos_deliminator[32] + 1, pos_deliminator[33]).toInt());

          pos_deliminator[34] = value_str.indexOf(",", pos_deliminator[33] + 1);
          prog.heaterOnDay = (bool)(value_str.substring(pos_deliminator[33] + 1, pos_deliminator[34]).toInt());

          pos_deliminator[35] = value_str.indexOf(",", pos_deliminator[34] + 1);
          prog.heaterOnNight = (bool)(value_str.substring(pos_deliminator[34] + 1, pos_deliminator[35]).toInt());

          hive.setProg(prog, true);
        }
      }
      if (pCharacteristic->getUUID().equals(HIVE_PUMP_UUID))
      {
        PUMP_t p;
        pos_deliminator[0] = value_str.indexOf(",");
        pos_deliminator[1] = value_str.indexOf(",", pos_deliminator[0] + 1);
        pos_deliminator[2] = value_str.indexOf(":", pos_deliminator[1] + 1);
        pos_deliminator[3] = value_str.indexOf(":", pos_deliminator[2] + 1);
        pos_deliminator[4] = value_str.indexOf(":", pos_deliminator[3] + 1);
        pos_deliminator[5] = value_str.indexOf(",", pos_deliminator[4] + 1);                        
        p.turnonlevel=(_feeding_t)(value_str.substring(0,pos_deliminator[0]).toFloat());
        p.controlmode=(_pumpcontrol_t)(value_str.substring(pos_deliminator[0]+1,pos_deliminator[1]).toInt());
        p.turnontime=TimeSpan(
                          value_str.substring(pos_deliminator[1]+1,pos_deliminator[2]).toInt(),
                          value_str.substring(pos_deliminator[2]+1,pos_deliminator[3]).toInt(),        
                          value_str.substring(pos_deliminator[3]+1,pos_deliminator[4]).toInt(),        
                          value_str.substring(pos_deliminator[4]+1,pos_deliminator[5]).toInt()        
        );
        hive.setPump(p,true);
      }
      if (pCharacteristic->getUUID().equals(HIVE_STATE_UUID))
      {
        value_str.toUpperCase();
        if(value_str.equals("AUTO"))
          hive.setHiveState(HIVEAUTO);
        else if(value_str.equals("STERIL"))
          hive.setHiveState(HIVESTERIL);
        else if(value_str.equals("EMERGENCY"))
          hive.setHiveState(HIVEEMERGENCY);
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
#define READONLY_CHARACTRISTIC_NUMBER 26

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
#define READONLY_HeaterCurrent        11
#define READONLY_HeaterAverageCurrent 12
#define READONLY_FanStatus            13
#define READONLY_PumpStatus           14
#define READONLY_InsideSensor         15
#define READONLY_OutsideSensor        16
#define READONLY_HiveDiurnal          17
#define READONLY_RTCStatus            18
#define READONLY_CheckHeater          19
#define READONLY_CheckFan             20
#define READONLY_CheckPump            21
#define READONLY_Checksht20Inside     22
#define READONLY_Checksht20Outside    23
#define READONLY_Sun                  24
#define READONLY_SDCARD               25

#define SERIAL_NUMBER_UUID      BLEUUID("734447d3-2cc6-4c0d-8eb6-ad942a931d01")
#define SOFTWARE_VERSION_UUID   BLEUUID("a628a3d6-51b4-44c2-b2d1-37c9b8be6972")
#define HARDWARE_VERSION_UUID   BLEUUID("00ce255b-d0ca-42ec-8ff2-7df81cfa3f52")
#define TEMP_INSIDE_UUID        BLEUUID("735d97aa-84e4-49b8-9c1c-8f9c47f0045f")
#define HUM_INSIDE_UUID         BLEUUID("c8abadb3-d645-4b16-a851-d113857bce4e")
#define TEMP_OUTSIDE_UUID       BLEUUID("9a170546-0e58-433d-8057-8478b95c0a18")
#define HUM_OUTSIDE_UUID        BLEUUID("fdda7dfd-79fa-41b6-80d8-ef01f6007360")
#define LIGHTNESS_UUID          BLEUUID("bcd72712-7705-49c5-b2a8-bbc0eb72a988")
#define DOOR_STATE_UUID         BLEUUID("29012989-0121-4fdb-b4b6-b1f62ffa15d1")
#define HIVE_WEIGHT_UUID        BLEUUID("92ae03b0-87dc-482d-8322-5ff2d12b6fe7")
#define HIVE_FEEDING_LEVEL_UUID BLEUUID("262ceb5c-19a1-4167-88bb-3a59f95b24ce")
#define HEATER_CURRENT_UUID     BLEUUID("94981336-ac09-48c7-8a62-86ba780bd734")
#define HEATER_AVG_CURRENT_UUID BLEUUID("fbf5e2d6-9d02-42c2-a33c-0b5bd6c22727")
#define FAN_STATUS_UUID         BLEUUID("b713687e-b84d-4ab8-b94b-af04c7b7b0ed")
#define PUMP_STATUS_UUID        BLEUUID("225f1fd0-10ba-46a9-859f-68e51d9be8f1")
#define SENSOR_INSIDE_UUID      BLEUUID("4dff8580-be85-45b8-bb8c-8d0477a4d3be")
#define SENSOR_OUTSIDE_UUID     BLEUUID("659cbe7c-0017-4625-987d-4c8c7a44a30a")
#define HIVE_DIURNAL_UUID       BLEUUID("aebe1a02-ca7b-4538-a540-ac35cdd1c8e9")
#define RTC_STATUS_UUID         BLEUUID("edc7e652-5627-42e1-b9aa-68db0ed33568")
#define HIVE_CHECKHEATER_UUID       BLEUUID("12cf5269-61e8-4201-a165-a5adeae5f83f")
#define HIVE_CHECKFAN_UUID          BLEUUID("f4fb3847-3532-4ad3-a58d-14b991b71fdd")
#define HIVE_CHECKPUMP_UUID         BLEUUID("4291737c-4e6e-481c-b0c7-e55fa3084414")
#define HIVE_CHECKST20INSIDE_UUID   BLEUUID("e02f6d38-6c4a-4389-8c9c-e980d522a491")
#define HIVE_CHECKST20OUTSIDE_UUID  BLEUUID("85d9ed33-04a8-4b97-ac32-1a046ee871bc")
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
      strTemp = hive.sht20Inside.isConnected() ? String(hive.sht20Inside.getTemperature(),1) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HUM_INSIDE_UUID))
    {
      strTemp = hive.sht20Inside.isConnected() ? String(hive.sht20Inside.getHumidity(),0) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(TEMP_OUTSIDE_UUID))
    {
      strTemp = hive.sht20Outside.isConnected() ? String(hive.sht20Outside.getTemperature(),1) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HUM_OUTSIDE_UUID))
    {
      strTemp = hive.sht20Outside.isConnected() ? String(hive.sht20Outside.getHumidity(),0) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(LIGHTNESS_UUID))
    {
      strTemp = String(hive.readPhotoCell());
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(DOOR_STATE_UUID))
    {
      strTemp = (hive.readDoorState()) ? "open" : "close";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_WEIGHT_UUID))
    {
      strTemp = (hive.getHiveWeightState()) ? String(hive.readWeight()) : "---";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_FEEDING_LEVEL_UUID))
    {
      int level = hive.getHiveFeedingLevel();
      if (level == 0)
      {
        strTemp = "Empty";
      }
      else if (level == 1)
      {
        strTemp = "Half";
      }
      else
      {
        strTemp = "Full";
      }

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

    if (pCharactristic->getUUID().equals(PUMP_STATUS_UUID))
    {
      strTemp = (hive.getPumpStatus() ? "ON" : "OFF");
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(SENSOR_INSIDE_UUID))
    {
      strTemp = (hive.sht20Inside.isConnected() ? "Connect" : "Disconnect");
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(SENSOR_OUTSIDE_UUID))
    {
      strTemp = (hive.sht20Outside.isConnected() ? "Connect" : "Disconnect");
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_DIURNAL_UUID))
    {
      strTemp = (hive.getDiurnalStatus() == DAY ? "DAY" : "NIGHT");
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(RTC_STATUS_UUID))
    {
      strTemp = hive.getRTCStatus() ? "Adjust" : "Correct";
      pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_CHECKHEATER_UUID))
    {
        strTemp=(hive.checkHeater()==MODEFAIL?"FAIL":"OK");
        pCharactristic->setValue(strTemp.c_str());
    }
    if (pCharactristic->getUUID().equals(HIVE_CHECKFAN_UUID))
    {
        strTemp=(hive.checkFan()==MODEFAIL?"FAIL":"OK");
        pCharactristic->setValue(strTemp.c_str());
    }    
    if (pCharactristic->getUUID().equals(HIVE_CHECKPUMP_UUID))
    {
        strTemp=(hive.checkPump()==MODEFAIL?"FAIL":"OK");
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