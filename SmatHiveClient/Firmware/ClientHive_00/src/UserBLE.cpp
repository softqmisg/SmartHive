#include "UserBLE.h"
BLEServer *hiveServer;
BLEService *readwriteService;
BLEService *readonlyService;
BLEAdvertising *hiveAdvertising;
/**
 * @brief readwrite Charactristics and Descriptions
 * 
 */
BLECharacteristic readwriteCharactristics[READWRITE_CHARACTRISTIC_NUMBER]={
    BLECharacteristic(POSITION_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(DEVICE_NAME_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(TIME_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(DATE_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(BLE_PASSWORD_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
    BLECharacteristic(PID_COEFF_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),    
    BLECharacteristic(LIGHT_LEVEL_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ),
};
BLEDescriptor *readwriteDescriptors[READWRITE_CHARACTRISTIC_NUMBER];
String readwriteDescriptorsValue[READWRITE_CHARACTRISTIC_NUMBER]={
    "Hive Position",
    "Hive Name",
    "Hive Time",
    "Hive Date",
    "BLE Pair Password",
    "Hive PID Coeff",
    "Light Level",
};
/**
 * @brief readonly Charactristics and Descriptions
 * 
 */
BLECharacteristic readonlyCharactristics[READONLY_CHARACTRISTIC_NUMBER] = {
    BLECharacteristic(SERIAL_NUMBER_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Serial Number String
    BLECharacteristic(SOFTWARE_VERSION_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Software Version String
    BLECharacteristic(HARDWARE_VERSION_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Hardware Version String
    BLECharacteristic(TEMP_INSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Inside Temperature with Timestap:Temperature Measurement
    BLECharacteristic(HUM_INSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Indide Humidity:Humidity
    BLECharacteristic(TEMP_OUTSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Outside Temperature with Timestamp:Temperature Measurement
    BLECharacteristic(HUM_OUTSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Outside Humidity:Humidity
    BLECharacteristic(LIGHTNESS_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Outside Lightness:Perceived Lightness
    BLECharacteristic(DOOR_STATE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Door State:
    BLECharacteristic(HIVE_WEIGHT_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Hive Weight:Weight
    BLECharacteristic(HIVE_FEEDING_LEVEL, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Hive feeding level:Generic Level
    BLECharacteristic(AVERAGE_CURRENT_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ),                       // Hive Heater current avrage: Average Current
    BLECharacteristic(FAN_STATUS_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Fan state
    BLECharacteristic(PUMP_STATUS_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Pump State
    BLECharacteristic(SENSOR_INSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Sensor inside State
    BLECharacteristic(SENSOR_OUTSIDE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Sensor outside State
    BLECharacteristic(HIVE_DIURNAL_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Diurnal Cycle Day/night
    BLECharacteristic(RTC_STATUS_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Diurnal Cycle Day/night
    BLECharacteristic(HIVE_STATE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ), // Hive Operational State

};
BLEDescriptor *readonlyDescriptors[READONLY_CHARACTRISTIC_NUMBER];
String readonlyDescriptorsValue[READONLY_CHARACTRISTIC_NUMBER]={
    "Serial number",
    "Software Version",
    "Hardware Version",
    "Inside Temperature",
    "Inside Humidity",
    "Outside Temperature",
    "Outside Humidity",
    "Outside Light",
    "Hive Door State",
    "Hive's Weight",
    "Level of bee feeding",
    "Heater Average current",
    "Fan Status",
    "Pump Status",
    "Inside Sensor Status",
    "Outside Sensor Status",
    "Hive Diurnal Status",
    "RTC Status",
    "Hive State",
};

void bleInit()
{
    for(uint8_t i=0;i<READWRITE_CHARACTRISTIC_NUMBER;i++)
    {
        readwriteDescriptors[i]=new BLEDescriptor(BLEUUID((uint16_t)0x2903));
        readwriteDescriptors[i]->setValue(readwriteDescriptorsValue[i].c_str());
    }

    for(uint8_t i=0;i<READONLY_CHARACTRISTIC_NUMBER;i++)
    {
        readonlyDescriptors[i]=new BLEDescriptor(BLEUUID((uint16_t)0x2903));
        readonlyDescriptors[i]->setValue(readonlyDescriptorsValue[i].c_str());
    }
    //Init Device BLE and Security
    BLEDevice::init(hive.gethiveName().c_str());
    BLEDevice::setMTU(517);
#if ENABLE_PAIR_ENCRYPTION
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new hiveSecurityCallbacks());
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setStaticPIN(bleStaticPIN);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_NO_BOND);
    pSecurity->setCapability(ESP_IO_CAP_OUT);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
#endif
    
    //Create Server
    hiveServer = BLEDevice::createServer();
    hiveServer->setCallbacks(new hiveServerCallbacks());
    //Create Service ReadWrite
    readwriteService=hiveServer->createService(READWRITE_SERVICE_UUID,READWRITE_CHARACTRISTIC_NUMBER*3+1);
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

    //Create Service ReadOnly and charctristics
    //numHandles = (# of Characteristics)*2 + (# of Services) + (# of Characteristics with BLE2902)
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
    ///Start Services
    readwriteService->start();
    readonlyService->start();
    //advertising the services
    hiveAdvertising = BLEDevice::getAdvertising();
    hiveAdvertising->addServiceUUID(READWRITE_SERVICE_UUID);
    hiveAdvertising->addServiceUUID(READONLY_SERVICE_UUID);
    hiveAdvertising->setScanResponse(true);
    hiveAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    hiveAdvertising->setMinPreferred(0x12);
    hiveServer->getAdvertising()->start(); // BLEDevice::startAdvertising();  rtcServer->startAdvertising();
}

void bleNotifyInitializeData()
{
    String strTemp;
    strTemp=SerialNumberString;
    readonlyCharactristics[READONLY_SerialNumber].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_SerialNumber].notify();

    strTemp=SoftwareVersionString;
    readonlyCharactristics[READONLY_Softwareversion].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_Softwareversion].notify();

    strTemp=HardwareVersionString;
    readonlyCharactristics[READONLY_Hardwareversion].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_Hardwareversion].notify();

    strTemp=hive.sht20Inside.isConnected()?String(hive.sht20Inside.getTemperature()):"---";
    readonlyCharactristics[READONLY_InsideTemperature].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_InsideTemperature].notify();

    strTemp=hive.sht20Inside.isConnected()?String(hive.sht20Inside.getHumidity()):"---";
    readonlyCharactristics[READONLY_InsideHumidity].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_InsideHumidity].notify();


    strTemp=hive.sht20Outside.isConnected()?String(hive.sht20Outside.getTemperature()):"---";
    readonlyCharactristics[READONLY_OutsideTemperature].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_OutsideTemperature].notify();

    strTemp=hive.sht20Outside.isConnected()?String(hive.sht20Outside.getHumidity()):"---";
    readonlyCharactristics[READONLY_OutsideHumidity].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_OutsideHumidity].notify();

    strTemp=String(hive.readPhotoCell());
    readonlyCharactristics[READONLY_OutsideLight].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_OutsideLight].notify();

    strTemp=(hive.readDoorState())?"open":"close";
    readonlyCharactristics[READONLY_DoorState].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_DoorState].notify();

    strTemp=(hive.getHiveWeightState())?String(hive.readWeight()):"---";
    readonlyCharactristics[READONLY_HiveWeight].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_HiveWeight].notify();

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
    readonlyCharactristics[READONLY_LevelFeeding].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_LevelFeeding].notify();

    strTemp=String(hive.getHeaterAverageCurrent());
    readonlyCharactristics[READONLY_AverageCurrent].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_AverageCurrent].notify();

    strTemp=(hive.getFanStatus()?"OK":"FAIL");
    readonlyCharactristics[READONLY_FanStatus].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_FanStatus].notify();

    strTemp=(hive.getPumpStatus()?"ON":"OFF");
    readonlyCharactristics[READONLY_PumpStatus].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_PumpStatus].notify();

    strTemp=(hive.sht20Inside.isConnected()?"OK":"FAIL");
    readonlyCharactristics[READONLY_InsideSensor].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_InsideSensor].notify();

    strTemp=(hive.sht20Outside.isConnected()?"OK":"FAIL");
    readonlyCharactristics[READONLY_OutsideSensor].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_OutsideSensor].notify();

    strTemp=(hive.getDiurnalStatus()==DAY ?"DAY":"NIGHT");
    readonlyCharactristics[READONLY_HiveDiurnal].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_HiveDiurnal].notify();

    strTemp=(hive.getRTCStatus()?"Adjust":"Correct");
    readonlyCharactristics[READONLY_RTCStatus].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_RTCStatus].notify();

    strTemp=(hive.getHiveState()==NORMAL?"NORMAL":"EMERGENCY");
    readonlyCharactristics[READONLY_HiveState].setValue(strTemp.c_str());
    readonlyCharactristics[READONLY_HiveState].notify();

    Serial.println(">>>>Initialize Notify");
}
