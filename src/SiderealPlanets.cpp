/******************************************************************************
SiderealPlanets.cpp
Sidereal Planets Arduino Library C++ source
David Armstrong
Version 1.0.0 - July 1, 2021
https://github.com/DavidArmstrong/SiderealPlanets

Resources:
Uses math.h for math function

Development environment specifics:
Arduino IDE 1.8.13
Teensy loader - untested

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example.
Distributed as-is; no warranty is given.
******************************************************************************/

// include this library's description file
#include "SiderealPlanets.h"

// Need the following define for SAMD processors
#if defined (ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

// Public Methods //////////////////////////////////////////////////////////
// Start by doing any setup, and verifying that doubles are supported
boolean SiderealPlanets::begin(void) {
  float  fpi = 3.14159265358979;
  double dpi = 3.14159265358979;
  if (dpi == (double)fpi ) return false; //double and float are the same here!

  DSToffset = 0; // GMT is default
  TimeZoneOffset = 0.0;
  decLat = 51.178889; // Default in decimal Degrees
  decLong = -1.826111;
  autoDST = false;
  useDST = false;
  leapYear = false;
  DstSelected = false;
  GmtDateInput = false;
  GmtTimeInput = false;
  MJDdone = false;
  precessArrayDone = false;
  obliquityDone = false;
  nutationDone = false;
  Ecl2RaDecDone = false;
  risetDone = false;
  return true;
}

double SiderealPlanets::decimalDegrees(int degrees, int minutes, float seconds) {
  int sign = 1;
  if (degrees < 0) {
	degrees = -degrees;
	sign = -1;
  }
  if (minutes < 0) {
	minutes = -minutes;
	sign = -1;
  }
  if (seconds < 0) {
	seconds = -seconds;
	sign = -1;
  }
  double decDeg = degrees + (minutes / 60.0) + (seconds / 3600.);
  return decDeg * sign;
}

boolean SiderealPlanets::setTimeZone(int zone) {
  if (zone < -12 || zone > 12) return false; //bad input
  return setTimeZone((float)zone);
}

boolean SiderealPlanets::setTimeZone(float zone) {
  if (zone < -12.0 || zone > 12.0) return false; //bad input
  TimeZoneOffset = zone;
  doAutoDST();
  return true;
}

// Select automatic determination of DST usage
boolean SiderealPlanets::useAutoDST(void) {
  // See https://github.com/nseidle/Daylight_Savings_Time_Example
  autoDST = true;
  DstSelected = true;
  doAutoDST();
  return useDST;
}

// Enable DST in calculations
void SiderealPlanets::setDST(void) {
  useDST = true;
  autoDST = false;
  DstSelected = true;
  DSToffset = 1;
}

// Disable DST use in all calculations
void SiderealPlanets::rejectDST(void) {
  useDST = false;
  autoDST = false;
  DstSelected = true;
  DSToffset = 0;
}

void SiderealPlanets::doAutoDST(void) {
  if (GmtTimeInput && GmtDateInput && DstSelected && autoDST)
    calcLocalHour( GMTyear, (byte)GMTmonth, (byte)GMTday, (byte)GMThour, (byte)TimeZoneOffset);
}

// See https://github.com/nseidle/Daylight_Savings_Time_Example
//Given a year/month/day/current UTC/local offset give me local time
byte SiderealPlanets::calcLocalHour(int year, byte month, byte day, byte hour, byte local_hour_offset)
{
  //Since 2007 DST starts on the second Sunday in March and ends the first Sunday of November
  //Let's just assume it's going to be this way for awhile (silly US government!)
  //Example from: http://stackoverflow.com/questions/5590429/calculating-daylight-savings-time-from-only-date

  byte DoW = day_of_week(year, month, day); //Get the day of the week. 0 = Sunday, 6 = Saturday
  int previousSunday = day - DoW;

  boolean dst = false; //Assume we're not in DST
  if(month > 3 && month < 11) dst = true; //DST is happening!

  //In March, we are DST if our previous Sunday was on or after the 8th.
  if (month == 3)
  {
    if(previousSunday >= 8) dst = true; 
  } 
  //In November we must be before the first Sunday to be dst.
  //That means the previous Sunday must be before the 1st.
  if(month == 11)
  {
    if(previousSunday <= 0) dst = true;
  }
  
  if(dst == true) hour++;

  //Convert UTC hours to local current time using local_hour
  if(hour < local_hour_offset)
    hour += 24; //Add 24 hours before subtracting local offset
  hour -= local_hour_offset;
  if(hour > 12) hour -= 12; //Get rid of military time
  
  /*
  Serial.print("Hour: ");
  Serial.println(hour);
  Serial.print("Day of week: ");
  if(DoW == 0) Serial.println("Sunday");
  if(DoW == 1) Serial.println("Monday");
  if(DoW == 2) Serial.println("Tuesday");
  if(DoW == 3) Serial.println("Wednesday");
  if(DoW == 4) Serial.println("Thursday");
  if(DoW == 5) Serial.println("Friday!");
  if(DoW == 6) Serial.println("Saturday");
  */
  useDST = dst; //set the flag that we are using it or not
  if (autoDST) {
    if (dst == true)
	  DSToffset = 1; // this is the one hour difference
    else
      DSToffset = 0;
  }
  return(hour);
}

//Given the current year/month/day
//Returns 0 (Sunday) through 6 (Saturday) for the day of the week
//From: http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
//This function assumes the month from the caller is 1-12
char SiderealPlanets::day_of_week(int year, int month, int day)
{
  //Devised by Tomohiko Sakamoto in 1993, it is accurate for any Gregorian date:
  static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4  };
  year -= month < 3;
  return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
}

double SiderealPlanets::modifiedJulianDate1900(void) {
  // Based on Year 1900
  if (MJDdone) return mjd1900;
  int y1 = GMTyear;
  long m1 = GMTmonth;
  long b = 0;
  boolean doextra = false;
  if (y1 < 1) y1 += 1; //Account for there being no year zero
  if (GMTmonth < 3) {
	m1 =  GMTmonth + 12;
	y1 -= 1;
  }
  if (y1 > 1582) {
	doextra = true;
  } else {
	if (y1 < 1582) {
      doextra = false;
	} else {
      if (y1 == 1582 && GMTmonth < 10) {
        doextra = false;
	  } else {
        if ((y1 == 1582 && GMTmonth == 10 && GMTday < 5)) {
          doextra = false;
		} else {
          if (GMTmonth > 10 || GMTday > 14) {
            doextra = true;
		  } else {
            return -9999999.; //not allowed
		  }
		}
	  }
	}
  }
  if (doextra) {
	long a = y1 / 100;
	b = 2 - a + (a / 4);
  }
  long c = (365.25 * (double)y1);
  c -= 694025;
  if (y1 < 0) {
    c = (365.25 * y1) - 0.75;
	c -= 694025;
  }
  long d = (30.6001 * (m1 + 1));
  mjd1900 = b + c + d + GMTday - 0.5;
  MJDdone = true;
  return mjd1900;
}

// Set the Latitude and Logitude used in calculations
boolean SiderealPlanets::setLatLong(double latitude, double longitude) {
  // Input latitude and longitude are in decimal degrees
  // save these, and also save values in radians
  if (decLat == latitude && decLong == longitude) return true; //Already did it
  decLat = latitude;
  radLat = deg2rad(decLat);
  decLong = longitude;
  radLong = deg2rad(decLong);
  cosLat = cos(radLat);
  sinLat = sin(radLat);
  risetDone = false;
  return true;
}

boolean SiderealPlanets::setGMTdate(int year, int month, int day) {
  if (GMTyear == year && GMTmonth == month && GMTday == day) return true; //Already did it
  if (year == 0) return false;
  GMTyear = year;
  // Is this a leap year?
  if (year % 4 == 0) {
    if (year % 100 == 0) {
      if (year % 400 == 0)
        leapYear = true;
      else
        leapYear = false;
    } else {
      leapYear = true;
	}
  } else {
    leapYear = false;
  }
  if (month < 1 or month > 12) {
	return false;
  } else {
	GMTmonth = month;
  }
  if (day < 1 or day > 31) {
	return false;
  } else {
	if (leapYear)
      if (monthDays[month + 12] < day) return false;
    else
      if (monthDays[month] < day) return false;
    GMTday = day;
  }
  GmtDateInput = true;
  doAutoDST();
  MJDdone = false;
  precessArrayDone = false;
  obliquityDone = false;
  nutationDone = false;
  Ecl2RaDecDone = false;
  return true;
}

boolean SiderealPlanets::setGMTtime(int hours, int minutes, float seconds) {
  if (GMThour == hours && GMTminute == minutes && GMTseconds == seconds) return true; //Already done
  if (hours < 0 || hours > 23)
	return false;
  else
	GMThour = hours;
  if (minutes < 0 || minutes > 59)
	return false;
  else
	GMTminute = minutes;
  if (seconds < 0. || seconds > 59.999999999)
	return false;
  else
	GMTseconds = seconds;
  GmtTimeInput = true;
  doAutoDST();
  GMTtime = GMThour + (GMTminute / 60.0) + (GMTseconds / 3600.0);
  return true;
}

boolean SiderealPlanets::setLocalTime(int hours, int minutes, float seconds) {
  // The user is responsible for setting the GMT date, the Time Zone,
  // and DST before calling this so that we can set GMT also
  if (GmtDateInput == false) return false;
  if (DstSelected == false) return false;
  // Cheat to do error checking, etc, to prep to set real GMT
  if (hours < 0 || hours > 23) return false;
  if (minutes < 0 || minutes > 59) return false;
  if (seconds < 0. || seconds > 59.999999999) return false;
  // if doAutoDST was set to do, we just did it anyway
  GMThour = hours - TimeZoneOffset;
  if (useDST) GMThour -= 1;
  return setGMTtime(inRange24(GMThour), minutes, seconds);
}

double SiderealPlanets::getGMT(void) {
  return GMTtime;
}

double SiderealPlanets::getLT(void) {
  double LT = GMTtime + TimeZoneOffset;
  if (useDST) LT += 1;
  return inRange24(LT);
}

double SiderealPlanets::getLocalSiderealTime(void) {
  LocalSiderealTime = inRange24(getGMTsiderealTime() + (decLong / 15.0));
  return LocalSiderealTime;
}

double SiderealPlanets::getGMTsiderealTime(void) {
  modifiedJulianDate1900();
  double d = ((int)(mjd1900 - 0.5)) + 0.5;
  double t = (d / 36525.0) - 1.;
  double r0 = t * (5.13366e-2 + (t * (2.586222e-5 - (t * 1.722e-9))));
  double r1 = 6.697374558 + (2400.0 * (t - ((GMTyear - 2000.0) / 100.0)));
  double t0 = inRange24(r0 + r1);
  GMTsiderealTime = inRange24((GMTtime * 1.002737908) + t0);
  return GMTsiderealTime;
}

double SiderealPlanets::doLST2LT(double LST) {
  //for computing rise/set times
  modifiedJulianDate1900();
  double d = ((int)(mjd1900 - 0.5)) + 0.5;
  double t = (d / 36525.0) - 1.;
  double r0 = t * (5.13366e-2 + (t * (2.586222e-5 - (t * 1.722e-9))));
  double r1 = 6.697374558 + (2400.0 * (t - ((GMTyear - 2000.0) / 100.0)));
  double t0 = inRange24(r0 + r1);
  double T = inRange24(t0 - (DSToffset + TimeZoneOffset) * 1.002737908);
  double SG = inRange24(LST - (decLong / 15.0));
  if (SG < T) SG += 24.;
  double TL = inRange24((SG - T) * 9.972695677e-1);
  return TL;
}

double SiderealPlanets::doLST2GMT(double LST) {
  //for computing rise/set times of Sun
  return doLST2LT(LST) - (DSToffset + TimeZoneOffset);
}

boolean SiderealPlanets::setElevationM(double height) {
  seaLevelHeightMeters = height;
  return true;
}

boolean SiderealPlanets::setElevationF(double height) {
  seaLevelHeightMeters = height / 3.2808;
  return true;
}

double SiderealPlanets::inRange24(double d) {
  while (d < 0.) {
	d += 24.;
  }
  while (d >= 24.) {
	d -= 24.;
  }
  return d;
}

double SiderealPlanets::inRange360(double d) {
  while (d < 0.) {
	d += 360.;
  }
  while (d >= 360.) {
	d -= 360.;
  }
  return d;
}

double SiderealPlanets::inRange2PI(double d) {
  while (d < 0.) {
	d += F2PI;
  }
  while (d >= F2PI) {
	d -= F2PI;
  }
  return d;
}

double SiderealPlanets::deg2rad(double n) {
  return n * 1.745329252e-2;
}

double SiderealPlanets::rad2deg(double n) {
  return n * 5.729577951e1;
}

boolean SiderealPlanets::setRAdec(double RightAscension, double Declination) {
  if (RAdec == RightAscension && DeclinationDec == Declination) return true; //Already done
  RAdec = RightAscension;
  RArad = deg2rad(RAdec) * 15.;
  sinRA = sin(RArad);
  cosRA = cos(RArad);
  DeclinationDec = Declination;
  DeclinationRad = deg2rad(DeclinationDec);
  sinDec = sin(DeclinationRad);
  cosDec = cos(DeclinationRad);
  return true;
}

boolean SiderealPlanets::setAltAz(double Altitude, double Azimuth) {
  if (AltDec == Altitude && AzDec == Azimuth) return true; //Already done
  AltDec = Altitude;
  AltRad = deg2rad(AltDec);
  sinAlt = sin(AltRad);
  cosAlt = cos(AltRad);
  AzDec = Azimuth;
  AzRad = deg2rad(AzDec);
  sinAz = sin(AzRad);
  cosAz = cos(AzRad);
  return true;
}

double SiderealPlanets::getRAdec(void) {
  RAdec = rad2deg(RArad) / 15.;
  return RAdec;
}

double SiderealPlanets::getDeclinationDec(void) {
  DeclinationDec = rad2deg(DeclinationRad);
  return DeclinationDec;
}

double SiderealPlanets::getAltitude(void) {
  AltDec = rad2deg(AltRad);
  return AltDec;
}

double SiderealPlanets::getAzimuth(void) {
  AzDec = rad2deg(AzRad);
  return AzDec;
}

boolean SiderealPlanets::doRAdec2AltAz(void) {
  double HAdec = inRange24(getLocalSiderealTime() - RAdec);
  double HArad = deg2rad(HAdec * 15.0);
  double cosHA = cos(HArad);
  double sinHA = sin(HArad);
  sinAlt = (sinDec * sinLat) + (cosDec * cosLat * cosHA);
  AltRad = asin(sinAlt);
  cosAlt = cos(AltRad);
  double b = cosLat * cosAlt;
  if (b < 1e-10) b = 1e-10;
  cosAz = (sinDec - (sinLat * sinAlt)) / b;
  AzRad = acos(cosAz);
  if (sinHA > 0) AzRad = F2PI - AzRad;
  sinAz = sin(AzRad);
  return true;
}

boolean SiderealPlanets::doAltAz2RAdec(void) {
  sinDec = (sinAlt * sinLat) + (cosAlt * cosLat * cosAz);
  DeclinationRad = asin(sinDec);
  cosDec = cos(DeclinationRad);
  double b = cosLat * cosDec;
  if (b < 1e-10) b = 1e-10;
  double cosHA = (sinAlt - (sinLat * sinDec)) / b;
  double HArad = acos(cosHA);
  if (sinAz > 0) HArad = F2PI - HArad;
  double HAdec = rad2deg(HArad) / 15.0;
  double RAdec = inRange24(getLocalSiderealTime() - HAdec);
  RArad = deg2rad(RAdec * 15.);
  sinRA = sin(RArad);
  cosRA = cos(RArad);
  cosDec = cos(DeclinationRad);
  return true;
}

boolean SiderealPlanets::doNutation(void) {
  //t = julian centuries since 2000 jan 1.5
  if (nutationDone == true) return true;
  double t = (modifiedJulianDate1900()) / 36525.0;
  double t2 = t * t;
  double a = 1.000021358e2 * t;
  double b = 360.0 * (a - floor(a));
  double L1 = 2.796967e2 + 3.03e-4 * t2 + b;
  double L2 = 2.0 * deg2rad(L1);
  a = 1.336855231e3 * t;
  b = 360. * (a - floor(a));
  double d1 = 2.704342e2 - 1.133e-3 * t2 + b;
  double d2 = 2.0 * deg2rad(d1);
  a = 9.999736056e1 * t;
  b = 360.0 * (a - floor(a));
  double M1 = 3.584758e2 - 1.5e-4 * t2 + b;
  M1 = deg2rad(M1);
  a = 1.325552359e3 * t;
  b = 360.0 * (a - floor(a));
  double M2 = 2.961046e2 + 9.192e-3 * t2 + b;
  M2 = deg2rad(M2);
  a = 5.372616667 * t;
  b = 360. * (a - floor(a));
  double N1 = 2.591833e2 + 2.078e-3 * t2 - b;
  N1 = deg2rad(N1);
  double N2 = 2. * N1;
  
  DP = (-17.2327 - 1.737e-2 * t) * sin(N1);
  DP = DP + (-1.2729 - 1.3e-4 * t) * sin(L2) + 2.088e-1 * sin(N2);
  DP = DP - 2.037e-1 * sin(d2) + (1.261e-1 - 3.1e-4 * t) * sin(M1);
  DP = DP + 6.75E-2 * sin(M2) - (4.97e-2 - 1.2e-4 * t) * sin(L2 + M1);
  DP = DP - 3.42e-2 * sin(d2 - N1) - 2.61e-2 * sin(d2 + M2);
  DP = DP + 2.14e-2 * sin(L2 - M1) - 1.49e-2 * sin(L2 - d2 + M2);
  DP = DP + 1.24E-2 * sin(L2 - N1) + 1.14e-2 * sin(d2 - M2);
  
  DO = (9.21 + 9.1E-4 * t) * cos(N1);
  DO = DO + (5.522e-1 - 2.9e-4 * t) * cos(L2) - 9.04e-2 * cos(N2);
  DO = DO + 8.84e-2 * cos(d2) + 2.16e-2 * cos(L2 + M1);
  DO = DO + 1.83e-2 * cos(d2 - N1) + 1.13e-2 * cos(d2 + M2);
  DO = DO + 9.3e-3 * cos(L2 - M1) - 6.6e-3 * cos(L2 - N1);
  
  DP = DP / 3600.0;
  DO = DO / 3600.0;
  nutationDone = true;
  return true;
}

double SiderealPlanets::getDP(void) {
  return DP;
}
double SiderealPlanets::getDO(void) {
  return DO;
}

double SiderealPlanets::doObliquity(void) {
  //Always include Nutation when computing Obliquity
  if (obliquityDone == true) return OB;
  doNutation();
  //modifiedJulianDate1900(); is mjd1900
  double t = (modifiedJulianDate1900() / 36525.0) - 1.0;
  double a = (46.815 + (0.0006 - 0.00181 * t) * t) * t;
  a = a / 3600.0;
  OB = 23.43929167 - a + DO;
  obliquityDone = true;
  return OB;
}

boolean SiderealPlanets::setEcliptic(double longitude, double latitude) {
  EclLongitude = deg2rad(longitude);
  EclLatitude = deg2rad(latitude);
  return true;
}

double SiderealPlanets::getEclipticLongitude(void) {
  return rad2deg(EclLongitude);
}

double SiderealPlanets::getEclipticLatitude(void) {
  return rad2deg(EclLatitude);
}

boolean SiderealPlanets::doEcliptic2RAdec(void) {
  // Ecliptic coordinates to Right Ascension, Declination
  if (Ecl2RaDecDone == false) {
    obliquityDone = false;
	doObliquity();
	double e = deg2rad(OB);
	SE = sin(e);
	CE = cos(e);
	Ecl2RaDecDone = true;
  }
  double CY = cos(EclLatitude);
  double SY = sin(EclLatitude);
  if (abs(CY) < 1e-20) CY = 1e-20;
  double TY = SY / CY;
  double CX = cos(EclLongitude);
  double SX = sin(EclLongitude);
  double S = (SY * CE) - (CY * SE * SX * (-1.));
  DeclinationRad = asin(S);
  sinDec = S;
  double A = (SX * CE) + (TY * SE * (-1.));
  RArad = atan(A / CX);
  if (CX < 0.) RArad += FPI;
  RArad = inRange2PI(RArad);
  getRAdec();
  getDeclinationDec();
  sinRA = sin(RArad);
  cosRA = cos(RArad);
  cosDec = cos(DeclinationRad);
  return true;
}

boolean SiderealPlanets::doPrecessFrom2000(void) {
  doPrecessArray();
  // convert input to column vector
  double cv[4], hl[4], sm;
  int i, j;
  cv[1] = cosRA * cosDec;
  cv[2] = sinRA * cosDec;
  cv[3] = sinDec;
  // multiply mv by cv
  for(j = 1; j < 4; j++) {
    sm = 0.0;
	for(i = 1; i < 4; i++) {
      sm += mv[i][j] * cv[i];
	}
	hl[j] = sm;
  }
  for(i = 1; i < 4; i++) {
    cv[i] = hl[i];
  }
  // convert column vector into output
  if (abs(cv[1]) < 1e-20) cv[1] = 1e-20;
  RArad = atan(cv[2] / cv[1]);
  DeclinationRad = asin(cv[3]);
  if (cv[1] < 0) RArad += FPI;
  RArad = inRange2PI(RArad);
  // Also update the 'other' internal stuff here
  sinRA = sin(RArad);
  cosRA = cos(RArad);
  sinDec = sin(DeclinationRad);
  cosDec = cos(DeclinationRad);
  return true;
}

boolean SiderealPlanets::doPrecessTo2000(void) {
  doPrecessArray();
  // convert input to column vector
  double cv[4], hl[4], sm;
  int i, j;
  cv[1] = cosRA * cosDec;
  cv[2] = sinRA * cosDec;
  cv[3] = sinDec;
  // multiply mt by cv
  for(j = 1; j < 4; j++) {
    sm = 0.0;
	for(i = 1; i < 4; i++) {
      sm += mt[i][j] * cv[i];
	}
	hl[j] = sm;
  }
  for(i = 1; i < 4; i++) {
    cv[i] = hl[i];
  }
  // convert column vector into output
  if (abs(cv[1]) < 1e-20) cv[1] = 1e-20;
  RArad = atan(cv[2] / cv[1]);
  DeclinationRad = asin(cv[3]);
  if (cv[1] < 0) RArad += FPI;
  RArad = inRange2PI(RArad);
  // Also update the 'other' internal stuff here
  sinRA = sin(RArad);
  cosRA = cos(RArad);
  sinDec = sin(DeclinationRad);
  cosDec = cos(DeclinationRad);
  return true;
}

boolean SiderealPlanets::doPrecessArray(void) {
  if (precessArrayDone == false) {
    //t = julian centuries since 2000 jan 1.5
    double t = (modifiedJulianDate1900() - 36525.0) / 36525.0;
	// Generate parameters for matrix elements
    double xa = (((0.000005 * t) + 0.0000839) * t + 0.6406161) * t;
    double za = (((0.0000051 * t) + 0.0003041) * t + 0.6406161) * t;
    double ta = (((-0.0000116 * t) - 0.0001185) * t + 0.556753) * t;
    xa = deg2rad(xa);
    za = deg2rad(za);
    ta = deg2rad(ta);
    double c1 = cos(xa);
    double c2 = cos(za);
    double c3 = cos(ta);
    double s1 = sin(xa);
    double s2 = sin(za);
    double s3 = sin(ta);
    // Generate matrix elements for the two matrixies needed
    mt[1][1] = c1 * c3 * c2 - s1 * s2;
	mv[1][1] = mt[1][1];
    mt[1][2] = -s1 * c3 * c2 - c1 * s2;
	mv[2][1] = mt[1][2];
    mt[1][3] = -s3 * c2;
	mv[3][1] = mt[1][3];
    mt[2][1] = c1 * c3 * s2 + s1 * c2;
	mv[1][2] = mt[2][1];
    mt[2][2] = -s1 * c3 * s2 + c1 * c2;
	mv[2][2] = mt[2][2];
    mt[2][3] = -s3 * s2;
	mv[3][2] = mt[2][3];
    mt[3][1] = c1 * s3;
	mv[1][3] = mt[3][1];
    mt[3][2] = -s1 * s3;
	mv[2][3] = mt[3][2];
    mt[3][3] = c3;
	mv[3][3] = mt[3][3];
    precessArrayDone = true;
  }
  return true;
}

boolean SiderealPlanets::doLunarParallax(void) {
  //do on first call to this location
  double u = atan(9.96647e-1 * sinLat / cosLat);
  double c2 = cos(u);
  double s2 = sin(u);
  double ht = seaLevelHeightMeters / 6378140.0; // height in earth radii
  double rs = (9.96647e-1 * s2) + (ht * sinLat);
  double rc = c2 + (ht * cosLat);
  
  double rp = 1. / sin(deg2rad(EquatHorizontalParallax));
  // x = Hour Angle, y = declination
  double lst = getLocalSiderealTime();
  double HAdec = inRange24(lst - RAdec);
  double HArad = deg2rad(HAdec) * 15.0;
  double cosHA = cos(HArad);
  double sinHA = sin(HArad);

  double a = (rc * sinHA) / ((rp * cosDec) - (rc * cosHA));
  double dx = atan(a); //correction to hour angle, in radians
  double p = HArad + dx;
  double cp = cos(p);
  HArad = inRange2PI(p);
  DeclinationRad = atan(cp * (rp * sinDec - rs) / (rp * cosDec * cosHA - rc));
  // Also update the 'other' internal stuff here
  HAdec = rad2deg(HArad) / 15.;
  RAdec = inRange24(lst - HAdec);
  RArad = deg2rad(RAdec) * 15.;
  sinRA = sin(RArad);
  cosRA = cos(RArad);
  sinDec = sin(DeclinationRad);
  cosDec = cos(DeclinationRad);
  return true;
}

boolean SiderealPlanets::setEquatHorizontalParallax(double hp) {
  //This is here for testing only - not to be used in Real Life!
  //That's why it's left undocumented
  EquatHorizontalParallax = hp;
  return true;
}

double SiderealPlanets::getEquatHorizontalParallax(void) {
  //This is here for testing only - not to be used in Real Life!
  //That's why it's left undocumented
  return EquatHorizontalParallax;
}

boolean SiderealPlanets::doRefractionF(double pressure, double temperature) {
  pressure *= 33.8639;
  temperature = (temperature - 32.) * (5. / 9.);
  doRefractionC(pressure, temperature);
  return true;
}

boolean SiderealPlanets::doRefractionC(double pressure, double temperature) {
  double rf = 0.;
  AltDec = rad2deg(AltRad);
  //Valid for any values above -5 degrees
  double y = AltRad;
  double y1 = AltRad;
  double y2 = AltRad;
  double r1 = 0.0;
  double r2 = r1;
  double q;
  do {
	r1 = r2;
	y = y1 + r1;
	q = y;
    if (y >= 2.617994e-1) {
      rf = 7.888888e-5 * pressure / ((273 + temperature) * tan(y));
    } else {
      if (y >= -8.7e-2) {
        double yd = y * 5.729578e1;
        double a = ((2e-5 * yd + 1.96e-2) * yd + 1.594e-1) * pressure;
        double b = (273 + temperature) * ((8.45e-2 * yd + 5.05e-1) * yd + 1.0);
        rf = (a / b) * 1.745329e-2;
      }
    }
	r2 = rf;
  } while (r2 != 0. && abs(r2 - r1) > 1e-6);
  y = y2;
  AltRad = AltRad + rf;
  sinAlt = sin(AltRad);
  cosAlt = cos(AltRad);
  //SerialUSB.print("Correction ");
  //printDegMinSecs(rad2deg(rf));
  //SerialUSB.println();
  return true;
}

boolean SiderealPlanets::doAntiRefractionF(double pressure, double temperature) {
  pressure *= 33.8639;
  temperature = (temperature - 32.) * (5. / 9.);
  doAntiRefractionC(pressure, temperature);
  return true;
}

boolean SiderealPlanets::doAntiRefractionC(double pressure, double temperature) {
  double rf = 0.;
  AltDec = rad2deg(AltRad);
  //Valid for any values above -5 degrees
  if (AltRad >= 2.617994e-1) {
    rf = -7.888888e-5 * pressure / ((273 + temperature) * tan(AltRad));
  } else {
    if (AltRad >= -8.7e-2) {
      double yd = AltRad * 5.729578e1;
      double a = ((2e-5 * yd + 1.96e-2) * yd + 1.594e-1) * pressure;
      double b = (273 + temperature) * ((8.45e-2 * yd + 5.05e-1) * yd + 1.0);
      rf = -(a / b) * 1.745329e-2;
    }
  }
  AltRad = AltRad + rf;
  sinAlt = sin(AltRad);
  cosAlt = cos(AltRad);
  //SerialUSB.print("Correction ");
  //printDegMinSecs(rad2deg(rf));
  //SerialUSB.println();
  return true;
}

boolean SiderealPlanets::doRiseSetTimes(double DIdeg) {
  //DI = vertical displacement in radians
  double DI = deg2rad(DIdeg);
  double SD = sin(DI);
  double CD = cos(DI);
  double CH = -(SD + (sinLat * sinDec)) / (cosLat * cosDec);
  if (CH < -1.0) return false; //circumpolar - never sets
  if (CH > 1.0) return false; // never rises
  double CA = (sinDec + (SD * sinLat)) / (CD * cosLat);
  double H = acos(CH);
  AU = acos(CA);
  double B = rad2deg(H) / 15.0;
  double A = rad2deg(RArad) / 15.0;
  LU = inRange24(24.0 + A - B);
  LD = inRange24(A + B);
  AD = inRange2PI(F2PI - AU);
  AU = inRange2PI(AU); //need for Moon rise/set
  return true;
}

double SiderealPlanets::getRiseTime(void) {
  return doLST2LT(LU);
}

double SiderealPlanets::getSetTime(void) {
  return doLST2LT(LD);
}

double SiderealPlanets::doAnomaly(double meanAnomaly, double eccentricity) {
  //Returns eccentric anomaly in degrees given the mean anomaly in degrees
  //and eccentricity for an elliptical orbit.
  double m, d, a;
  AM = deg2rad(meanAnomaly);
  boolean flag = false;
  m = AM - F2PI * floor(AM / F2PI);
  eccentricAnomaly = m;
  do {
    flag = false;
    d = eccentricAnomaly - (eccentricity * sin(eccentricAnomaly)) - m;
    if (fabs(d) >= 1.0e-9) {
      d = d / (1.0 - (eccentricity * cos(eccentricAnomaly)));
      eccentricAnomaly = eccentricAnomaly - d;
	  flag = true;
    }
  } while (flag == true);
  
  a = sqrt((1.0 + eccentricity) / (1.0 - eccentricity)) * tan(eccentricAnomaly / 2.0);
  trueAnomaly = 2.0 * atan(a);
  return rad2deg(eccentricAnomaly);
}

double SiderealPlanets::getTrueAnomaly(void) {
  //Returns true anomaly in degrees after doAnomaly() has been called.
  return rad2deg(trueAnomaly);
}

boolean SiderealPlanets::doSun(void) {
  T = (modifiedJulianDate1900() / 36525.0) + (getGMT() / 8.766e5);
  double T2 = T * T;
  double A = 1.000021359e2 * T;
  double B = 360.0 * (A - floor(A));
  double L = 2.7969668e2 + 3.025e-4 * T2 + B;
  A = 9.999736042e1 * T;
  B = 360.0 * (A - floor(A));
  meanAnomaly = 3.5847583e2 - (1.5e-4 + 3.3e-6 * T) * T2 + B;
  double eccentricity = 1.675104e-2 - 4.18e-5 * T - 1.26e-7 * T2;
  doAnomaly(meanAnomaly, eccentricity);
  
  A = 6.255209472e1 * T;
  B = 360.0 * (A - floor(A));
  double A1 = deg2rad(153.23 + B);
  A =1.251041894e2 * T;
  B = 360.0 * (A - floor(A));
  double B1b = deg2rad(216.57 + B);
  A = 9.156766028e1 * T;
  B = 360.0 * (A - floor(A));
  double C1 = deg2rad(312.69 + B);
  A = 1.236853095E3 * T;
  B = 360.0 * (A - floor(A));
  double D1 = deg2rad(350.74 + 1.44e-3 * T2 + B);
  double E1 = deg2rad(231.19 + 20.2 * T);
  A = 1.831353208e2 * T;
  B = 360.0 * (A- floor(A));
  double H1 = deg2rad(353.4 + B);
  
  double D2 = 1.34e-3 * cos(A1) + 1.54e-3 * cos(B1b) + 2e-3 * cos(C1);
  D2 = D2 + 1.79e-3 * sin(D1) + 1.78e-3 * sin(E1);
  
  double D3 = 5.43e-6 * sin(A1) + 1.575e-5 * sin(B1b);
  D3 = D3 + 1.627e-5 * sin(C1) + 3.076e-5 * cos(D1);
  D3 = D3 + 9.27e-6 * sin(H1);
  
  SR = trueAnomaly + deg2rad(L - meanAnomaly + D2);
  // Distance from Earth in Astronomical Units = int(RR * 1e5 + 0.5) / 1e5
  RR = 1.0000002 * (1.0 - eccentricity * cos(eccentricAnomaly)) + D3;
  // true geocentric longitude of the Sun
  SR = inRange2PI(SR);
  
  // Apparent ecliptic longitude
  doNutation();
  AL = rad2deg(SR) + DP - 5.69e-3;
  Ecl2RaDecDone = false;
  setEcliptic(AL, 0.0);
  doEcliptic2RAdec();
  
  return true;
}

boolean SiderealPlanets::doMoon(void) {
  T = (modifiedJulianDate1900() / 36525.0) + (getGMT() / 8.766e5);
  double T2 = T * T;
  double M1 = 2.732158213e1;
  double M2 = 3.652596407e2;
  double M3 = 2.755455094e1;
  double M4 = 2.953058868e1;
  double M5 = 2.721222039e1;
  double M6 = 6.798363307e3;
  double Q = modifiedJulianDate1900()+(getGMT() / 24.0);
  M1 = Q / M1;
  M2 = Q / M2;
  M3 = Q / M3;
  M4 = Q / M4;
  M5 = Q / M5;
  M6 = Q / M6;
  M1 = 360. * (M1 - floor(M1));
  M2 = 360. * (M2 - floor(M2));
  M3 = 360. * (M3 - floor(M3));
  M4 = 360. * (M4 - floor(M4));
  M5 = 360. * (M5 - floor(M5));
  M6 = 360. * (M6 - floor(M6));
  double ML = 2.70434164E2 + M1 - (1.133E-3 - 1.9E-6 * T) * T2;
  double MS = 3.58475833E2 + M2 - (1.5E-4 + 3.3E-6 * T) * T2;
  double MD = 2.96104608E2 + M3+(9.192E-3 + 1.44E-5 * T) * T2;
  double ME = 3.50737486E2 + M4 - (1.436E-3 - 1.9E-6 * T) * T2;
  double MF = 11.250889 + M5 - (3.211E-3 + 3E-7 * T) * T2;
  double NA = 2.59183275E2 - M6+(2.078E-3 + 2.2E-6 * T) * T2;
  double A = deg2rad(51.2 + 20.2 * T);
  double S1 = sin(A);
  double S2 = sin(deg2rad(NA));
  double B = 346.56+(132.87 - 9.1731E-3 * T) * T;
  double S3 = 3.964E-3 * sin(deg2rad(B));
  double C = deg2rad(NA + 275.05 - 2.3 * T);
  double S4 = sin(C);
  ML = ML + 2.33E-4 * S1 + S3 + 1.964E-3 * S2;
  MS = MS - 1.778E-3 * S1;
  MD = MD + 8.17E-4 * S1 + S3 + 2.541E-3 * S2;
  MF = MF + S3 - 2.4691E-2 * S2 - 4.328E-3 * S4;
  ME = ME + 2.011E-3 * S1 + S3 + 1.964E-3 * S2;
  double E = 1. - (2.495E-3 + 7.52E-6 * T) * T;
  double E2 = E * E;
  ML = deg2rad(ML);
  MS = deg2rad(MS);
  NA = deg2rad(NA);
  ME = deg2rad(ME);
  MF = deg2rad(MF);
  MD = deg2rad(MD);
  double L = 6.28875 * sin(MD) + 1.274018 * sin(2. * ME - MD);
  L = L + 6.58309e-1 * sin(2. * ME) + 2.13616e-1 * sin(2. * MD);
  L = L - E * 1.85596E-1 * sin(MS) - 1.14336E-1 * sin(2. * MF);
  L = L + 5.8793E-2 * sin(2. * (ME - MD));
  L = L + 5.7212E-2 * E * sin(2. * ME - MS - MD) + 5.332E-2 * sin(2. * ME + MD);
  L = L + 4.5874E-2 * E * sin(2. * ME - MS) + 4.1024E-2 * E * sin(MD - MS);
  L = L - 3.4718E-2 * sin(ME) - E * 3.0465E-2 * sin(MS + MD);
  L = L + 1.5326E-2 * sin(2. * (ME - MF)) - 1.2528E-2 * sin(2. * MF + MD);
  L = L - 1.098E-2 * sin(2. * MF - MD) + 1.0674E-2 * sin(4. * ME - MD);
  L = L + 1.0034E-2 * sin(3. * MD) + 8.548E-3 * sin(4. * ME - 2. * MD);
  L = L - E * 7.91E-3 * sin(MS - MD + 2. * ME) - E * 6.783E-3 * sin(2. * ME + MS);
  L = L + 5.162E-3 * sin(MD - ME) + E * 5E-3 * sin(MS + ME);
  L = L + 3.862E-3 * sin(4. * ME) + E * 4.049E-3 * sin(MD - MS + 2. * ME);
  L = L + 3.996E-3 * sin(2. * (MD + ME)) + 3.665E-3 * sin(2. * ME - 3. * MD);
  L = L + E * 2.695E-3 * sin(2. * MD - MS) + 2.602E-3 * sin(MD - 2. * (MF + ME));
  L = L + E * 2.396E-3 * sin(2. * (ME - MD) - MS) - 2.349E-3 * sin(MD + ME);
  L = L + E2 * 2.249E-3 * sin(2. * (ME - MS)) - E * 2.125E-3 * sin(2. * MD + MS);
  L = L - E2 * 2.079E-3 * sin(2. * MS) + E2 * 2.059E-3 * sin(2. * (ME - MS) - MD);
  L = L - 1.773E-3 * sin(MD + 2. * (ME - MF)) - 1.595E-3 * sin(2. * (MF + ME));
  L = L + E * 1.22E-3 * sin(4. * ME - MS - MD) - 1.11E-3 * sin(2. * (MD + MF));
  L = L + 8.92E-4 * sin(MD - 3. * ME) - E * 8.11E-4 * sin(MS + MD + 2. * ME);
  L = L + E * 7.61E-4 * sin(4. * ME - MS - 2. * MD);
  L = L + E2 * 7.04E-4 * sin(MD - 2. * (MS + ME));
  L = L + E * 6.93E-4 * sin(MS - 2. * (MD - ME));
  L = L + E * 5.98E-4 * sin(2. * (ME - MF) - MS);
  L = L + 5.5E-4 * sin(MD + 4 * ME) + 5.38E-4 * sin(4. * MD);
  L = L + E * 5.21E-4 * sin(4. * ME - MS) + 4.86E-4 * sin(2. * MD - ME);
  L = L + E2 * 7.17E-4 * sin(MD - 2. * MS);
  double MM = inRange2PI(ML + deg2rad(L)); //Moon's geocentric ecliptic longitude

  double G = 5.128189 * sin(MF) + 2.80606e-1 * sin(MD + MF);
  G = G + 2.77693E-1 * sin(MD - MF) + 1.73238E-1 * sin(2. * ME - MF);
  G = G + 5.5413E-2 * sin(2. * ME + MF - MD) + 4.6272E-2 * sin(2. * ME - MF - MD);
  G = G + 3.2573E-2 * sin(2. * ME + MF) + 1.7198E-2 * sin(2. * MD + MF);
  G = G + 9.267E-3 * sin(2. * ME + MD - MF) + 8.823E-3 * sin(2. * MD - MF);
  G = G + E * 8.247E-3 * sin(2. * ME - MS - MF) + 4.323E-3 * sin(2. * (ME - MD) - MF);
  G = G + 4.2E-3 * sin(2. * ME + MF + MD) + E * 3.372E-3 * sin(MF - MS - 2. * ME);
  G = G + E * 2.472E-3 * sin(2. * ME + MF - MS - MD);
  G = G + E * 2.222E-3 * sin(2. * ME + MF - MS);
  G = G + E * 2.072E-3 * sin(2. * ME - MF - MS - MD);
  G = G + E * 1.877E-3 * sin(MF - MS + MD) + 1.828E-3 * sin(4. * ME - MF - MD);
  G = G - E * 1.803E-3 * sin(MF + MS) - 1.75E-3 * sin(3. * MF);
  G = G + E * 1.57E-3 * sin(MD - MS - MF) - 1.487E-3 * sin(MF + ME);
  G = G - E * 1.481E-3 * sin(MF + MS + MD) + E * 1.417E-3 * sin(MF - MS - MD);
  G = G + E * 1.35E-3 * sin(MF - MS) + 1.33E-3 * sin(MF - ME);
  G = G + 1.106E-3 * sin(MF + 3. * MD) + 1.02E-3 * sin(4. * ME - MF);
  G = G + 8.33E-4 * sin(MF + 4 * ME - MD) + 7.81E-4 * sin(MD - 3. * MF);
  G = G + 6.7E-4 * sin(MF + 4 * ME - 2. * MD) + 6.06E-4 * sin(2. * ME - 3. * MF);
  G = G + 5.97E-4 * sin(2. * (ME + MD) - MF);
  G = G + E * 4.92E-4 * sin(2. * ME + MD - MS - MF) + 4.5E-4 * sin(2. * (MD - ME) - MF);
  G = G + 4.39E-4 * sin(3. * MD - MF) + 4.23E-4 * sin(MF + 2. * (ME + MD));
  G = G + 4.22E-4 * sin(2. * ME - MF - 3. * MD) - E * 3.67E-4 * sin(MS + MF + 2. * ME - MD);
  G = G - E * 3.53E-4 * sin(MS + MF + 2. * ME) + 3.31E-4 * sin(MF + 4 * ME);
  G = G + E * 3.17E-4 * sin(2. * ME + MF - MS + MD);
  G = G + E2 * 3.06E-4 * sin(2. * (ME - MS) - MF) - 2.83E-4 * sin(MD + 3. * MF);
  double W1 = 4.664E-4 * cos(NA);
  double W2 = 7.54E-5 * cos(C);
  double BM = deg2rad(G) * (1.0 - W1 - W2); // Moon's geocentric ecliptic latitude
  PMpl = 9.50724E-1 + 5.1818E-2 * cos(MD) + 9.531E-3 * cos(2. * ME - MD);
  PMpl = PMpl + 7.843E-3 * cos(2. * ME) + 2.824E-3 * cos(2. * MD);
  PMpl = PMpl + 8.57E-4 * cos(2. * ME + MD) + E * 5.33E-4 * cos(2. * ME - MS);
  PMpl = PMpl + E * 4.01E-4 * cos(2. * ME - MD - MS);
  PMpl = PMpl + E * 3.2E-4 * cos(MD - MS) - 2.71E-4 * cos(ME);
  PMpl = PMpl - E * 2.64E-4 * cos(MS + MD) - 1.98E-4 * cos(2. * MF - MD);
  PMpl = PMpl + 1.73E-4 * cos(3. * MD) + 1.67E-4 * cos(4. * ME - MD);
  PMpl = PMpl - E * 1.11E-4 * cos(MS) + 1.03E-4 * cos(4. * ME - 2. * MD);
  PMpl = PMpl - 8.4E-5 * cos(2. * MD - 2. * ME) - E * 8.3E-5 * cos(2. * ME + MS);
  PMpl = PMpl + 7.9E-5 * cos(2. * ME + 2. * MD) + 7.2E-5 * cos(4. * ME);
  PMpl = PMpl + E * 6.4E-5 * cos(2. * ME - MS + MD) - E * 6.3E-5 * cos(2. * ME + MS - MD);
  PMpl = PMpl + E * 4.1E-5 * cos(MS + ME) + E * 3.5E-5 * cos(2. * MD - MS);
  PMpl = PMpl - 3.3E-5 * cos(3. * MD - 2. * ME) - 3E-5 * cos(MD + ME);
  PMpl = PMpl - 2.9E-5 * cos(2. * (MF - ME)) - E * 2.9E-5 * cos(2. * MD + MS);
  PMpl = PMpl + E2 * 2.6E-5 * cos(2. * (ME - MS)) - 2.3E-5 * cos(2. * (MF - ME) + MD);
  PMpl = PMpl + E * 1.9E-5 * cos(4. * ME - MS - MD);
  EquatHorizontalParallax = PMpl;
  PMpl = deg2rad(PMpl); // Horizntal parallax
  // end of regular routine
  //EclLongitude = MM to AL, EclLatitude = BM
  doNutation();
  double AL = rad2deg(MM) + DP;
  //setEcliptic(MM, BM);
  EclLongitude = deg2rad(AL);
  EclLatitude = BM;
  doEcliptic2RAdec();
  return true;
}

boolean SiderealPlanets::doPlanetElements(void) {
  // 'i' in PL[i][j] is the number of the planet to compute
  // 1=Mercury, 2=Venus, 3=Mars, 4=Jupiter, 5=Saturn, 6=Uranus, 7=Neptune
  const double readData[] = {
    //Mercury
    178.179078,415.2057519,3.011e-4,0,
	75.899697,1.5554889,2.947e-4,0,
	2.0561421e-1,2.046e-5,-3e-8,0,
	7.002881,1.8608e-3,-1.83e-5,0,
	47.145944,1.1852083,1.739e-4,0,
	3.870986e-1,6.74,-0.42,
	//Venus
	342.767053,162.5533664,3.097e-4,0,
	130.163833,1.4080361,-9.764e-4,0,
	6.82069e-3,-4.774e-5,9.1e-8,0,
	3.393631,1.0058e-3,-1.0e-6,0,
	75.779647,8.9985e-1,4.1e-4,0,
	7.233316e-1,16.92,-4.4,
	//Mars
	293.737334,53.17137642,3.107e-4,0,
	3.34218203e2,1.8407584,1.299e-4,-1.19e-6,
	9.33129e-2,9.2064e-5,-7.7e-8,0,
	1.850333,-6.75e-4,1.26e-5,0,
	48.786442,7.709917e-1,-1.4e-6,-5.33e-6,
	1.5236883,9.36,-1.52,
	//Jupiter
	238.049257,8.434172183,3.347e-4,-1.65e-6,
	1.2720972e1,1.6099617,1.05627e-3,-3.43e-6,
	4.833475e-2,1.6418e-4,-4.676e-7,-1.7e-9,
	1.308736,-5.6961e-3,3.9e-6,0,
	99.443414,1.01053,3.5222e-4,-8.51e-6,
	5.202561,196.74,-9.4,
	//Saturn
	266.564377,3.398638567,3.245e-4,-5.8e-6,
	9.1098214e1,1.9584158,8.2636e-4,4.61e-6,
	5.589232e-2,-3.455e-4,-7.28e-7,7.4e-10,
	2.492519,-3.9189e-3,-1.549e-5,4.0e-8,
	112.790414,8.731951e-1,-1.5218e-4,-5.31e-6,
	9.554747,165.6,-8.88,
	//Uranus
	244.19747,1.194065406,3.16e-4,-6.0e-7,
	1.71548692e2,1.4844328,2.372e-4,-6.1e-7,
	4.63444e-2,-2.658e-5,7.7e-8,0,
	7.72464e-1,6.253e-4,3.95e-5,0,
	73.477111,4.986678e-1,1.3117e-3,0,
	19.21814,65.8,-7.19,
	//Neptune
	84.457994,6.107942056e-1,3.205e-4,-6.0e-7,
	4.6727364e1,1.4245744,3.9082e-4,-6.05e-7,
	8.99704e-3,6.33e-6,-2.0e-9,0,
	1.779242,-9.5436e-3,-9.1e-6,0,
	130.681389,1.098935,2.4987e-4,-4.718e-6,
	30.10957,62.2,-6.87
  };
  T = (modifiedJulianDate1900() / 36525.0) + (getGMT() / 8.766e5);
  double A0, A1, A2, A3, AA, B;
  int i, j, k;
  k = 0; //data array index
  for (i = 1; i < 8; i++) {
	A0 = readData[k++];
	A1 = readData[k++];
	A2 = readData[k++];
	A3 = readData[k++];
	AA = A1 * T;
	B = 360.0 * (AA - floor(AA));
	PL[i][1] = inRange360(A0 + B + (A3 * T + A2) * T * T);
	PL[i][2] = (A1 * 9.856263e-3) + (A2 + A3) / 36525.0;
	for (j = 3; j <= 6; j++) {
	  A0 = readData[k++];
	  A1 = readData[k++];
	  A2 = readData[k++];
	  A3 = readData[k++];
	  PL[i][j] = ((A3 * T + A2) * T + A1) * T + A0;
	}
	PL[i][7] = readData[k++];
	PL[i][8] = readData[k++];
	PL[i][9] = readData[k++];
  }
  return true;
}

double SiderealPlanets::getPL(int i, int j) {
  // get orbital element j for a planet i
  return PL[i][j];
}

boolean SiderealPlanets::doPlans(int IP) {
  //Calculate apparent geocentric ecliptic coordinates,
  //allowing for light travel time, for the planets
  int K, J;
  double QA, QB, QC, QD, QE, QF, QG;
  double MS, RE, LG, EC;
  double AP[8];
  double PV, LO, LP, OM, CO, IN, SO, SP, Y, PS, PD, CI, RD, LL, RH, L1, L2, EP, BP, A;
  double SA, CA, J1, J2, J3, J4, J5, J6, J7, J8, J9, JA, JB, JC;
  double  U1, U2, U3, U4, U5, U6, U7, U8, U9, UA, UB, UC, UD, UE, UF, UG, UI, UJ, UK, UL, UN, UO, UP, UQ, UR, UU, UV, UW, UX, UY, UZ;
  double  VA, VB, VC, VD, VE, VF, VG, VH, VI, VJ, VK;
  
  if (IP < 1 || IP > 7) return false; //bad planet value
  doPlanetElements();
  double LI = 0.;
  doSun();
  MS = AM; // AM is Mean Anomaly of the Sun in radians
  RE = RR;
  LG = SR + FPI; //Earth's ecliptic longitude (radians)
  for (K = 1; K < 3; K++) {
	for (J = 1; J < 8; J++) {
	  AP[J] = deg2rad(PL[J][1] - PL[J][3] - LI * PL[J][2]);
	}
	QA = 0.;
	QB = 0.;
	QC = 0.;
	QD = 0.;
	QE = 0.;
	QF = 0.;
	QG = 0.;
	if (IP == 1) {
      //Mercury
	  QA = 2.04e-3 * cos(5. * AP[2] - 2. * AP[1] + 2.1328e-1);
	  QA = QA + 1.03e-3 * cos(2. * AP[2] - AP[1] - 2.08046);
	  QA = QA + 9.1e-4 * cos(2. * AP[4] - AP[1] - 6.4582e-1);
	  QA = QA + 7.8e-4 * cos(5. * AP[2] - 3. * AP[1] + 1.7692e-1);
	  
	  QB = 7.525e-6 * cos(2. * AP[4] - AP[1] + 9.25251e-1);
	  QB = QB + 6.802e-6 * cos(5. * AP[2] - 3. * AP[1] - 4.53642);
	  QB = QB + 5.457e-6 * cos(2. * AP[2] - 2. * AP[1] - 1.24246);
	  QB = QB + 3.569e-6 * cos(5. * AP[2] - AP[1] - 1.35699);
	} else if (IP == 2) {
      //Venus
	  QC = 7.7e-4 * sin(4.1406 + T * 2.6227);
	  QC = deg2rad(QC);
	  QE = QC;
	  
	  QA = 3.13e-3 * cos(2. * MS - 2. * AP[2] - 2.587);
	  QA = QA + 1.98e-3 * cos(3. * MS - 3. * AP[2] + 4.4768e-2);
	  QA = QA + 1.36e-3 * cos(MS - AP[2] - 2.0788);
	  QA = QA + 9.6e-4 * cos(3. * MS - 2. * AP[2] - 2.3721);
	  QA = QA + 8.2e-4 * cos(AP[4] - AP[2] - 3.6318);
	  
	  QB = 2.2501e-5 * cos(2. * MS - 2. * AP[2] - 1.01592);
	  QB = QB + 1.9045e-5 * cos(3. * MS - 3.* AP[2] + 1.61577);
	  QB = QB + 6.887e-6 * cos(AP[4] - AP[2] - 2.06106);
	  QB = QB + 5.172e-6 * cos(MS - AP[2] - 5.08065e-1);
	  QB = QB + 3.62e-6 * cos(5. * MS - 4. * AP[2] - 1.81877);
	  QB = QB + 3.283e-6 * cos(4. * MS - 4. * AP[2] + 1.10851);
	  QB = QB + 3.074e-6 * cos(2. * AP[4] - 2. * AP[2] - 9.62846e-1);
	} else if (IP == 3) {
      //Mars
	  A = 3. * AP[4] - 8. * AP[3] + 4. * MS;
	  SA = sin(A);
	  CA = cos(A);
	  QC = -(1.133e-2 * SA + 9.33e-3 * CA);
	  QC = deg2rad(QC); 
	  QE = QC;
	  
	  QA = 7.05e-3 * cos(AP[4] - 8. * AP[3] - 8.5448e-1);
	  QA = QA + 6.07e-3 * cos(2. * AP[4] - AP[3] - 3.2873);
	  QA = QA + 4.45e-3 * cos(2. * AP[4] - 2. * AP[3] - 3.3492);
	  QA = QA + 3.88e-3 * cos(MS - 2. * AP[3] + 3.5771e-1);
	  QA = QA + 2.38e-3 * cos(MS - AP[3] + 6.1256e-1);
	  QA = QA + 2.04e-3 * cos(2. * MS - 3. * AP[3] + 2.7688);
	  QA = QA + 1.77e-3 * cos(3. * AP[3] - AP[2] - 1.0053);
	  QA = QA + 1.36e-3 * cos(2. * MS - 4. * AP[3] + 2.6894);
	  QA = QA + 1.04e-3 * cos(AP[4] + 3.0749e-1);
	  
	  QB = 5.3227e-5 * cos(AP[4] - AP[3] + 7.17864e-1);
	  QB = QB + 5.0989e-5 * cos(2. * AP[4] - 2. * AP[3] - 1.77997);
	  QB = QB + 3.8278e-5 * cos(2. * AP[4] - AP[3] - 1.71617);
	  QB = QB + 1.5996e-5 * cos(MS - AP[3] - 9.69618e-1);
	  QB = QB + 1.4764e-5 * cos(2. * MS - 3. * AP[3] + 1.19768);
	  QB = QB + 8.966e-6 * cos(AP[4] - 2. * AP[3] + 7.61225e-1);
	  QB = QB + 7.914e-6 * cos(3. * AP[4] - 2. * AP[3] - 2.43887);
	  QB = QB + 7.004e-6 * cos(2. * AP[4] - 3. * AP[3] - 1.79573);
	  QB = QB + 6.62e-6 * cos(MS - 2. * AP[3] + 1.97575);
	  QB = QB + 4.93e-6 * cos(3. * AP[4] - 3. * AP[3] - 1.33069);
	  QB = QB + 4.693e-6 * cos(3. * MS - 5. * AP[3] + 3.32665);
	  QB = QB + 4.571e-6 * cos(2. * MS - 4. * AP[3] + 4.27086);
	  QB = QB + 4.409e-6 * cos(3. * AP[4] - AP[3] - 2.02158);
	} else {
      //4=Jupiter, 5=Saturn, 6=Uranus, and 7=Neptune
	  J1 = T / 5.0 + 0.1;
	  J2 = inRange2PI(4.14473 + 5.29691e1 * T);
	  J3 = inRange2PI(4.641118 + 2.132991e1 * T);
	  J4 = inRange2PI(4.250177 + 7.478172 * T);
	  J5 = 5.0 * J3 - 2.0 * J2;
	  J6 = 2.0 * J2 - 6.0 * J3 + 3.0 * J4;
	  if ((IP == 4) || (IP == 5)) {
        //Common code for Jupiter and Saturn
		J7 = J3 - J2;
		U1 = sin(J3);
		U2 = cos(J3);
		U3 = sin(2.0 * J3);
		U4 = cos(2.0 * J3);
		U5 = sin(J5);
		U6 = cos(J5);
		U7 = sin(2.0 * J5);
		U8 = sin(J6);
		U9 = sin(J7);
		UA = cos(J7);
		UB = sin(2.0 * J7);
		UC = cos(2.0 * J7);
		UD = sin(3.0 * J7);
		UE = cos(3.0 * J7);
		UF = sin(4.0 * J7);
		UG = cos(4.0 * J7);
		VH = cos(5.0 * J7);
	  } else {
        //Common code for Uranus and Neptune
		J8 = inRange2PI(1.46205 + 3.81337 * T);
		J9 = 2. * J8 - J4;
		VJ = sin(J9);
		UU = cos(J9);
		UV = sin(2. * J9);
		UW = cos(2. * J9);
	  }
	  
      if (IP == 4) {
        //Jupiter
		QC = (3.31364e-1 - (1.0281e-2 + 4.692e-3 * J1) * J1) * U5;
		QC = QC + (3.228e-3 - (6.4436e-2 - 2.075e-3 * J1) * J1) * U6;
		QC = QC - (3.083e-3 + (2.75e-4 - 4.89e-4 * J1) * J1) * U7;
		QC = QC + 2.472e-3 * U8 + 1.3619e-2 * U9 + 1.8472e-2 * UB;
		QC = QC + 6.717e-3 * UD + 2.775e-3 * UF + 6.417e-3 * UB * U1;
		QC = QC + (7.275e-3 - 1.253e-3 * J1) * U9 * U1 + 2.439e-3 * UD * U1;
		QC = QC - (3.5681e-2 + 1.208e-3 * J1) * U9 * U2 - 3.767e-3 * UC * U1;
		QC = QC - (3.3839e-2 + 1.125e-3 * J1) * UA * U1 - 4.261e-3 * UB * U2;
		QC = QC + (1.161e-3 * J1 - 6.333e-3) * UA * U2 + 2.178e-3 * U2;
		QC = QC - 6.675e-3 * UC * U2 - 2.664e-3 * UE * U2 - 2.572e-3 * U9 * U3;
		QC = QC - 3.567e-3 * UB * U3 + 2.094e-3 * UA * U4 + 3.342e-3 * UC * U4;
		QC = deg2rad(QC);
		
		QD = (3606. + (130. - 43. * J1) * J1) * U5 + (1289. - 580. * J1) * U6;
		QD = QD - 6764. * U9 * U1 - 1110. * UB * U1 - 224. * UD * U1 - 204. * U1;
		QD = QD + (1284. + 116. * J1) * UA * U1 + 188. * UC * U1;
		QD = QD + (1460. + 130. * J1) * U9 * U2 + 224. * UB * U2 - 817. * U2;
		QD = QD + 6074. * U2 * UA + 992. * UC * U2 + 508. * UE * U2 + 230. * UG * U2;
		QD = QD + 108. * VH * U2 - (956. + 73. * J1) * U9 * U3 + 448. * UB * U3;
		QD = QD + 137. * UD * U3 + (108. * J1 - 997.) * UA * U3 + 480. * UC * U3;
		QD = QD + 148. * UE * U3 + (99. * J1 - 956.) * U9 * U4 + 490. * UB * U4;
		QD = QD + 158. * UD * U4 + 179. * U4 + (1024. + 75. * J1) * UA * U4;
		QD = QD - 437. * UC * U4 - 132. * UE * U4;
		QD = QD * 1.0e-7;
		
		VK = (7.192e-3 - 3.147e-3 * J1) * U5 - 4.344e-3 * U1;
		VK = VK + (J1 * (1.97e-4 * J1 - 6.75e-4) - 2.0428e-2) * U6;
		VK = VK + 3.4036e-2 * UA * U1 + (7.269e-3 + 6.72e-4 * J1) * U9 * U1;
		VK = VK + 5.614e-3 * UC * U1 + 2.964e-3 * UE * U1 + 3.7761e-2 * U9 * U2;
		VK = VK + 6.158e-3 * UB * U2 - 6.603e-3 * UA * U2 - 5.356e-3 * U9 * U3;
		VK = VK + 2.722e-3 * UB * U3 + 4.483e-3 * UA * U3;
		VK = VK - 2.642e-3 * UC * U3 + 4.403e-3 * U9 * U4;
		VK = VK - 2.536e-3 * UB * U4 + 5.547e-3 * UA * U4 - 2.689e-3 * UC * U4;
		QE = QC - (deg2rad(VK) / PL[IP][4]);
		
		QF = 205. * UA - 263. * U6 + 693. * UC + 312. * UE + 147. * UG + 299. * U9 * U1;
		QF = QF + 181. * UC * U1 + 204. * UB * U2 + 111. * UD * U2 - 337. * UA * U2;
		QF = QF - 111. * UC * U2;
		QF = QF * 1.0e-6;
	  } else if (IP == 5) {
        //Saturn
		UI = sin(3. * J3);
		UJ = cos(3. * J3);
		UK = sin(4. * J3);
		UL = cos(4. * J3);
		VI = cos(2. * J5);
		UN = sin(5. * J7);
		J8 = J4 - J3;
		UO = sin(2. * J8);
		UP = cos(2. * J8);
		UQ = sin(3. * J8);
		UR = cos(3. * J8);
		
		QC = 7.581e-3 * U7 - 7.986e-3 * U8 - 1.48811e-1 * U9;
		QC = QC - (8.14181e-1 - (1.815e-2 - 1.6714e-2 * J1) * J1) * U5;
		QC = QC - (1.0497e-2 - (1.60906e-1 - 4.1e-3 * J1) * J1) * U6;
		QC = QC - 1.5208e-2 * UD - 6.339e-3 * UF - 6.244e-3 * U1;
		QC = QC - 1.65e-2 * UB * U1 - 4.0786e-2 * UB;
		QC = QC + (8.931e-3 + 2.728e-3 * J1) * U9 * U1 - 5.775e-3 * UD * U1;
		QC = QC + (8.1344e-2 + 3.206e-3 * J1) * UA * U1 + 1.5019e-2 * UC * U1;
		QC = QC + (8.5581e-2 + 2.494e-3 * J1) * U9 * U2 + 1.4394e-2 * UC * U2;
		QC = QC + (2.5328e-2 - 3.117e-3 * J1) * UA * U2 + 6.319e-3 * UE * U2;
		QC = QC + 6.369e-3 * U9 * U3 + 9.156e-3 * UB * U3 + 7.525e-3 * UQ * U3;
		QC = QC - 5.236e-3 * UA * U4 - 7.736e-3 * UC * U4 - 7.528e-3 * UR * U4;
		QC = deg2rad(QC);
		
		QD = (-7927. + (2548. + 91. * J1) * J1) * U5;
		QD = QD + (13381. + (1226. - 253. * J1) * J1) * U6 + (248. - 121. * J1) * U7;
		QD = QD - (305. + 91. * J1) * VI + 412. * UB + 12415. * U1;
		QD = QD + (390. - 617. * J1) * U9 * U1 + (165. - 204. * J1) * UB * U1;
		QD = QD + 26599. * UA * U1 - 4687. * UC * U1 - 1870. * UE * U1 - 821. * UG * U1;
		QD = QD - 377. * VH * U1 + 497. * UP * U1 + (163. - 611. * J1) * U2;
		QD = QD - 12696. * U9 * U2 - 4200. * UB * U2 - 1503. * UD * U2 - 619. * UF * U2;
		QD = QD - 268. * UN * U2 - (282. + 1306. * J1) * UA * U2;
		QD = QD + (-86. + 230. * J1) * UC * U2 + 461. * UO * U2 - 350. * U3;
		QD = QD + (2211. - 286. * J1) * U9 * U3 - 2208. * UB * U3 - 568. * UD * U3;
		QD = QD - 346. * UF * U3 - (2780. + 222. * J1) * UA * U3;
		QD = QD + (2022. + 263. * J1) * UC * U3 + 248. * UE * U3 + 242. * UQ * U3;
		QD = QD + 467. * UR * U3 - 490. * U4 - (2842. + 279. * J1) * U9 * U4;
		QD = QD + (128. + 226. * J1) * UB * U4 + 224. * UD * U4;
		QD = QD + (-1594. + 282. * J1) * UA * U4 + (2162. - 207. * J1) * UC * U4;
		QD = QD + 561. * UE * U4 + 343. * UG * U4 + 469. * UQ * U4 - 242. * UR * U4;
		QD = QD - 205. * U9 * UI + 262. * UD * UI + 208. * UA * UJ - 271. * UE * UJ;
		QD = QD - 382. * UE * UK - 376. * UD * UL;
		QD = QD * 1.0e-7;
		
		VK = (7.7108e-2 + (7.186e-3 - 1.533e-3 * J1) * J1) * U5;
		VK = VK - 7.075e-3 * U9;
		VK = VK + (4.5803e-2 - (1.4766e-2 + 5.36e-4 * J1) * J1) * U6;
		VK = VK - 7.2586e-2 * U2 - 7.5825e-2 * U9 * U1 - 2.4839e-2 * UB * U1;
		VK = VK - 8.631e-3 * UD * U1 - 1.50383e-1 * UA * U2;
		VK = VK + 2.6897e-2 * UC * U2 + 1.0053e-2 * UE * U2;
		VK = VK - (1.3597e-2 + 1.719e-3 * J1) * U9 * U3 + 1.1981e-2 * UB * U4;
		VK = VK - (7.742e-3 - 1.517e-3 * J1) * UA * U3;
		VK = VK + (1.3586e-2 - 1.375e-3 * J1) * UC * U3;
		VK = VK - (1.3667e-2 - 1.239e-3 * J1) * U9 * U4;
		VK = VK + (1.4861e-2 + 1.136e-3 * J1) * UA * U4;
		VK = VK - (1.3064e-2 + 1.628e-3 * J1) * UC * U4;
		QE = QC - (deg2rad(VK) / PL[IP][4]);
		QF = 572. * U5 - 1590. * UB * U2 + 2933. * U6 - 647. * UD * U2;
		QF = QF + 33629. * UA - 344. * UF * U2 - 3081. * UC + 2885. * UA * U2;
		QF = QF - 1423. * UE + (2172. + 102. * J1) * UC * U2 - 671. * UG;
		QF = QF + 296. * UE * U2 - 320. * VH - 267. * UB * U3 + 1098. * U1;
		QF = QF - 778. * UA * U3 - 2812. * U9 * U1 + 495. * UC * U3 + 688. * UB * U1;
		QF = QF + 250. * UE * U3 - 393. * UD * U1 - 856. * U9 * U4 - 228. * UF * U1;
		QF = QF + 441. * UB * U4 + 2138. * UA * U1 + 296. * UC * U4 - 999. * UC * U1;
		QF = QF + 211. * UE * U4 - 642. * UE * U1 - 427. * U9 * UI - 325. * UG * U1;
		QF = QF + 398. * UD * UI - 890. * U2 + 344. * UA * UJ + 2206. * U9 * U2;
		QF = QF - 427. * UE * UJ;
		QF = QF * 1.0e-6;
		
		QG = 7.47e-4 * UA * U1 + 1.069e-3 * UA * U2 + 2.108e-3 * UB * U3;
		QG = QG + 1.261e-3 * UC * U3 + 1.236e-3 * UB * U4 - 2.075e-3 * UC * U4;
		QG = deg2rad(QG);
	  } else if (IP == 6) {
        //Uranus
		JA = J4 - J2;
		JB = J4 - J3;
		JC = J8 - J4;
		QC = (8.64319e-1 - 1.583e-3 * J1) * VJ;
		QC = QC + (8.2222e-2 - 6.833e-3 * J1) * UU + 3.6017e-2 * UV;
		QC = QC - 3.019e-3 * UW + 8.122e-3 * sin(J6);
		QC = deg2rad(QC);
		VK = 1.20303e-1 * VJ + 6.197e-3 * UV;
		VK = VK + (1.9472e-2 - 9.47e-4 * J1) * UU;
		QE = QC - (deg2rad(VK) / PL[IP][4]);
		QD = (163. * J1 - 3349.) * VJ + 20981. * UU + 1311. * UW;
		QD = QD * 1.0e-7;
		QF = -3.825e-3 * UU;
		QA = (-3.8581e-2 + (2.031e-3 - 1.91e-3 * J1) * J1) * cos(J4 + JB);
		QA = QA + (1.0122e-2 - 9.88e-4 * J1) * sin(J4 + JB);
		A = (3.4964e-2 - (1.038e-3 - 8.68e-4 * J1) * J1) * cos(2. * J4 + JB);
        QA = A + QA + 5.594e-3 * sin(J4 + 3. * JC) - 1.4808e-2 * sin(JA);
        QA = QA - 5.794e-3 * sin(JB) + 2.347e-3 * cos(JB);
		QA = QA + 9.872e-3 * sin(JC) + 8.803e-3 * sin(2. * JC);
		QA = QA - 4.308e-3 * sin(3. * JC);
		
		UX = sin(JB);
		UY = cos(JB);
		UZ = sin(J4);
		VA = cos(J4);
		VB = sin(2. * J4);
		VC = cos(2. * J4);
		QG = (4.58e-4 * UX - 6.42e-4 * UY - 5.17e-4 * cos(4. * JC)) * UZ;
		QG = QG - (3.47e-4 * UX + 8.53e-4 * UY + 5.17e-4 * sin(4. * JB)) * VA;
		QG = QG + 4.03e-4 * (cos(2. * JC) * VB + sin(2. * JC) * VC);
		QG = deg2rad(QG);
		
		QB = -25948. + 4985. * cos(JA) - 1230. * VA + 3354. * UY;
		QB = QB + 904. * cos(2. * JC) + 894. * (cos(JC) - cos(3. * JC));
		QB = QB + (5795. * VA - 1165. * UZ + 1388. * VC) * UX;
		QB = QB + (1351. * VA + 5702. * UZ + 1388. * VB) * UY;
		QB = QB * 1.0e-6;
	  } else if (IP == 7) {
        //Neptune
		JA = J8 - J2;
		JB = J8 - J3;
		JC = J8 - J4;
		QC = (1.089e-3 * J1 - 5.89833e-1) * VJ;
		QC = QC + (4.658e-3 * J1 - 5.6094e-2) * UU - 2.4286e-2 * UV;
		QC = deg2rad(QC);
		
		VK = 2.4039e-2 * VJ - 2.5303e-2 * UU + 6.206e-3 * UV;
		
		VK = VK - 5.992e-3 * UW;
		QE = QC - (deg2rad(VK) / PL[IP][4]);
		QD = 4389. * VJ + 1129. * UV + 4262. * UU + 1089. * UW;
		QD = QD * 1.0e-7;
		QF = 8189. * UU - 817. * VJ + 781. * UW;
		QF = QF * 1.0e-6;
		VD = sin(2. * JC);
		VE = cos(2. * JC);
		VF = sin(J8);
		VG = cos(J8);
		QA = -9.556e-3 * sin(JA) - 5.178e-3 * sin(JB);
		QA = QA + 2.572e-3 * VD - 2.972e-3 * VE * VF - 2.833e-3 * VD * VG;
		QG = 3.36e-4 * VE * VF + 3.64e-4 * VD * VG;
		QG = deg2rad(QG);
		QB = -40596. + 4992. * cos(JA) + 2744. * cos(JB);
		QB = QB + 2044. * cos(JC) + 1051. * VE;
		QB = QB * 1.0e-6;
	  }
	}
	EC = PL[IP][4] + QD;
	AM = AP[IP] + QE;
	doAnomaly(rad2deg(AM), EC);
	PV = (PL[IP][7] + QF) * (1.0 - EC * EC) / (1.0 + EC * cos(trueAnomaly));
	LP = rad2deg(trueAnomaly) + PL[IP][3] + rad2deg(QC - QE);
	LP = deg2rad(LP);
	OM = deg2rad(PL[IP][6]);
	LO = LP - OM;
	SO = sin(LO);
	CO = cos(LO);
	IN = deg2rad(PL[IP][5]);
	PV = PV + QB;
	SP = SO * sin(IN);
	Y = SO * cos(IN);
	PS = asin(SP) + QG;
	SP = sin(PS);
	PD = atan(Y / CO) + OM + deg2rad(QA);
	if (CO < 0.) PD = PD + FPI;
	if (PD > F2PI) PD = PD - F2PI;
	CI = cos(PS);
	RD = PV * CI;
	LL = PD - LG;
	RH = RE * RE + PV * PV - 2.0 * RE * PV * CI * cos(LL);
	RH = sqrt(RH); //Earth-planet distance in AU
	LI = RH * 5.775518e-3;
	if (K == 1) {
      L0 = PD; //heliocentric ecliptic longitude (radians)
	  V0 = RH; //distance from Earth in AU
	  S0 = PS; //heliocentric latitude in radians
	  P0 = PV; //radius vector (AU)
	}
  }

  L1 = sin(LL);
  L2 = cos(LL);
  if (IP < 3) {
    EP = atan(-1.0 * RD * L1 / (RE - RD * L2)) + LG + FPI;
  } else {
    EP = atan(RE * L1 / (RD - RE * L2)) + PD;
  }
  EP = inRange2PI(EP); //geocentric ecliptic longitude
  BP = atan(RD * SP * sin(EP - PD) / (CI * RE * L1)); //geocentric ecliptic latitude
  //end of standard routine here
  //EclLongitude = EP, EclLatitude = BP
  double EPtemp = EP;
  double BPtemp = BP;
  doNutation();
  EP = EP + deg2rad(DP);
  A = LG + FPI - EP;
  double B = cos(A);
  double C = sin(A);
  EP = EP - (9.9387e-5 * B / cos(BP));
  BP = BP - (9.9387e-5 * C * sin(BP));
  //setEcliptic(EP, BP);
  EclLongitude = EP;
  EclLatitude = BP;
  doEcliptic2RAdec();
  EclLongitude = EPtemp;
  EclLatitude = BPtemp;
  return true;
}

double SiderealPlanets::getHelioLong(void) {
  return rad2deg(L0);
}

double SiderealPlanets::getHelioLat(void) {
  return rad2deg(S0);
}

double SiderealPlanets::getRadiusVec(void) {
  return (P0);
}

double SiderealPlanets::getDistance(void) {
  return (V0);
}

boolean SiderealPlanets::doMercury(void) {
  doPlans(1);
  return true;
}

boolean SiderealPlanets::doVenus(void) {
  doPlans(2);
  return true;
}

boolean SiderealPlanets::doMars(void) {
  doPlans(3);
  return true;
}

boolean SiderealPlanets::doJupiter(void) {
  doPlans(4);
  return true;
}

boolean SiderealPlanets::doSaturn(void) {
  doPlans(5);
  return true;
}

boolean SiderealPlanets::doUranus(void) {
  doPlans(6);
  return true;
}

boolean SiderealPlanets::doNeptune(void) {
  doPlans(7);
  return true;
}

boolean SiderealPlanets::doSunRiseSetTimes(void) {
  double DI = 1.454441e-2;
  double tmpGMT = GMTtime;
  GMTtime = (12.0 + (TimeZoneOffset + DSToffset));
  doSun();
  GMTtime = tmpGMT;
  if (doRiseSetTimes(rad2deg(DI)) == false) return false;
  
  double LA = LU; //LST of rising - first guesstimate
  double LB = LD; //LST of setting - first guesstimate
  double GU = doLST2GMT(LA);
  double GD = doLST2GMT(LB);
  
  GMTtime = GU;
  double DN = mjd1900;
  double A = GMTtime + TimeZoneOffset + DSToffset;
  if (A > 24.) mjd1900 -= 1;
  if (A < 0.)  mjd1900 += 1;
  doSun();
  mjd1900 = DN;
  GMTtime = tmpGMT;
  if (doRiseSetTimes(rad2deg(DI)) == false) return false;
  LA = LU; //AA = AU;
  
  GMTtime = GD;
  A = GMTtime + TimeZoneOffset + DSToffset;
  if (A > 24.) mjd1900 -= 1;
  if (A < 0.)  mjd1900 += 1;
  doSun();
  mjd1900 = DN;
  GMTtime = tmpGMT;
  if (doRiseSetTimes(rad2deg(DI)) == false) return false;
  
  LB = LD; //AB = AD;
  LU = LA;
  return true;
}

double SiderealPlanets::getSunriseTime(void) {
  return getRiseTime();
}

double SiderealPlanets::getSunsetTime(void) {
  return getSetTime();
}

boolean SiderealPlanets::doMoonRiseSetTimes(void) {
  //int ER[10];
  double DN, DI, A, TH, AA, AB, GU, GD;
  double tmpGMT = GMTtime;
  GMTtime = (12.0 + (TimeZoneOffset + DSToffset)); //Set to local mid-day
  
  //local rise-set routine
  doMoon(); //Already does nutation too
  TH = 2.7249e-1 * sin(PMpl);
  DI = TH + 9.8902e-3 - PMpl;
  // return if moon doesn't cross horizon
  if (doRiseSetTimes(rad2deg(DI)) == false) {
	  GMTtime = tmpGMT;
	  return false;
  }

  double LA = LU; //LST of rising - first guesstimate
  double LB = LD; //LST of setting - first guesstimate
  for(int K=1; K <= 3; K++) {
    // local sidereal time to local civil time
    GU = doLST2GMT(LA);
    // local sidereal time to local civil time
    GD = doLST2GMT(LB);
	//find a better time of rising
    GMTtime = GU;
    // find time
    DN = mjd1900;
    A = GMTtime + TimeZoneOffset + DSToffset;
    if (A > 24.) mjd1900 -= 1;
    if (A < 0.) mjd1900 += 1;
    //local rise-set routine
    doMoon(); //Already does nutation too
    TH = 2.7249e-1 * sin(PMpl);
    DI = TH + 9.8902e-3 - PMpl;
    if (doRiseSetTimes(rad2deg(DI)) == false) {
		mjd1900=DN;
		GMTtime = tmpGMT;
		return false;
	}
    mjd1900=DN;
    //find a better time of setting
    LA = LU;
    //AA = AU;
    GMTtime = GD;
    // find time
    DN = mjd1900;
    A = GMTtime + TimeZoneOffset + DSToffset;
    if (A > 24.) mjd1900 -= 1;
    if (A < 0.) mjd1900 += 1;
    //local rise-set routine
    doMoon(); //Already does nutation too
    TH = 2.7249e-1 * sin(PMpl);
    DI = TH + 9.8902e-3 - PMpl;
    if (doRiseSetTimes(rad2deg(DI)) == false) {
		mjd1900=DN;
		GMTtime = tmpGMT;
		return false;
	}
    mjd1900=DN;
    LB = LD;
    //AB = AD;
  }
  //AU = AA;
  //AD = AB;
  LU = LA;
  LD = LB;
  GMTtime = tmpGMT;
  return true;
}

double SiderealPlanets::getMoonriseTime(void) {
  return getRiseTime();
}

double SiderealPlanets::getMoonsetTime(void) {
  return getSetTime();
}

void SiderealPlanets::printDegMinSecs(double n) {
  boolean sign = (n < 0.);
  if (sign) n = -n;
  long lsec = n * 360000.0;
  long deg = lsec / 360000;
  long min = (lsec - (deg * 360000)) / 6000;
  float secs = (lsec - (deg * 360000) - (min * 6000)) / 100.;
  if (sign) Serial.print("-");
  Serial.print(deg); Serial.print(":");
  Serial.print(min); Serial.print(":");
  Serial.print(abs(secs)); Serial.print(" ");
}
