#include "UserBLE.h"
BLEServer *hiveServer;
BLEService *readwriteService;
BLEService *readonlyService;
BLEAdvertising *hiveAdvertising;

/**
 * @brief readwrite Charactristics and Descriptions
 *
 */
BLECharacteristic readwriteCharactristics[READWRITE_CHARACTRISTIC_NUMBER] = {
    BLECharacteristic(POSITION_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(DEVICE_NAME_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(TIME_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(DATE_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(BLE_PASSWORD_UUID, BLECharacteristic::PROPERTY_WRITE),
    BLECharacteristic(PID_COEFF_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_AUTOPROG_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_STERILPROG_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_LOADDEFAULT_UUID, BLECharacteristic::PROPERTY_WRITE),
    BLECharacteristic(HIVE_HEATRSTATE_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_FANSTATE_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_STATE_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),         // Hive Operational State
};
BLEDescriptor *readwriteDescriptors[READWRITE_CHARACTRISTIC_NUMBER];
String readwriteDescriptorsValue[READWRITE_CHARACTRISTIC_NUMBER] = {
    "Hive Position",
    "Hive Name",
    "Hive Time",
    "Hive Date",
    "BLE Pair Password",
    "Hive PID Coeff",
    "Hive AutoProg",
    "Hive SterilProg",
    "Hive's Default",
    "Hive's HeatrState",
    "Hive's FanState",
    "Hive State"
    };
/**
 * @brief readonly Charactristics and Descriptions
 *
 */
BLECharacteristic readonlyCharactristics[READONLY_CHARACTRISTIC_NUMBER] = {
    BLECharacteristic(SERIAL_NUMBER_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),      // Serial Number String
    BLECharacteristic(SOFTWARE_VERSION_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),   // Software Version String
    BLECharacteristic(HARDWARE_VERSION_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),   // Hardware Version String
    BLECharacteristic(TEMP_INSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),        // Inside Temperature with Timestap:Temperature Measurement
    BLECharacteristic(HUM_INSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),         // Indide Humidity:Humidity
    BLECharacteristic(TEMP_OUTSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),       // Outside Temperature with Timestamp:Temperature Measurement
    BLECharacteristic(HUM_OUTSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),        // Outside Humidity:Humidity
    BLECharacteristic(HEATER_CURRENT_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),     // Hive Heater current : Current
    BLECharacteristic(HEATER_AVG_CURRENT_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Heater current avrage: Average Current
    BLECharacteristic(SENSOR_INSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),      // Hive Sensor inside State
    BLECharacteristic(SENSOR_OUTSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),     // Hive Sensor outside State
    BLECharacteristic(RTC_STATUS_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),         // Hive Diurnal Cycle Day/night
    BLECharacteristic(HIVE_CHECKHEATER_UUID, BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_CHECKFAN_UUID, BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_CHECKST20INSIDE_UUID, BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(HIVE_CHECKST20OUTSIDE_UUID, BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(SUN_TIME_UUID, BLECharacteristic::PROPERTY_NOTIFY |BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(SDCARD_UUID, BLECharacteristic::PROPERTY_NOTIFY |BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(PID_STATE_UUID, BLECharacteristic::PROPERTY_NOTIFY |BLECharacteristic::PROPERTY_READ),

};
BLEDescriptor *readonlyDescriptors[READONLY_CHARACTRISTIC_NUMBER];
String readonlyDescriptorsValue[READONLY_CHARACTRISTIC_NUMBER] = {
    "Serial number",
    "Software Version",
    "Hardware Version",
    "Inside Temperature",
    "Inside Humidity",
    "Outside Temperature",
    "Outside Humidity",
    "Heater current(mA)",
    "Heater Average current(mA)",
    "Inside Sensor Status",
    "Outside Sensor Status",
    "RTC Status",
    "Check Heater",
    "Check Fan",
    "Check sht20 inside",
    "Check sht20 outside",
    "Sunrise/Noon/Sunset",
    "SD MB(used/total)",
    "PID State"
};

void bleInit()
{
    for (uint8_t i = 0; i < READWRITE_CHARACTRISTIC_NUMBER; i++)
    {
        readwriteDescriptors[i] = new BLEDescriptor(BLEUUID((uint16_t)0x2903));
        readwriteDescriptors[i]->setValue(readwriteDescriptorsValue[i].c_str());
    }

    for (uint8_t i = 0; i < READONLY_CHARACTRISTIC_NUMBER; i++)
    {
        readonlyDescriptors[i] = new BLEDescriptor(BLEUUID((uint16_t)0x2903));
        readonlyDescriptors[i]->setValue(readonlyDescriptorsValue[i].c_str());
    }
    // Init Device BLE and Security
    BLEDevice::init(hive.gethiveName().c_str());
    BLEDevice::setMTU(517);
#if ENABLE_PAIR_ENCRYPTION
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new hiveSecurityCallbacks());
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setStaticPIN(hive.getblepassword());
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_NO_BOND);
    pSecurity->setCapability(ESP_IO_CAP_OUT);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
#endif

    // Create Server
    hiveServer = BLEDevice::createServer();
    hiveServer->setCallbacks(new hiveServerCallbacks());
    // Create Service ReadWrite
    readwriteService = hiveServer->createService(READWRITE_SERVICE_UUID, READWRITE_CHARACTRISTIC_NUMBER * 3 + 1);

    for (uint8_t i = 0; i < READWRITE_CHARACTRISTIC_NUMBER; i++)
    {
        readwriteService->addCharacteristic(&readwriteCharactristics[i]);
#if ENABLE_PAIR_ENCRYPTION
        readwriteCharactristics[i].setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
#endif
        readwriteCharactristics[i].setCallbacks(new readwriteCharactristicCallbacks());

        // Serial.println(readonlyDescriptorsValue[i].c_str());
#if ENABLE_PAIR_ENCRYPTION
        readwriteDescriptors[i].setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
#endif
        readwriteCharactristics[i].addDescriptor(readwriteDescriptors[i]);
    }

    // Create Service ReadOnly and charctristics
    // numHandles = (# of Characteristics)*2 + (# of Services) + (# of Characteristics with BLE2902)
    readonlyService = hiveServer->createService(READONLY_SERVICE_UUID, READONLY_CHARACTRISTIC_NUMBER * 3 + 1);
    for (uint8_t i = 0; i < READONLY_CHARACTRISTIC_NUMBER; i++)
    {
        readonlyService->addCharacteristic(&readonlyCharactristics[i]);
#if ENABLE_PAIR_ENCRYPTION
        readonlyCharactristics[i].setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
#endif
        readonlyCharactristics[i].setCallbacks(new readonlyCharactristicCallbacks());

        // Serial.println(readonlyDescriptorsValue[i].c_str());
#if ENABLE_PAIR_ENCRYPTION
        readonlyDescriptors[i].setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
#endif
        readonlyCharactristics[i].addDescriptor(readonlyDescriptors[i]);
    }
    /// Start Services
    readwriteService->start();
    readonlyService->start();
    // advertising the services
    hiveAdvertising = BLEDevice::getAdvertising();
    hiveAdvertising->addServiceUUID(READWRITE_SERVICE_UUID);
    hiveAdvertising->addServiceUUID(READONLY_SERVICE_UUID);
    hiveAdvertising->setScanResponse(true);
    hiveAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    hiveAdvertising->setMinPreferred(0x12);
    hiveServer->getAdvertising()->start(); // BLEDevice::startAdvertising();  rtcServer->startAdvertising();
}

void bleNotify(uint8_t code)
{
    String strTemp;
    if(hive.isDeviceConnected())
    {
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_SerialNumber)
        {
            strTemp = SerialNumberString;
            readonlyCharactristics[READONLY_SerialNumber].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_SerialNumber].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_Softwareversion)
        {
            strTemp = SoftwareVersionString;
            readonlyCharactristics[READONLY_Softwareversion].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_Softwareversion].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_Hardwareversion)
        {
            strTemp = HardwareVersionString;
            readonlyCharactristics[READONLY_Hardwareversion].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_Hardwareversion].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_InsideTemperature)
        {
            strTemp = hive.sht20Inside.isConnected() ? String(hive.getInsideTemperature(), 1) : "---";
            readonlyCharactristics[READONLY_InsideTemperature].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_InsideTemperature].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_InsideHumidity)
        {
            strTemp = hive.sht20Inside.isConnected() ? String(hive.getInsideHumidity(), 0) : "---";
            readonlyCharactristics[READONLY_InsideHumidity].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_InsideHumidity].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_OutsideTemperature)
        {
            strTemp = hive.sht20Outside.isConnected() ? String(hive.getOutsideTemperature(), 1) : "---";
            readonlyCharactristics[READONLY_OutsideTemperature].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_OutsideTemperature].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_OutsideHumidity)
        {
            strTemp = hive.sht20Outside.isConnected() ? String(hive.getOutsideHumidity(), 0) : "---";
            readonlyCharactristics[READONLY_OutsideHumidity].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_OutsideHumidity].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_HeaterCurrent)
        {
            strTemp = String(hive.getHeaterCurrent());
            readonlyCharactristics[READONLY_HeaterCurrent].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_HeaterCurrent].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_HeaterAverageCurrent)
        {
            strTemp = String(hive.getHeaterAverageCurrent());
            readonlyCharactristics[READONLY_HeaterAverageCurrent].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_HeaterAverageCurrent].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_InsideSensor)
        {
            strTemp = (hive.sht20Inside.isConnected() ? "Connect" : "Disconnect");
            readonlyCharactristics[READONLY_InsideSensor].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_InsideSensor].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_OutsideSensor)
        {
            strTemp = (hive.sht20Outside.isConnected() ? "Connect" : "Disconnect");
            readonlyCharactristics[READONLY_OutsideSensor].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_OutsideSensor].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_RTCStatus)
        {
            strTemp = (hive.getRTCStatus() ? "Adjust" : "Correct");
            readonlyCharactristics[READONLY_RTCStatus].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_RTCStatus].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_CheckHeater)
        {
            strTemp=(hive.checkHeater()==MODEFAIL?"FAIL":"OK");
            readonlyCharactristics[READONLY_CheckHeater].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_CheckHeater].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_CheckFan)
        {
            strTemp=(hive.checkFan()==MODEFAIL?"FAIL":"OK");
            readonlyCharactristics[READONLY_CheckFan].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_CheckFan].notify();
        }    
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_Checksht20Inside)
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
            readonlyCharactristics[READONLY_Checksht20Inside].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_Checksht20Inside].notify();
        }        
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_Checksht20Outside)
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
            readonlyCharactristics[READONLY_Checksht20Outside].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_Checksht20Outside].notify();
        }        
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_Sun)
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
            readonlyCharactristics[READONLY_Sun].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_Sun].notify();
        }
        if (code == READONLY_CHARACTRISTIC_NUMBER || code == READONLY_PIDState)
        {
            String strTemp=String(hive.getTargetTemperature())+","+
                            String(hive.getInsideTemperature())+" 'C,"+
                            String(hive.getHeater())+" ,"+
                            String(hive.getHeater()/51*20)+" %";

                        
            readonlyCharactristics[READONLY_PIDState].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_PIDState].notify();
        }        
        if (code==READONLY_SDCARD)
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
            readonlyCharactristics[READONLY_SDCARD].setValue(strTemp.c_str());
            readonlyCharactristics[READONLY_SDCARD].notify();
        }
    }
    // Serial.println(">>>>Initialize Notify");
}
