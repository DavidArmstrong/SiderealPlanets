/******************************************************************************
SiderealPlanets.h
Sidereal Planets Arduino Library Header File
David Armstrong
Version 1.0.0 - July 1, 2021
https://github.com/DavidArmstrong/SiderealPlanets

This file prototypes the SiderealPlanets class, as implemented in SiderealPlanets.cpp

Resources:
Uses math.h for math functions

Development environment specifics:
Arduino IDE 1.8.13
Teensy loader - untested

This code is released under the [MIT License](http://opensource.org/licenses/MIT)
Please review the LICENSE.md file included with this example.
Distributed as-is; no warranty is given.

TODO:
  Test with Teensy

******************************************************************************/

// ensure this library description is only included once
#ifndef __SiderealPlanets_h
#define __SiderealPlanets_h

//Uncomment the following line for debugging output
//#define debug_sidereal_planets

#include <stdint.h>
#include <math.h>

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// Structure to hold data
// We need to populate this when we calculate data
struct SiderealPlanetsData {
  public:
    double Latitude;
    double Longitude;
    double GMTdate;
    double GMTtime;
    double DST;
    double timeZone;
    double LST;
    double RightAscension;
    double Declination;
    double Altitude;
	double Azimuth;
};

// Sidereal_Planets library description
class SiderealPlanets {
  // user-accessible "public" interface
  public:
    SiderealPlanetsData spData;
    boolean begin(void);
	double decimalDegrees(int degrees, int minutes, float seconds);
	boolean setTimeZone(int zone);
	boolean setTimeZone(float zone);
    boolean useAutoDST(void);
	void setDST(void);
	void rejectDST(void);
    boolean setLatLong(double latitude, double longitude);
	boolean setGMTdate(int year, int month, int day);
    boolean setGMTtime(int hours, int minutes, float seconds);
    boolean setLocalTime(int hours, int minutes, float seconds);
	double getGMT(void);
	double getLT(void);
	double modifiedJulianDate1900(void);
    double getLocalSiderealTime(void);
    double getGMTsiderealTime(void);
	double doLST2LT(double LST);
	double doLST2GMT(double LST);
	boolean setElevationM(double height);
	boolean setElevationF(double height);
    boolean setRAdec(double RightAscension, double Declination);
    boolean setAltAz(double Altitude, double Azimuth);
    double getRAdec(void);
    double getDeclinationDec(void);
    double getAltitude(void);
    double getAzimuth(void);
    boolean doRAdec2AltAz(void);
    boolean doAltAz2RAdec(void);
	boolean doNutation(void);
	double getDP(void);
	double getDO(void);
	double doObliquity(void);
	boolean setEcliptic(double longitude, double latitude);
	double getEclipticLongitude(void);
	double getEclipticLatitude(void);
	boolean doEcliptic2RAdec(void);
    boolean doPrecessFrom2000(void);
	boolean doPrecessTo2000(void);
	boolean doLunarParallax(void);
	boolean setEquatHorizontalParallax(double hp); //For testing only
	double getEquatHorizontalParallax(void);
    boolean doRefractionF(double pressure, double temperature);
    boolean doRefractionC(double pressure, double temperature);
	boolean doAntiRefractionF(double pressure, double temperature);
	boolean doAntiRefractionC(double pressure, double temperature);
	boolean doRiseSetTimes(double DI);
	double getRiseTime(void);
	double getSetTime(void);
	double doAnomaly(double meanAnomaly, double eccentricity);
	double getTrueAnomaly(void);
    boolean doSun(void);
    boolean doMoon(void);
	boolean doPlanetElements(void);
	double getPL(int i, int j);
	boolean doPlans(int IP);
	double getHelioLong(void);
	double getHelioLat(void);
	double getRadiusVec(void);
	double getDistance(void);
    boolean doMercury(void);
    boolean doVenus(void);
	boolean doMars(void);
    boolean doJupiter(void);
    boolean doSaturn(void);
    boolean doUranus(void);
	boolean doNeptune(void);
	boolean doSunRiseSetTimes(void);
	double getSunriseTime(void);
	double getSunsetTime(void);
	boolean doMoonRiseSetTimes(void);
	double getMoonriseTime(void);
	double getMoonsetTime(void);
	void printDegMinSecs(double n);
	
  // library-accessible "private" interface
  private:
    const double F2PI = 2.0 * M_PI;
    const double FPI  = M_PI;
    const double FPIdiv2 = M_PI_2;
    const double FminusPIdiv2 = -M_PI_2;
    const double FPIdiv4 = M_PI_4;

    float TimeZoneOffset;
	int DSToffset;
	double decLat, decLong, radLat, radLong, cosLat, sinLat, mjd1900;
	boolean autoDST, useDST, leapYear, DstSelected, GmtDateInput, GmtTimeInput;
	boolean MJDdone, precessArrayDone;
	boolean obliquityDone, nutationDone, Ecl2RaDecDone, risetDone;
	int GMTyear, GMTmonth, GMTday, GMTminute, GMThour;
	float GMTseconds;
	double T, GMTtime, GMTsiderealTime, LocalSiderealTime;
	double seaLevelHeightMeters, EquatHorizontalParallax;
	double RAdec, DeclinationDec, AltDec, AzDec;
	double RArad, DeclinationRad, AltRad, AzRad;
	double sinRA, sinDec, sinAlt, sinAz;
	double cosRA, cosDec, cosAlt, cosAz;
	double mt[4][4], mv[4][4]; //precession arrays
    double trueAnomaly, eccentricAnomaly, meanAnomaly;
	double DP, DO, OB; //nutation, obliquity
	double EclLongitude, EclLatitude, SE, CE, AM, SR, RR, AL;
	double AU, AD, LU, LD, L0, S0, P0, V0, PMpl;
	double PL[8][10]; //array for planetary elements

    void doAutoDST(void);
	byte calcLocalHour(int year, byte month, byte day, byte hour, byte offset);
	char day_of_week(int year, int month, int day);
	double inRange24(double d);
	double inRange360(double d);
	double inRange2PI(double d);
	double deg2rad(double n);
	double rad2deg(double n);
	boolean doPrecessArray(void);

    int monthDays[25] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    union FourByte {
      unsigned long bit32;
      unsigned int bit16[2];
      unsigned char bit8[4];
    };
};
#endif
