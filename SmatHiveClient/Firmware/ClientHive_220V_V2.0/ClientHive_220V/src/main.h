#ifndef __MAIN_H__
#define __MAIN_H__
    #define ESP32_GPIO0                     0
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
    #define ESP32_GPIO25_SDPRESENT          25
    #define ESP32_GPIO26_SDCS               26
    #define ESP32_GPIO27_RTC_SDA            27
    #define ESP32_GPIO32_RTC_SCL            32
    #define ESP32_GPIO35_THERMALFEEDBACK    35
    #define ESP32_GPIO39_FANFEEDBACK        39
    //////////////////////////////////////////////////////
    #define EEPROM_SZ       512
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

    #define EE_ADD_KPCOEFF          EE_ADD_BLEPASS+EE_SZ_BLEPASS
    #define EE_SZ_KPCOEFF           8

    #define EE_ADD_KICOEFF          EE_ADD_KPCOEFF+EE_SZ_KPCOEFF
    #define EE_SZ_KICOEFF           8

    #define EE_ADD_KDCOEFF          EE_ADD_KICOEFF+EE_SZ_KICOEFF
    #define EE_SZ_KDCOEFF           8

    #define EE_ADD_AUTOPROGS_START  EE_ADD_KDCOEFF+EE_SZ_KDCOEFF
    #define EE_SZ_AUTOPROGS         42//(1+1+8+8+8+8+8) 

    #define EE_ADD_STERILPROGS_START  EE_ADD_AUTOPROGS_START+EE_SZ_AUTOPROGS
    #define EE_SZ_STERILPROGS         42//(1+1+8+8+8+8+8) 
    //////////////////////////////////////////////////////////
    #define DEFAULT_LAT         36.809982
    #define DEFAULT_LNG         54.448833
    #define DEFAULT_NAME        "SmartHive1"
    #define DEFAULT_BLEPASS     123456
    #define DEFAULT_KPCOEFF     2
    #define DEFAULT_KICOEFF     5
    #define DEFAULT_KDCOEFF     1
    
#endif 
