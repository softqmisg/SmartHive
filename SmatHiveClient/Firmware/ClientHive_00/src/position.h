#ifndef _POSITION_H_
#define _POSITION_H_
#include "time.h"
extern "C"{
    typedef struct
    {
        double latitude;
        double longitude;
    } position_t;
}
class Position {
    private:
        position_t position;
        tm sunrise;
        tm sunset;
        tm noon;
    public:
    Position(double lat,double lng):position({lat,lng}){};
    Position(){};
    ~Position(){};
    void setPosition(double lat,double lng){ position.latitude=lat;position.longitude=lng;};
    void setPosition(position_t pos){ position.latitude=pos.latitude;position.longitude=pos.longitude;};
    position_t getPosition(){return position;};
    void calculateSun(tm t);
    tm getSunrise(){return sunrise;};
    tm getSunset(){return sunset;};

};
#endif