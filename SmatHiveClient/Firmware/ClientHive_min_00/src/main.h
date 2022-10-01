#ifndef __MAIN_H__
#define __MAIN_H__
    #define ESP32_GPIO0                     0
    #define ESP32_GPIO2_WATERLEVEL          2
    #define ESP32_GPIO4_PUMP                4
    #define ESP32_GPIO5_LED0                5
    #define ESP32_GPIO12_SDMISO             12
    #define ESP32_GPIO13_SDMOSI             13
    #define ESP32_GPIO14_SDSCK              14
    #define ESP32_GPIO15_KEYDOWN            15
    #define ESP32_GPIO16_SDA2               16
    #define ESP32_GPIO17_SCL2               17
    #define ESP32_GPIO18_FAN                18
    #define ESP32_GPIO19_THERMAL            19
    #define ESP32_GPIO21_SDA1               21
    #define ESP32_GPIO22_SCL1               22
    #define ESP32_GPIO23_REED               23
    #define ESP32_GPIO25_SDPRESENT          25
    #define ESP32_GPIO26_SDCS               26
    #define ESP32_GPIO27_RTC_SDA            27
    #define ESP32_GPIO32_RTC_SCL            32
    #define ESP32_GPIO33_FANFEEDBACK       33
    #define ESP32_GPIO34_HX711_CLK          34
    #define ESP32_GPIO35_HX711_DT           35
    #define ESP32_GPIO36_THERMALFEEDBACK    36
    #define ESP32_GPIO39_PHOTOCELL          39
    //////////////////////////////////////////////////////
    #define EEPROM_SZ       2048
    ///////////////////////////////////////////////////////
    #define EE_ADD_FIRSTTIME        0
    #define EE_SZ_FIRSTTME          1

    #define EE_ADD_LAT              EE_ADD_FIRSTTIME+EE_SZ_FIRSTTME
    #define EE_SZ_LAT               8

    #define EE_ADD_LNG              EE_ADD_LAT+EE_SZ_LAT
    #define EE_SZ_LNG               8

    #define EE_ADD_HIVE_NAME        EE_ADD_LNG+EE_SZ_LNG
    #define EE_SZ_HIVE_NAME         15
    
    #define EE_ADD_BLEPASS          EE_ADD_HIVE_NAME+EE_SZ_HIVE_NAME
    #define EE_SZ_BLEPASS           8

    #define EE_ADD_PIDCOEFF          EE_ADD_BLEPASS+EE_SZ_BLEPASS
    #define EE_SZ_PIDCOEFF           24

    #define EE_ADD_AUTOPROG         EE_ADD_PIDCOEFF+EE_SZ_PIDCOEFF
    #define EE_SZ_AUTOPROG         40

    #define EE_ADD_STERILPROG       EE_ADD_AUTOPROG+EE_SZ_AUTOPROG
    #define EE_SZ_STERILPROG         13

    //////////////////////////////////////////////////////////
    #define DEFAULT_LAT         36.809982
    #define DEFAULT_LNG         54.448833
    #define DEFAULT_NAME        "SmartHive1"
    #define DEFAULT_BLEPASS     123456

    
#endif 
