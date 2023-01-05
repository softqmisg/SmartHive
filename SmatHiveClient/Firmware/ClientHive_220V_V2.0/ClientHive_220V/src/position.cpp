#include "position.h"
double Position::calcJD(DateTime t)
{
    int year=t.year(),month=t.month(),day=t.day();
	if (month <= 2) {
		year -= 1;
		month += 12;
	}
	int A = floor(year / 100);
	int B = 2 - A + floor(A / 4);

	double JD = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1))
			+ day + B - 1524.5;
	return JD;
}


double Position::degToRad(double angleDeg) {
	return (M_PI * angleDeg / 180.0);
}

double Position::radToDeg(double angleRad) {
	return (180.0 * angleRad / M_PI);
}

double Position::calcMeanObliquityOfEcliptic(double t) {
	double seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * (0.001813)));
	double e0 = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;

	return e0;              // in degrees
}

double Position::calcGeomMeanLongSun(double t) {

	double L = 280.46646 + t * (36000.76983 + 0.0003032 * t);
	while ((int) L > 360) {
		L -= 360.0;

	}
	while (L < 0) {
		L += 360.0;

	}

	return L;              // in degrees
}

double Position::calcObliquityCorrection(double t) {
	double e0 = calcMeanObliquityOfEcliptic(t);

	double omega = 125.04 - 1934.136 * t;
	double e = e0 + 0.00256 * cos(degToRad(omega));
	return e;               // in degrees
}

double Position::calcEccentricityEarthOrbit(double t) {
	double e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
	return e;               // unitless
}

double Position::calcGeomMeanAnomalySun(double t) {
	double M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
	return M;               // in degrees
}

double Position::calcEquationOfTime(double t) {

	double epsilon = calcObliquityCorrection(t);
	double l0 = calcGeomMeanLongSun(t);
	double e = calcEccentricityEarthOrbit(t);
	double m = calcGeomMeanAnomalySun(t);
	double y = tan(degToRad(epsilon) / 2.0);
	y *= y;
	double sin2l0 = sin(2.0 * degToRad(l0));
	double sinm = sin(degToRad(m));
	double cos2l0 = cos(2.0 * degToRad(l0));
	double sin4l0 = sin(4.0 * degToRad(l0));
	double sin2m = sin(2.0 * degToRad(m));
	double Etime = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0
			- 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;

	return radToDeg(Etime) * 4.0;	// in minutes of time

}

double Position::calcTimeJulianCent(double jd) {

	double T = (jd - 2451545.0) / 36525.0;
	return T;
}

double Position::calcSunTrueLong(double t) {
	double l0 = calcGeomMeanLongSun(t);
	double c = calcSunEqOfCenter(t);

	double O = l0 + c;
	return O;               // in degrees
}

double Position::calcSunApparentLong(double t) {
	double o = calcSunTrueLong(t);

	double omega = 125.04 - 1934.136 * t;
	double lambda = o - 0.00569 - 0.00478 * sin(degToRad(omega));
	return lambda;          // in degrees
}

double Position::calcSunDeclination(double t) {
	double e = calcObliquityCorrection(t);
	double lambda = calcSunApparentLong(t);

	double sint = sin(degToRad(e)) * sin(degToRad(lambda));
	double theta = radToDeg(asin(sint));
	return theta;           // in degrees
}

double Position::calcHourAngleSunrise(double lat, double solarDec) {
	double latRad = degToRad(lat);
	double sdRad = degToRad(solarDec);

	double HA = (acos(
			cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad))
					- tan(latRad) * tan(sdRad)));

	return HA;              // in radians
}

double Position::calcHourAngleSunset(double lat, double solarDec) {
	double latRad = degToRad(lat);
	double sdRad = degToRad(solarDec);

	double HA = (acos(
			cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad))
					- tan(latRad) * tan(sdRad)));

	return -HA;              // in radians
}


double Position::calcJDFromJulianCent(double t) {
	double JD = t * 36525.0 + 2451545.0;
	return JD;
}

double Position::calcSunEqOfCenter(double t) {
	double m = calcGeomMeanAnomalySun(t);

	double mrad = degToRad(m);
	double sinm = sin(mrad);
	double sin2m = sin(mrad + mrad);
	double sin3m = sin(mrad + mrad + mrad);

	double C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t))
			+ sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
	return C;		// in degrees
}

double Position::calcSunriseUTC(double JD, double latitude, double longitude) {

	double t = calcTimeJulianCent(JD);

	// *** First pass to approximate sunrise

	double eqTime = calcEquationOfTime(t);
	double solarDec = calcSunDeclination(t);
	double hourAngle = calcHourAngleSunrise(latitude, solarDec);
	double delta = longitude - radToDeg(hourAngle);
	double timeDiff = 4 * delta;	// in minutes of time
	double timeUTC = 720 + timeDiff - eqTime;	// in minutes
	double newt = calcTimeJulianCent(
			calcJDFromJulianCent(t) + timeUTC / 1440.0);

	eqTime = calcEquationOfTime(newt);
	solarDec = calcSunDeclination(newt);

	hourAngle = calcHourAngleSunrise(latitude, solarDec);
	delta = longitude - radToDeg(hourAngle);
	timeDiff = 4 * delta;
	timeUTC = 720 + timeDiff - eqTime; // in minutes

	return timeUTC;
}

double Position::calcSunsetUTC(double JD, double latitude, double longitude) {

	double t = calcTimeJulianCent(JD);

	// *** First pass to approximate sunset

	double eqTime = calcEquationOfTime(t);
	double solarDec = calcSunDeclination(t);
	double hourAngle = calcHourAngleSunset(latitude, solarDec);
	double delta = longitude - radToDeg(hourAngle);
	double timeDiff = 4 * delta;	// in minutes of time
	double timeUTC = 720 + timeDiff - eqTime;	// in minutes
	double newt = calcTimeJulianCent(
			calcJDFromJulianCent(t) + timeUTC / 1440.0);

	eqTime = calcEquationOfTime(newt);
	solarDec = calcSunDeclination(newt);

	hourAngle = calcHourAngleSunset(latitude, solarDec);
	delta = longitude - radToDeg(hourAngle);
	timeDiff = 4 * delta;
	timeUTC = 720 + timeDiff - eqTime; // in minutes

	return timeUTC;
}

void Position::calculateSun(DateTime t)
{
    double JD=calcJD(t);
    double minutesunrise= calcSunriseUTC( JD,  position.latitude,  -position.longitude)+(double)UTCoff*60.0;
    double minutesunset= 	calcSunsetUTC( JD,  position.latitude,  -position.longitude)+(double)UTCoff*60.0;
    double minutenoon=(minutesunrise+minutesunset)/2.0;
	DateTime tmp;
	tmp=DateTime(minutesunrise*60.0+SECONDS_FROM_1970_TO_2000);
    sunrise=DateTime(t.year(),t.month(),t.day(),tmp.hour(),tmp.minute(),tmp.second());

	tmp=DateTime(minutesunset*60.0+SECONDS_FROM_1970_TO_2000);
    sunset=DateTime(t.year(),t.month(),t.day(),tmp.hour(),tmp.minute(),tmp.second());

    tmp=DateTime(minutenoon*60.0+SECONDS_FROM_1970_TO_2000);
    noon=DateTime(t.year(),t.month(),t.day(),tmp.hour(),tmp.minute(),tmp.second());

}