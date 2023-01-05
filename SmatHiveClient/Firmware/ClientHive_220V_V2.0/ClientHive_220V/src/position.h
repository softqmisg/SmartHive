#ifndef _POSITION_H_
#define _POSITION_H_
#include "RTClib_Tiny.h"
#include "math.h"


typedef struct
{
    double latitude;
    double longitude;
} position_t;

class Position {
    private:
        position_t position;
        double UTCoff=3.5;
        DateTime sunrise;
        DateTime sunset;
        DateTime noon;
        double calcJD(DateTime t);
        double degToRad(double angleDeg);
        double radToDeg(double angleRad);
        double calcMeanObliquityOfEcliptic(double t);
        double calcGeomMeanLongSun(double t);
        double calcObliquityCorrection(double t);
        double calcEccentricityEarthOrbit(double t);
        double calcGeomMeanAnomalySun(double t);
        double calcEquationOfTime(double t);
        double calcTimeJulianCent(double jd);
        double calcSunTrueLong(double t);
        double calcSunApparentLong(double t);
        double calcSunDeclination(double t);
        double calcHourAngleSunrise(double lat, double solarDec);
        double calcHourAngleSunset(double lat, double solarDec);
        double calcJDFromJulianCent(double t);
        double calcSunEqOfCenter(double t);
        double calcSunriseUTC(double JD, double latitude, double longitude);
        double calcSunsetUTC(double JD, double latitude, double longitude);

    public:
        Position(double lat,double lng):position{lat,lng}{};
        Position(position_t pos):position{pos.latitude,pos.longitude}{};
        ~Position(){};
        void setPosition(double lat,double lng){ position.latitude=lat;position.longitude=lng;};
        void setPosition(position_t pos){ position.latitude=pos.latitude;position.longitude=pos.longitude;};
        position_t getPosition(){return position;};
        void calculateSun(DateTime t);
        DateTime getSunrise(){return sunrise;};
        DateTime getSunset(){return sunset;};
        DateTime getNoon(){return noon;};
};
#endif