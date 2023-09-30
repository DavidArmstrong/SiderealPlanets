/******************************************************************************
SiderealPlanets.cpp
Sidereal Planets Arduino Library C++ source
David Armstrong
Version 1.3.0 - September 30, 2023
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
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

// Public Methods //////////////////////////////////////////////////////////
// Start by doing any setup, and verifying that doubles are supported
boolean SiderealPlanets::begin(void) {
  //float  fpi = 3.14159265358979;
  //double dpi = 3.14159265358979;
  //if (dpi == (double)fpi ) return false; //double and float are the same here!

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

double SiderealPlanets::getLatitude(void) {
  return decLat;
}

double SiderealPlanets::getLongitude(void) {
  return decLong;
}

double SiderealPlanets::getGMT(void) {
  return GMTtime;
}

double SiderealPlanets::getLT(void) {
  double localStandardTime = GMTtime + TimeZoneOffset;
  if (useDST) localStandardTime += 1;
  return inRange24(localStandardTime);
}

double SiderealPlanets::getLocalSiderealTime(void) {
  LocalSiderealTime = inRange24(getGMTsiderealTime() + (decLong / 15.0));
  return LocalSiderealTime;
}

double SiderealPlanets::getGMTsiderealTime(void) {
  modifiedJulianDate1900();
  double days = ((long)(mjd1900 - 0.5)) + 0.5;
  double t = (days / 36525.0) - 1.;
  double r0 = t * (5.13366e-2 + (t * (2.586222e-5 - (t * 1.722e-9))));
  double r1 = 6.697374558 + (2400.0 * (t - ((GMTyear - 2000.0) / 100.0)));
  double t0 = inRange24(r0 + r1);
  GMTsiderealTime = inRange24((GMTtime * 1.002737908) + t0);
  return GMTsiderealTime;
}

double SiderealPlanets::doLST2LT(double localSiderealTime) {
  //for computing rise/set times
  modifiedJulianDate1900();
  double days = ((long)(mjd1900 - 0.5)) + 0.5;
  double t = (days / 36525.0) - 1.;
  double r0 = t * (5.13366e-2 + (t * (2.586222e-5 - (t * 1.722e-9))));
  double r1 = 6.697374558 + (2400.0 * (t - ((GMTyear - 2000.0) / 100.0)));
  double t0 = inRange24(r0 + r1);
  double julianCenturies1900 = inRange24(t0 - (DSToffset + TimeZoneOffset) * 1.002737908);
  double GSTdecimalhours = inRange24(localSiderealTime - (decLong / 15.0));
  if (GSTdecimalhours < julianCenturies1900) GSTdecimalhours += 24.;
  double localTimeDecimalHours = inRange24((GSTdecimalhours - julianCenturies1900) * 9.972695677e-1);
  return localTimeDecimalHours;
}

double SiderealPlanets::doLST2GMT(double localSiderealTime) {
  //for computing rise/set times of Sun
  return doLST2LT(localSiderealTime) - (DSToffset + TimeZoneOffset);
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
  //cosDec = cos(DeclinationRad);
  return true;
}

boolean SiderealPlanets::doNutation(void) {
  //t = julian centuries since 2000 jan 1.5
  if (nutationDone == true) return true;
  double t = (modifiedJulianDate1900()) / 36525.0;
  double t2 = t * t;
  double a = 1.000021358e2 * t;
  double b = 360.0 * (a - floor(a));
  double L1_local = 2.796967e2 + 3.03e-4 * t2 + b;
  double L2_local = 2.0 * deg2rad(L1_local);
  a = 1.336855231e3 * t;
  b = 360. * (a - floor(a));
  double d1 = 2.704342e2 - 1.133e-3 * t2 + b;
  double d2 = 2.0 * deg2rad(d1);
  a = 9.999736056e1 * t;
  b = 360.0 * (a - floor(a));
  double M1_local = 3.584758e2 - 1.5e-4 * t2 + b;
  M1_local = deg2rad(M1_local);
  a = 1.325552359e3 * t;
  b = 360.0 * (a - floor(a));
  double M2_local = 2.961046e2 + 9.192e-3 * t2 + b;
  M2_local = deg2rad(M2_local);
  a = 5.372616667 * t;
  b = 360. * (a - floor(a));
  double N1_local = 2.591833e2 + 2.078e-3 * t2 - b;
  N1_local = deg2rad(N1_local);
  double N2_local = 2. * N1_local;
  
  nutationInLongitude = (-17.2327 - 1.737e-2 * t) * sin(N1_local);
  nutationInLongitude = nutationInLongitude + (-1.2729 - 1.3e-4 * t) * sin(L2_local) + 2.088e-1 * sin(N2_local);
  nutationInLongitude = nutationInLongitude - 2.037e-1 * sin(d2) + (1.261e-1 - 3.1e-4 * t) * sin(M1_local);
  nutationInLongitude = nutationInLongitude + 6.75E-2 * sin(M2_local) - (4.97e-2 - 1.2e-4 * t) * sin(L2_local + M1_local);
  nutationInLongitude = nutationInLongitude - 3.42e-2 * sin(d2 - N1_local) - 2.61e-2 * sin(d2 + M2_local);
  nutationInLongitude = nutationInLongitude + 2.14e-2 * sin(L2_local - M1_local) - 1.49e-2 * sin(L2_local - d2 + M2_local);
  nutationInLongitude = nutationInLongitude + 1.24E-2 * sin(L2_local - N1_local) + 1.14e-2 * sin(d2 - M2_local);
  
  nutationInObliquity = (9.21 + 9.1E-4 * t) * cos(N1_local);
  nutationInObliquity = nutationInObliquity + (5.522e-1 - 2.9e-4 * t) * cos(L2_local) - 9.04e-2 * cos(N2_local);
  nutationInObliquity = nutationInObliquity + 8.84e-2 * cos(d2) + 2.16e-2 * cos(L2_local + M1_local);
  nutationInObliquity = nutationInObliquity + 1.83e-2 * cos(d2 - N1_local) + 1.13e-2 * cos(d2 + M2_local);
  nutationInObliquity = nutationInObliquity + 9.3e-3 * cos(L2_local - M1_local) - 6.6e-3 * cos(L2_local - N1_local);
  
  nutationInLongitude = nutationInLongitude / 3600.0;
  nutationInObliquity = nutationInObliquity / 3600.0;
  nutationDone = true;
  return true;
}

double SiderealPlanets::getDP(void) {
  return nutationInLongitude;
}
double SiderealPlanets::getDO(void) {
  return nutationInObliquity;
}

double SiderealPlanets::doObliquity(void) {
  //Always include Nutation when computing Obliquity
  if (obliquityDone == true) return obliquityEcliptic;
  doNutation();
  //modifiedJulianDate1900(); is mjd1900
  double t = (modifiedJulianDate1900() / 36525.0) - 1.0;
  double a = (46.815 + (0.0006 - 0.00181 * t) * t) * t;
  a = a / 3600.0;
  obliquityEcliptic = 23.43929167 - a + nutationInObliquity;
  obliquityDone = true;
  return obliquityEcliptic;
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
	double e = deg2rad(obliquityEcliptic);
	sineObliquity = sin(e);
	cosineObliquity = cos(e);
	Ecl2RaDecDone = true;
  }
  double CY_local = cos(EclLatitude);
  double SY_local = sin(EclLatitude);
  if (abs(CY_local) < 1e-20) CY_local = 1e-20;
  double TY_local = SY_local / CY_local;
  double CX_local = cos(EclLongitude);
  double SX_local = sin(EclLongitude);
  double S_local = (SY_local * cosineObliquity) - (CY_local * sineObliquity * SX_local * (-1.));
  DeclinationRad = asin(S_local);
  sinDec = S_local;
  double A_local = (SX_local * cosineObliquity) + (TY_local * sineObliquity * (-1.));
  RArad = atan(A_local / CX_local);
  if (CX_local < 0.) RArad += FPI;
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
  // multiply otherPrecessionMatrix by cv
  for(j = 1; j < 4; j++) {
    sm = 0.0;
	for(i = 1; i < 4; i++) {
      sm += otherPrecessionMatrix[i][j] * cv[i];
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
  // multiply precessionMatrix by cv
  for(j = 1; j < 4; j++) {
    sm = 0.0;
	for(i = 1; i < 4; i++) {
      sm += precessionMatrix[i][j] * cv[i];
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
    precessionMatrix[1][1] = c1 * c3 * c2 - s1 * s2;
	otherPrecessionMatrix[1][1] = precessionMatrix[1][1];
    precessionMatrix[1][2] = -s1 * c3 * c2 - c1 * s2;
	otherPrecessionMatrix[2][1] = precessionMatrix[1][2];
    precessionMatrix[1][3] = -s3 * c2;
	otherPrecessionMatrix[3][1] = precessionMatrix[1][3];
    precessionMatrix[2][1] = c1 * c3 * s2 + s1 * c2;
	otherPrecessionMatrix[1][2] = precessionMatrix[2][1];
    precessionMatrix[2][2] = -s1 * c3 * s2 + c1 * c2;
	otherPrecessionMatrix[2][2] = precessionMatrix[2][2];
    precessionMatrix[2][3] = -s3 * s2;
	otherPrecessionMatrix[3][2] = precessionMatrix[2][3];
    precessionMatrix[3][1] = c1 * s3;
	otherPrecessionMatrix[1][3] = precessionMatrix[3][1];
    precessionMatrix[3][2] = -s1 * s3;
	otherPrecessionMatrix[2][3] = precessionMatrix[3][2];
    precessionMatrix[3][3] = c3;
	otherPrecessionMatrix[3][3] = precessionMatrix[3][3];
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

float SiderealPlanets::getLunarLuminance() {
  // Assumes user has called DoMoon() first, before calling this function
  double tmpRArad, tmpRAdec, tmpDeclinationRad, tmpDeclinationDec;
  float SD_local, CD_local, D_local, Irad, K_local, FMoon;
  // Save off current RA/Dec, which we assume is for the Moon
  tmpRArad = RArad;
  tmpRAdec = RAdec;
  tmpDeclinationRad = DeclinationRad;
  tmpDeclinationDec = DeclinationDec;
  
  // Call Sun routine
  doSun();
  CD_local = cos(moonGeocentricEclipticLongitude - sunTrueGeocentricLongitude) * cos(moonGeocentricEclipticLatitude);
  D_local = 1.570796327 - asin(CD_local);
  SD_local =sin(D_local);
  Irad = 1.468e-1 * SD_local * (1.0 - 5.49e-2 * sin(moonMeanAnomaly));
  Irad = Irad / (1.0 - 1.67e-2 * sin(sunMeanAnomaly));
  Irad = FPI - D_local - deg2rad(Irad);
  K_local = (1.0 + cos(Irad)) / 2.0;
  FMoon = int(K_local * 1000.0 + 0.5) / 1000.0;
  // Restore RA/Dec
  RArad = tmpRArad;
  RAdec = tmpRAdec;
  DeclinationRad = tmpDeclinationRad;
  DeclinationDec = tmpDeclinationDec;
  return FMoon * 100.0;
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
  //horizonVerticalDisplacement = vertical displacement in radians
  double horizonVerticalDisplacement = deg2rad(DIdeg);
  double SD_local = sin(horizonVerticalDisplacement);
  double CD_local = cos(horizonVerticalDisplacement);
  double CH_local = -(SD_local + (sinLat * sinDec)) / (cosLat * cosDec);
  if (CH_local < -1.0) return false; //circumpolar - never sets
  if (CH_local > 1.0) return false; // never rises
  double CA_local = (sinDec + (SD_local * sinLat)) / (CD_local * cosLat);
  double H_local = acos(CH_local);
  azimuthRising = acos(CA_local);
  double B_local = rad2deg(H_local) / 15.0;
  double A_local = rad2deg(RArad) / 15.0;
  localSiderealTimeRising = inRange24(24.0 + A_local - B_local);
  localSiderealTimeSetting = inRange24(A_local + B_local);
  azimuthSetting = inRange2PI(F2PI - azimuthRising);
  azimuthRising = inRange2PI(azimuthRising); //need for Moon rise/set
  return true;
}

double SiderealPlanets::getRiseTime(void) {
  return doLST2LT(localSiderealTimeRising);
}

double SiderealPlanets::getSetTime(void) {
  return doLST2LT(localSiderealTimeSetting);
}

double SiderealPlanets::doAnomaly(double meanAnomaly, double eccentricity) {
  //Returns eccentric anomaly in degrees given the mean anomaly in degrees
  //and eccentricity for an elliptical orbit.
  double m, d, a;
  SP_meanAnomaly = deg2rad(meanAnomaly);
  boolean flag = false;
  m = SP_meanAnomaly - F2PI * floor(SP_meanAnomaly / F2PI);
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
  julianCenturies1900 = (modifiedJulianDate1900() / 36525.0) + (getGMT() / 8.766e5);
  double T2_local = julianCenturies1900 * julianCenturies1900;
  double A_local = 1.000021359e2 * julianCenturies1900;
  double B_local = 360.0 * (A_local - floor(A_local));
  double L_local = 2.7969668e2 + 3.025e-4 * T2_local + B_local;
  A_local = 9.999736042e1 * julianCenturies1900;
  B_local = 360.0 * (A_local - floor(A_local));
  meanAnomaly = 3.5847583e2 - (1.5e-4 + 3.3e-6 * julianCenturies1900) * T2_local + B_local;
  double eccentricity = 1.675104e-2 - 4.18e-5 * julianCenturies1900 - 1.26e-7 * T2_local;
  doAnomaly(meanAnomaly, eccentricity);
  
  A_local = 6.255209472e1 * julianCenturies1900;
  B_local = 360.0 * (A_local - floor(A_local));
  double A1_local = deg2rad(153.23 + B_local);
  A_local =1.251041894e2 * julianCenturies1900;
  B_local = 360.0 * (A_local - floor(A_local));
  double B1b = deg2rad(216.57 + B_local);
  A_local = 9.156766028e1 * julianCenturies1900;
  B_local = 360.0 * (A_local - floor(A_local));
  double C1_local = deg2rad(312.69 + B_local);
  A_local = 1.236853095E3 * julianCenturies1900;
  B_local = 360.0 * (A_local - floor(A_local));
  double D1_local = deg2rad(350.74 + 1.44e-3 * T2_local + B_local);
  double E1_local = deg2rad(231.19 + 20.2 * julianCenturies1900);
  A_local = 1.831353208e2 * julianCenturies1900;
  B_local = 360.0 * (A_local- floor(A_local));
  double H1_local = deg2rad(353.4 + B_local);
  
  double D2_local = 1.34e-3 * cos(A1_local) + 1.54e-3 * cos(B1b) + 2e-3 * cos(C1_local);
  D2_local = D2_local + 1.79e-3 * sin(D1_local) + 1.78e-3 * sin(E1_local);
  
  double D3_local = 5.43e-6 * sin(A1_local) + 1.575e-5 * sin(B1b);
  D3_local = D3_local + 1.627e-5 * sin(C1_local) + 3.076e-5 * cos(D1_local);
  D3_local = D3_local + 9.27e-6 * sin(H1_local);
  
  sunTrueGeocentricLongitude = trueAnomaly + deg2rad(L_local - meanAnomaly + D2_local);
  // Distance from Earth in Astronomical Units = int(sunEarthDistance * 1e5 + 0.5) / 1e5
  sunEarthDistance = 1.0000002 * (1.0 - eccentricity * cos(eccentricAnomaly)) + D3_local;
  // true geocentric longitude of the Sun
  sunTrueGeocentricLongitude = inRange2PI(sunTrueGeocentricLongitude);
  
  // Apparent ecliptic longitude
  doNutation();
  apparentEclipticLongitude = rad2deg(sunTrueGeocentricLongitude) + nutationInLongitude - 5.69e-3;
  Ecl2RaDecDone = false;
  setEcliptic(apparentEclipticLongitude, 0.0);
  doEcliptic2RAdec();
  
  return true;
}

boolean SiderealPlanets::doMoon(void) {
  julianCenturies1900 = (modifiedJulianDate1900() / 36525.0) + (getGMT() / 8.766e5);
  double T2_local = julianCenturies1900 * julianCenturies1900;
  double M1_local = 2.732158213e1;
  double M2_local = 3.652596407e2;
  double M3_local = 2.755455094e1;
  double M4_local = 2.953058868e1;
  double M5_local = 2.721222039e1;
  double M6_local = 6.798363307e3;
  double Q_local = modifiedJulianDate1900()+(getGMT() / 24.0);
  M1_local = Q_local / M1_local;
  M2_local = Q_local / M2_local;
  M3_local = Q_local / M3_local;
  M4_local = Q_local / M4_local;
  M5_local = Q_local / M5_local;
  M6_local = Q_local / M6_local;
  M1_local = 360. * (M1_local - floor(M1_local));
  M2_local = 360. * (M2_local - floor(M2_local));
  M3_local = 360. * (M3_local - floor(M3_local));
  M4_local = 360. * (M4_local - floor(M4_local));
  M5_local = 360. * (M5_local - floor(M5_local));
  M6_local = 360. * (M6_local - floor(M6_local));
  double moonMeanLongitude = 2.70434164E2 + M1_local - (1.133E-3 - 1.9E-6 * julianCenturies1900) * T2_local;
  sunMeanAnomaly = 3.58475833E2 + M2_local - (1.5E-4 + 3.3E-6 * julianCenturies1900) * T2_local;
  moonMeanAnomaly = 2.96104608E2 + M3_local+(9.192E-3 + 1.44E-5 * julianCenturies1900) * T2_local;
  double moonMeanElongation = 3.50737486E2 + M4_local - (1.436E-3 - 1.9E-6 * julianCenturies1900) * T2_local;
  double moonMeanDistanceAcendingNode = 11.250889 + M5_local - (3.211E-3 + 3E-7 * julianCenturies1900) * T2_local;
  double moonLongitudeAscendingNode = 2.59183275E2 - M6_local+(2.078E-3 + 2.2E-6 * julianCenturies1900) * T2_local;
  double A_local = deg2rad(51.2 + 20.2 * julianCenturies1900);
  double S1_local = sin(A_local);
  double S2_local = sin(deg2rad(moonLongitudeAscendingNode));
  double B_local = 346.56+(132.87 - 9.1731E-3 * julianCenturies1900) * julianCenturies1900;
  double S3_local = 3.964E-3 * sin(deg2rad(B_local));
  double C_local = deg2rad(moonLongitudeAscendingNode + 275.05 - 2.3 * julianCenturies1900);
  double S4_local = sin(C_local);
  moonMeanLongitude = moonMeanLongitude + 2.33E-4 * S1_local + S3_local + 1.964E-3 * S2_local;
  sunMeanAnomaly = sunMeanAnomaly - 1.778E-3 * S1_local;
  moonMeanAnomaly = moonMeanAnomaly + 8.17E-4 * S1_local + S3_local + 2.541E-3 * S2_local;
  moonMeanDistanceAcendingNode = moonMeanDistanceAcendingNode + S3_local - 2.4691E-2 * S2_local - 4.328E-3 * S4_local;
  moonMeanElongation = moonMeanElongation + 2.011E-3 * S1_local + S3_local + 1.964E-3 * S2_local;
  double E_local = 1. - (2.495E-3 + 7.52E-6 * julianCenturies1900) * julianCenturies1900;
  double E2_local = E_local * E_local;
  moonMeanLongitude = deg2rad(moonMeanLongitude);
  sunMeanAnomaly = deg2rad(sunMeanAnomaly);
  moonLongitudeAscendingNode = deg2rad(moonLongitudeAscendingNode);
  moonMeanElongation = deg2rad(moonMeanElongation);
  moonMeanDistanceAcendingNode = deg2rad(moonMeanDistanceAcendingNode);
  moonMeanAnomaly = deg2rad(moonMeanAnomaly);
  double L_local = 6.28875 * sin(moonMeanAnomaly) + 1.274018 * sin(2. * moonMeanElongation - moonMeanAnomaly);
  L_local = L_local + 6.58309e-1 * sin(2. * moonMeanElongation) + 2.13616e-1 * sin(2. * moonMeanAnomaly);
  L_local = L_local - E_local * 1.85596E-1 * sin(sunMeanAnomaly) - 1.14336E-1 * sin(2. * moonMeanDistanceAcendingNode);
  L_local = L_local + 5.8793E-2 * sin(2. * (moonMeanElongation - moonMeanAnomaly));
  L_local = L_local + 5.7212E-2 * E_local * sin(2. * moonMeanElongation - sunMeanAnomaly - moonMeanAnomaly) + 5.332E-2 * sin(2. * moonMeanElongation + moonMeanAnomaly);
  L_local = L_local + 4.5874E-2 * E_local * sin(2. * moonMeanElongation - sunMeanAnomaly) + 4.1024E-2 * E_local * sin(moonMeanAnomaly - sunMeanAnomaly);
  L_local = L_local - 3.4718E-2 * sin(moonMeanElongation) - E_local * 3.0465E-2 * sin(sunMeanAnomaly + moonMeanAnomaly);
  L_local = L_local + 1.5326E-2 * sin(2. * (moonMeanElongation - moonMeanDistanceAcendingNode)) - 1.2528E-2 * sin(2. * moonMeanDistanceAcendingNode + moonMeanAnomaly);
  L_local = L_local - 1.098E-2 * sin(2. * moonMeanDistanceAcendingNode - moonMeanAnomaly) + 1.0674E-2 * sin(4. * moonMeanElongation - moonMeanAnomaly);
  L_local = L_local + 1.0034E-2 * sin(3. * moonMeanAnomaly) + 8.548E-3 * sin(4. * moonMeanElongation - 2. * moonMeanAnomaly);
  L_local = L_local - E_local * 7.91E-3 * sin(sunMeanAnomaly - moonMeanAnomaly + 2. * moonMeanElongation) - E_local * 6.783E-3 * sin(2. * moonMeanElongation + sunMeanAnomaly);
  L_local = L_local + 5.162E-3 * sin(moonMeanAnomaly - moonMeanElongation) + E_local * 5E-3 * sin(sunMeanAnomaly + moonMeanElongation);
  L_local = L_local + 3.862E-3 * sin(4. * moonMeanElongation) + E_local * 4.049E-3 * sin(moonMeanAnomaly - sunMeanAnomaly + 2. * moonMeanElongation);
  L_local = L_local + 3.996E-3 * sin(2. * (moonMeanAnomaly + moonMeanElongation)) + 3.665E-3 * sin(2. * moonMeanElongation - 3. * moonMeanAnomaly);
  L_local = L_local + E_local * 2.695E-3 * sin(2. * moonMeanAnomaly - sunMeanAnomaly) + 2.602E-3 * sin(moonMeanAnomaly - 2. * (moonMeanDistanceAcendingNode + moonMeanElongation));
  L_local = L_local + E_local * 2.396E-3 * sin(2. * (moonMeanElongation - moonMeanAnomaly) - sunMeanAnomaly) - 2.349E-3 * sin(moonMeanAnomaly + moonMeanElongation);
  L_local = L_local + E2_local * 2.249E-3 * sin(2. * (moonMeanElongation - sunMeanAnomaly)) - E_local * 2.125E-3 * sin(2. * moonMeanAnomaly + sunMeanAnomaly);
  L_local = L_local - E2_local * 2.079E-3 * sin(2. * sunMeanAnomaly) + E2_local * 2.059E-3 * sin(2. * (moonMeanElongation - sunMeanAnomaly) - moonMeanAnomaly);
  L_local = L_local - 1.773E-3 * sin(moonMeanAnomaly + 2. * (moonMeanElongation - moonMeanDistanceAcendingNode)) - 1.595E-3 * sin(2. * (moonMeanDistanceAcendingNode + moonMeanElongation));
  L_local = L_local + E_local * 1.22E-3 * sin(4. * moonMeanElongation - sunMeanAnomaly - moonMeanAnomaly) - 1.11E-3 * sin(2. * (moonMeanAnomaly + moonMeanDistanceAcendingNode));
  L_local = L_local + 8.92E-4 * sin(moonMeanAnomaly - 3. * moonMeanElongation) - E_local * 8.11E-4 * sin(sunMeanAnomaly + moonMeanAnomaly + 2. * moonMeanElongation);
  L_local = L_local + E_local * 7.61E-4 * sin(4. * moonMeanElongation - sunMeanAnomaly - 2. * moonMeanAnomaly);
  L_local = L_local + E2_local * 7.04E-4 * sin(moonMeanAnomaly - 2. * (sunMeanAnomaly + moonMeanElongation));
  L_local = L_local + E_local * 6.93E-4 * sin(sunMeanAnomaly - 2. * (moonMeanAnomaly - moonMeanElongation));
  L_local = L_local + E_local * 5.98E-4 * sin(2. * (moonMeanElongation - moonMeanDistanceAcendingNode) - sunMeanAnomaly);
  L_local = L_local + 5.5E-4 * sin(moonMeanAnomaly + 4 * moonMeanElongation) + 5.38E-4 * sin(4. * moonMeanAnomaly);
  L_local = L_local + E_local * 5.21E-4 * sin(4. * moonMeanElongation - sunMeanAnomaly) + 4.86E-4 * sin(2. * moonMeanAnomaly - moonMeanElongation);
  L_local = L_local + E2_local * 7.17E-4 * sin(moonMeanAnomaly - 2. * sunMeanAnomaly);
  moonGeocentricEclipticLongitude = inRange2PI(moonMeanLongitude + deg2rad(L_local)); //Moon's geocentric ecliptic longitude

  double G_local = 5.128189 * sin(moonMeanDistanceAcendingNode) + 2.80606e-1 * sin(moonMeanAnomaly + moonMeanDistanceAcendingNode);
  G_local = G_local + 2.77693E-1 * sin(moonMeanAnomaly - moonMeanDistanceAcendingNode) + 1.73238E-1 * sin(2. * moonMeanElongation - moonMeanDistanceAcendingNode);
  G_local = G_local + 5.5413E-2 * sin(2. * moonMeanElongation + moonMeanDistanceAcendingNode - moonMeanAnomaly) + 4.6272E-2 * sin(2. * moonMeanElongation - moonMeanDistanceAcendingNode - moonMeanAnomaly);
  G_local = G_local + 3.2573E-2 * sin(2. * moonMeanElongation + moonMeanDistanceAcendingNode) + 1.7198E-2 * sin(2. * moonMeanAnomaly + moonMeanDistanceAcendingNode);
  G_local = G_local + 9.267E-3 * sin(2. * moonMeanElongation + moonMeanAnomaly - moonMeanDistanceAcendingNode) + 8.823E-3 * sin(2. * moonMeanAnomaly - moonMeanDistanceAcendingNode);
  G_local = G_local + E_local * 8.247E-3 * sin(2. * moonMeanElongation - sunMeanAnomaly - moonMeanDistanceAcendingNode) + 4.323E-3 * sin(2. * (moonMeanElongation - moonMeanAnomaly) - moonMeanDistanceAcendingNode);
  G_local = G_local + 4.2E-3 * sin(2. * moonMeanElongation + moonMeanDistanceAcendingNode + moonMeanAnomaly) + E_local * 3.372E-3 * sin(moonMeanDistanceAcendingNode - sunMeanAnomaly - 2. * moonMeanElongation);
  G_local = G_local + E_local * 2.472E-3 * sin(2. * moonMeanElongation + moonMeanDistanceAcendingNode - sunMeanAnomaly - moonMeanAnomaly);
  G_local = G_local + E_local * 2.222E-3 * sin(2. * moonMeanElongation + moonMeanDistanceAcendingNode - sunMeanAnomaly);
  G_local = G_local + E_local * 2.072E-3 * sin(2. * moonMeanElongation - moonMeanDistanceAcendingNode - sunMeanAnomaly - moonMeanAnomaly);
  G_local = G_local + E_local * 1.877E-3 * sin(moonMeanDistanceAcendingNode - sunMeanAnomaly + moonMeanAnomaly) + 1.828E-3 * sin(4. * moonMeanElongation - moonMeanDistanceAcendingNode - moonMeanAnomaly);
  G_local = G_local - E_local * 1.803E-3 * sin(moonMeanDistanceAcendingNode + sunMeanAnomaly) - 1.75E-3 * sin(3. * moonMeanDistanceAcendingNode);
  G_local = G_local + E_local * 1.57E-3 * sin(moonMeanAnomaly - sunMeanAnomaly - moonMeanDistanceAcendingNode) - 1.487E-3 * sin(moonMeanDistanceAcendingNode + moonMeanElongation);
  G_local = G_local - E_local * 1.481E-3 * sin(moonMeanDistanceAcendingNode + sunMeanAnomaly + moonMeanAnomaly) + E_local * 1.417E-3 * sin(moonMeanDistanceAcendingNode - sunMeanAnomaly - moonMeanAnomaly);
  G_local = G_local + E_local * 1.35E-3 * sin(moonMeanDistanceAcendingNode - sunMeanAnomaly) + 1.33E-3 * sin(moonMeanDistanceAcendingNode - moonMeanElongation);
  G_local = G_local + 1.106E-3 * sin(moonMeanDistanceAcendingNode + 3. * moonMeanAnomaly) + 1.02E-3 * sin(4. * moonMeanElongation - moonMeanDistanceAcendingNode);
  G_local = G_local + 8.33E-4 * sin(moonMeanDistanceAcendingNode + 4 * moonMeanElongation - moonMeanAnomaly) + 7.81E-4 * sin(moonMeanAnomaly - 3. * moonMeanDistanceAcendingNode);
  G_local = G_local + 6.7E-4 * sin(moonMeanDistanceAcendingNode + 4 * moonMeanElongation - 2. * moonMeanAnomaly) + 6.06E-4 * sin(2. * moonMeanElongation - 3. * moonMeanDistanceAcendingNode);
  G_local = G_local + 5.97E-4 * sin(2. * (moonMeanElongation + moonMeanAnomaly) - moonMeanDistanceAcendingNode);
  G_local = G_local + E_local * 4.92E-4 * sin(2. * moonMeanElongation + moonMeanAnomaly - sunMeanAnomaly - moonMeanDistanceAcendingNode) + 4.5E-4 * sin(2. * (moonMeanAnomaly - moonMeanElongation) - moonMeanDistanceAcendingNode);
  G_local = G_local + 4.39E-4 * sin(3. * moonMeanAnomaly - moonMeanDistanceAcendingNode) + 4.23E-4 * sin(moonMeanDistanceAcendingNode + 2. * (moonMeanElongation + moonMeanAnomaly));
  G_local = G_local + 4.22E-4 * sin(2. * moonMeanElongation - moonMeanDistanceAcendingNode - 3. * moonMeanAnomaly) - E_local * 3.67E-4 * sin(sunMeanAnomaly + moonMeanDistanceAcendingNode + 2. * moonMeanElongation - moonMeanAnomaly);
  G_local = G_local - E_local * 3.53E-4 * sin(sunMeanAnomaly + moonMeanDistanceAcendingNode + 2. * moonMeanElongation) + 3.31E-4 * sin(moonMeanDistanceAcendingNode + 4 * moonMeanElongation);
  G_local = G_local + E_local * 3.17E-4 * sin(2. * moonMeanElongation + moonMeanDistanceAcendingNode - sunMeanAnomaly + moonMeanAnomaly);
  G_local = G_local + E2_local * 3.06E-4 * sin(2. * (moonMeanElongation - sunMeanAnomaly) - moonMeanDistanceAcendingNode) - 2.83E-4 * sin(moonMeanAnomaly + 3. * moonMeanDistanceAcendingNode);
  double W1 = 4.664E-4 * cos(moonLongitudeAscendingNode);
  double W2 = 7.54E-5 * cos(C_local);
  moonGeocentricEclipticLatitude = deg2rad(G_local) * (1.0 - W1 - W2); // Moon's geocentric ecliptic latitude
  moonHorizontalParallax = 9.50724E-1 + 5.1818E-2 * cos(moonMeanAnomaly) + 9.531E-3 * cos(2. * moonMeanElongation - moonMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + 7.843E-3 * cos(2. * moonMeanElongation) + 2.824E-3 * cos(2. * moonMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + 8.57E-4 * cos(2. * moonMeanElongation + moonMeanAnomaly) + E_local * 5.33E-4 * cos(2. * moonMeanElongation - sunMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + E_local * 4.01E-4 * cos(2. * moonMeanElongation - moonMeanAnomaly - sunMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + E_local * 3.2E-4 * cos(moonMeanAnomaly - sunMeanAnomaly) - 2.71E-4 * cos(moonMeanElongation);
  moonHorizontalParallax = moonHorizontalParallax - E_local * 2.64E-4 * cos(sunMeanAnomaly + moonMeanAnomaly) - 1.98E-4 * cos(2. * moonMeanDistanceAcendingNode - moonMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + 1.73E-4 * cos(3. * moonMeanAnomaly) + 1.67E-4 * cos(4. * moonMeanElongation - moonMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax - E_local * 1.11E-4 * cos(sunMeanAnomaly) + 1.03E-4 * cos(4. * moonMeanElongation - 2. * moonMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax - 8.4E-5 * cos(2. * moonMeanAnomaly - 2. * moonMeanElongation) - E_local * 8.3E-5 * cos(2. * moonMeanElongation + sunMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + 7.9E-5 * cos(2. * moonMeanElongation + 2. * moonMeanAnomaly) + 7.2E-5 * cos(4. * moonMeanElongation);
  moonHorizontalParallax = moonHorizontalParallax + E_local * 6.4E-5 * cos(2. * moonMeanElongation - sunMeanAnomaly + moonMeanAnomaly) - E_local * 6.3E-5 * cos(2. * moonMeanElongation + sunMeanAnomaly - moonMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + E_local * 4.1E-5 * cos(sunMeanAnomaly + moonMeanElongation) + E_local * 3.5E-5 * cos(2. * moonMeanAnomaly - sunMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax - 3.3E-5 * cos(3. * moonMeanAnomaly - 2. * moonMeanElongation) - 3E-5 * cos(moonMeanAnomaly + moonMeanElongation);
  moonHorizontalParallax = moonHorizontalParallax - 2.9E-5 * cos(2. * (moonMeanDistanceAcendingNode - moonMeanElongation)) - E_local * 2.9E-5 * cos(2. * moonMeanAnomaly + sunMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + E2_local * 2.6E-5 * cos(2. * (moonMeanElongation - sunMeanAnomaly)) - 2.3E-5 * cos(2. * (moonMeanDistanceAcendingNode - moonMeanElongation) + moonMeanAnomaly);
  moonHorizontalParallax = moonHorizontalParallax + E_local * 1.9E-5 * cos(4. * moonMeanElongation - sunMeanAnomaly - moonMeanAnomaly);
  EquatHorizontalParallax = moonHorizontalParallax;
  moonHorizontalParallax = deg2rad(moonHorizontalParallax); // Horizntal parallax
  // end of regular routine
  //EclLongitude = moonGeocentricEclipticLongitude to SP_AL, EclLatitude = moonGeocentricEclipticLatitude
  doNutation();
  double SP_AL = rad2deg(moonGeocentricEclipticLongitude) + nutationInLongitude;
  //setEcliptic(moonGeocentricEclipticLongitude, moonGeocentricEclipticLatitude);
  EclLongitude = deg2rad(SP_AL);
  EclLatitude = moonGeocentricEclipticLatitude;
  doEcliptic2RAdec();
  return true;
}

boolean SiderealPlanets::doPlanetElements(void) {
  // 'i' in planetaryOrbitalElements[i][j] is the number of the planet to compute
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
  julianCenturies1900 = (modifiedJulianDate1900() / 36525.0) + (getGMT() / 8.766e5);
  double A0_local, A1_local, A2_local, A3_local, AA_local, B_local;
  int i, j, k;
  k = 0; //data array index
  for (i = 1; i < 8; i++) {
	A0_local = readData[k++];
	A1_local = readData[k++];
	A2_local = readData[k++];
	A3_local = readData[k++];
	AA_local = A1_local * julianCenturies1900;
	B_local = 360.0 * (AA_local - floor(AA_local));
	planetaryOrbitalElements[i][1] = inRange360(A0_local + B_local + (A3_local * julianCenturies1900 + A2_local) * julianCenturies1900 * julianCenturies1900);
	planetaryOrbitalElements[i][2] = (A1_local * 9.856263e-3) + (A2_local + A3_local) / 36525.0;
	for (j = 3; j <= 6; j++) {
	  A0_local = readData[k++];
	  A1_local = readData[k++];
	  A2_local = readData[k++];
	  A3_local = readData[k++];
	  planetaryOrbitalElements[i][j] = ((A3_local * julianCenturies1900 + A2_local) * julianCenturies1900 + A1_local) * julianCenturies1900 + A0_local;
	}
	planetaryOrbitalElements[i][7] = readData[k++];
	planetaryOrbitalElements[i][8] = readData[k++];
	planetaryOrbitalElements[i][9] = readData[k++];
  }
  return true;
}

double SiderealPlanets::getPL(int i, int j) {
  // get orbital element j for a planet i
  return planetaryOrbitalElements[i][j];
}

boolean SiderealPlanets::doPlans(int planetNumber) {
  //Calculate apparent geocentric ecliptic coordinates,
  //allowing for light travel time, for the planets
  int K_local, J_local;
  double perturbationLongitude, perturbationRadiusVector, perturbationMeanLongitude, perturbationEccentricity, perturbationMeanAnomaly, perturbationSemiMajorAxis, perturbationHeliocentricEclipticLatitude;
  double sunMeanAnomaly, radiusVectorEarth, earthEclipticLongitude, SP_eccentricity;
  double planetAnomalies[8];
  double radiusVectorCorrected, LO_local, LP_local, LongitudeAscendingNode, CO_local, inclination, SO_local, SP_local, Y_local, PS_local, PD_local, cosine_PS_local, RD_local, LL_local, distanceEarthCorrected, L1_local, L2_local, geocentricEclipticLongitude, geocentricEclipticLatitude, A_local;
  double SA_local, CA_local, J1_local, J2_local, J3_local, J4_local, J5_local, J6_local, J7_local, J8_local, J9_local, JA_local, JB_local, JC_local;
  double  U1_local, U2_local, U3_local, U4_local, U5_local, U6_local, U7_local, U8_local, U9_local, UA_local, UB_local, UC_local, UD_local, UE_local, UF_local, UG_local, UI_local, UJ_local, UK_local, UL_local, UN_local, UO_local, UP_local, UQ_local, UR_local, UU_local, UV_local, UW_local, UX_local, UY_local, UZ_local;
  double  VA_local, VB_local, VC_local, VD_local, VE_local, VF_local, VG_local, VH_local, VI_local, VJ_local, VK_local;
  
  if (planetNumber < 1 || planetNumber > 7) return false; //bad planet value
  doPlanetElements();
  double lightTravelTime = 0.;
  doSun();
  sunMeanAnomaly = SP_meanAnomaly; // SP_meanAnomaly is Mean Anomaly of the Sun in radians
  radiusVectorEarth = sunEarthDistance;
  earthEclipticLongitude = sunTrueGeocentricLongitude + FPI; //Earth's ecliptic longitude (radians)
  for (K_local = 1; K_local < 3; K_local++) {
	for (J_local = 1; J_local < 8; J_local++) {
	  planetAnomalies[J_local] = deg2rad(planetaryOrbitalElements[J_local][1] - planetaryOrbitalElements[J_local][3] - lightTravelTime * planetaryOrbitalElements[J_local][2]);
	}
	perturbationLongitude = 0.;
	perturbationRadiusVector = 0.;
	perturbationMeanLongitude = 0.;
	perturbationEccentricity = 0.;
	perturbationMeanAnomaly = 0.;
	perturbationSemiMajorAxis = 0.;
	perturbationHeliocentricEclipticLatitude = 0.;
	if (planetNumber == 1) {
      //Mercury
	  perturbationLongitude = 2.04e-3 * cos(5. * planetAnomalies[2] - 2. * planetAnomalies[1] + 2.1328e-1);
	  perturbationLongitude = perturbationLongitude + 1.03e-3 * cos(2. * planetAnomalies[2] - planetAnomalies[1] - 2.08046);
	  perturbationLongitude = perturbationLongitude + 9.1e-4 * cos(2. * planetAnomalies[4] - planetAnomalies[1] - 6.4582e-1);
	  perturbationLongitude = perturbationLongitude + 7.8e-4 * cos(5. * planetAnomalies[2] - 3. * planetAnomalies[1] + 1.7692e-1);
	  
	  perturbationRadiusVector = 7.525e-6 * cos(2. * planetAnomalies[4] - planetAnomalies[1] + 9.25251e-1);
	  perturbationRadiusVector = perturbationRadiusVector + 6.802e-6 * cos(5. * planetAnomalies[2] - 3. * planetAnomalies[1] - 4.53642);
	  perturbationRadiusVector = perturbationRadiusVector + 5.457e-6 * cos(2. * planetAnomalies[2] - 2. * planetAnomalies[1] - 1.24246);
	  perturbationRadiusVector = perturbationRadiusVector + 3.569e-6 * cos(5. * planetAnomalies[2] - planetAnomalies[1] - 1.35699);
	} else if (planetNumber == 2) {
      //Venus
	  perturbationMeanLongitude = 7.7e-4 * sin(4.1406 + julianCenturies1900 * 2.6227);
	  perturbationMeanLongitude = deg2rad(perturbationMeanLongitude);
	  perturbationMeanAnomaly = perturbationMeanLongitude;
	  
	  perturbationLongitude = 3.13e-3 * cos(2. * sunMeanAnomaly - 2. * planetAnomalies[2] - 2.587);
	  perturbationLongitude = perturbationLongitude + 1.98e-3 * cos(3. * sunMeanAnomaly - 3. * planetAnomalies[2] + 4.4768e-2);
	  perturbationLongitude = perturbationLongitude + 1.36e-3 * cos(sunMeanAnomaly - planetAnomalies[2] - 2.0788);
	  perturbationLongitude = perturbationLongitude + 9.6e-4 * cos(3. * sunMeanAnomaly - 2. * planetAnomalies[2] - 2.3721);
	  perturbationLongitude = perturbationLongitude + 8.2e-4 * cos(planetAnomalies[4] - planetAnomalies[2] - 3.6318);
	  
	  perturbationRadiusVector = 2.2501e-5 * cos(2. * sunMeanAnomaly - 2. * planetAnomalies[2] - 1.01592);
	  perturbationRadiusVector = perturbationRadiusVector + 1.9045e-5 * cos(3. * sunMeanAnomaly - 3.* planetAnomalies[2] + 1.61577);
	  perturbationRadiusVector = perturbationRadiusVector + 6.887e-6 * cos(planetAnomalies[4] - planetAnomalies[2] - 2.06106);
	  perturbationRadiusVector = perturbationRadiusVector + 5.172e-6 * cos(sunMeanAnomaly - planetAnomalies[2] - 5.08065e-1);
	  perturbationRadiusVector = perturbationRadiusVector + 3.62e-6 * cos(5. * sunMeanAnomaly - 4. * planetAnomalies[2] - 1.81877);
	  perturbationRadiusVector = perturbationRadiusVector + 3.283e-6 * cos(4. * sunMeanAnomaly - 4. * planetAnomalies[2] + 1.10851);
	  perturbationRadiusVector = perturbationRadiusVector + 3.074e-6 * cos(2. * planetAnomalies[4] - 2. * planetAnomalies[2] - 9.62846e-1);
	} else if (planetNumber == 3) {
      //Mars
	  A_local = 3. * planetAnomalies[4] - 8. * planetAnomalies[3] + 4. * sunMeanAnomaly;
	  SA_local = sin(A_local);
	  CA_local = cos(A_local);
	  perturbationMeanLongitude = -(1.133e-2 * SA_local + 9.33e-3 * CA_local);
	  perturbationMeanLongitude = deg2rad(perturbationMeanLongitude); 
	  perturbationMeanAnomaly = perturbationMeanLongitude;
	  
	  perturbationLongitude = 7.05e-3 * cos(planetAnomalies[4] - 8. * planetAnomalies[3] - 8.5448e-1);
	  perturbationLongitude = perturbationLongitude + 6.07e-3 * cos(2. * planetAnomalies[4] - planetAnomalies[3] - 3.2873);
	  perturbationLongitude = perturbationLongitude + 4.45e-3 * cos(2. * planetAnomalies[4] - 2. * planetAnomalies[3] - 3.3492);
	  perturbationLongitude = perturbationLongitude + 3.88e-3 * cos(sunMeanAnomaly - 2. * planetAnomalies[3] + 3.5771e-1);
	  perturbationLongitude = perturbationLongitude + 2.38e-3 * cos(sunMeanAnomaly - planetAnomalies[3] + 6.1256e-1);
	  perturbationLongitude = perturbationLongitude + 2.04e-3 * cos(2. * sunMeanAnomaly - 3. * planetAnomalies[3] + 2.7688);
	  perturbationLongitude = perturbationLongitude + 1.77e-3 * cos(3. * planetAnomalies[3] - planetAnomalies[2] - 1.0053);
	  perturbationLongitude = perturbationLongitude + 1.36e-3 * cos(2. * sunMeanAnomaly - 4. * planetAnomalies[3] + 2.6894);
	  perturbationLongitude = perturbationLongitude + 1.04e-3 * cos(planetAnomalies[4] + 3.0749e-1);
	  
	  perturbationRadiusVector = 5.3227e-5 * cos(planetAnomalies[4] - planetAnomalies[3] + 7.17864e-1);
	  perturbationRadiusVector = perturbationRadiusVector + 5.0989e-5 * cos(2. * planetAnomalies[4] - 2. * planetAnomalies[3] - 1.77997);
	  perturbationRadiusVector = perturbationRadiusVector + 3.8278e-5 * cos(2. * planetAnomalies[4] - planetAnomalies[3] - 1.71617);
	  perturbationRadiusVector = perturbationRadiusVector + 1.5996e-5 * cos(sunMeanAnomaly - planetAnomalies[3] - 9.69618e-1);
	  perturbationRadiusVector = perturbationRadiusVector + 1.4764e-5 * cos(2. * sunMeanAnomaly - 3. * planetAnomalies[3] + 1.19768);
	  perturbationRadiusVector = perturbationRadiusVector + 8.966e-6 * cos(planetAnomalies[4] - 2. * planetAnomalies[3] + 7.61225e-1);
	  perturbationRadiusVector = perturbationRadiusVector + 7.914e-6 * cos(3. * planetAnomalies[4] - 2. * planetAnomalies[3] - 2.43887);
	  perturbationRadiusVector = perturbationRadiusVector + 7.004e-6 * cos(2. * planetAnomalies[4] - 3. * planetAnomalies[3] - 1.79573);
	  perturbationRadiusVector = perturbationRadiusVector + 6.62e-6 * cos(sunMeanAnomaly - 2. * planetAnomalies[3] + 1.97575);
	  perturbationRadiusVector = perturbationRadiusVector + 4.93e-6 * cos(3. * planetAnomalies[4] - 3. * planetAnomalies[3] - 1.33069);
	  perturbationRadiusVector = perturbationRadiusVector + 4.693e-6 * cos(3. * sunMeanAnomaly - 5. * planetAnomalies[3] + 3.32665);
	  perturbationRadiusVector = perturbationRadiusVector + 4.571e-6 * cos(2. * sunMeanAnomaly - 4. * planetAnomalies[3] + 4.27086);
	  perturbationRadiusVector = perturbationRadiusVector + 4.409e-6 * cos(3. * planetAnomalies[4] - planetAnomalies[3] - 2.02158);
	} else {
      //4=Jupiter, 5=Saturn, 6=Uranus, and 7=Neptune
	  J1_local = julianCenturies1900 / 5.0 + 0.1;
	  J2_local = inRange2PI(4.14473 + 5.29691e1 * julianCenturies1900);
	  J3_local = inRange2PI(4.641118 + 2.132991e1 * julianCenturies1900);
	  J4_local = inRange2PI(4.250177 + 7.478172 * julianCenturies1900);
	  J5_local = 5.0 * J3_local - 2.0 * J2_local;
	  J6_local = 2.0 * J2_local - 6.0 * J3_local + 3.0 * J4_local;
	  if ((planetNumber == 4) || (planetNumber == 5)) {
        //Common code for Jupiter and Saturn
		J7_local = J3_local - J2_local;
		U1_local = sin(J3_local);
		U2_local = cos(J3_local);
		U3_local = sin(2.0 * J3_local);
		U4_local = cos(2.0 * J3_local);
		U5_local = sin(J5_local);
		U6_local = cos(J5_local);
		U7_local = sin(2.0 * J5_local);
		U8_local = sin(J6_local);
		U9_local = sin(J7_local);
		UA_local = cos(J7_local);
		UB_local = sin(2.0 * J7_local);
		UC_local = cos(2.0 * J7_local);
		UD_local = sin(3.0 * J7_local);
		UE_local = cos(3.0 * J7_local);
		UF_local = sin(4.0 * J7_local);
		UG_local = cos(4.0 * J7_local);
		VH_local = cos(5.0 * J7_local);
	  } else {
        //Common code for Uranus and Neptune
		J8_local = inRange2PI(1.46205 + 3.81337 * julianCenturies1900);
		J9_local = 2. * J8_local - J4_local;
		VJ_local = sin(J9_local);
		UU_local = cos(J9_local);
		UV_local = sin(2. * J9_local);
		UW_local = cos(2. * J9_local);
	  }
	  
      if (planetNumber == 4) {
        //Jupiter
		perturbationMeanLongitude = (3.31364e-1 - (1.0281e-2 + 4.692e-3 * J1_local) * J1_local) * U5_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (3.228e-3 - (6.4436e-2 - 2.075e-3 * J1_local) * J1_local) * U6_local;
		perturbationMeanLongitude = perturbationMeanLongitude - (3.083e-3 + (2.75e-4 - 4.89e-4 * J1_local) * J1_local) * U7_local;
		perturbationMeanLongitude = perturbationMeanLongitude + 2.472e-3 * U8_local + 1.3619e-2 * U9_local + 1.8472e-2 * UB_local;
		perturbationMeanLongitude = perturbationMeanLongitude + 6.717e-3 * UD_local + 2.775e-3 * UF_local + 6.417e-3 * UB_local * U1_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (7.275e-3 - 1.253e-3 * J1_local) * U9_local * U1_local + 2.439e-3 * UD_local * U1_local;
		perturbationMeanLongitude = perturbationMeanLongitude - (3.5681e-2 + 1.208e-3 * J1_local) * U9_local * U2_local - 3.767e-3 * UC_local * U1_local;
		perturbationMeanLongitude = perturbationMeanLongitude - (3.3839e-2 + 1.125e-3 * J1_local) * UA_local * U1_local - 4.261e-3 * UB_local * U2_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (1.161e-3 * J1_local - 6.333e-3) * UA_local * U2_local + 2.178e-3 * U2_local;
		perturbationMeanLongitude = perturbationMeanLongitude - 6.675e-3 * UC_local * U2_local - 2.664e-3 * UE_local * U2_local - 2.572e-3 * U9_local * U3_local;
		perturbationMeanLongitude = perturbationMeanLongitude - 3.567e-3 * UB_local * U3_local + 2.094e-3 * UA_local * U4_local + 3.342e-3 * UC_local * U4_local;
		perturbationMeanLongitude = deg2rad(perturbationMeanLongitude);
		
		perturbationEccentricity = (3606. + (130. - 43. * J1_local) * J1_local) * U5_local + (1289. - 580. * J1_local) * U6_local;
		perturbationEccentricity = perturbationEccentricity - 6764. * U9_local * U1_local - 1110. * UB_local * U1_local - 224. * UD_local * U1_local - 204. * U1_local;
		perturbationEccentricity = perturbationEccentricity + (1284. + 116. * J1_local) * UA_local * U1_local + 188. * UC_local * U1_local;
		perturbationEccentricity = perturbationEccentricity + (1460. + 130. * J1_local) * U9_local * U2_local + 224. * UB_local * U2_local - 817. * U2_local;
		perturbationEccentricity = perturbationEccentricity + 6074. * U2_local * UA_local + 992. * UC_local * U2_local + 508. * UE_local * U2_local + 230. * UG_local * U2_local;
		perturbationEccentricity = perturbationEccentricity + 108. * VH_local * U2_local - (956. + 73. * J1_local) * U9_local * U3_local + 448. * UB_local * U3_local;
		perturbationEccentricity = perturbationEccentricity + 137. * UD_local * U3_local + (108. * J1_local - 997.) * UA_local * U3_local + 480. * UC_local * U3_local;
		perturbationEccentricity = perturbationEccentricity + 148. * UE_local * U3_local + (99. * J1_local - 956.) * U9_local * U4_local + 490. * UB_local * U4_local;
		perturbationEccentricity = perturbationEccentricity + 158. * UD_local * U4_local + 179. * U4_local + (1024. + 75. * J1_local) * UA_local * U4_local;
		perturbationEccentricity = perturbationEccentricity - 437. * UC_local * U4_local - 132. * UE_local * U4_local;
		perturbationEccentricity = perturbationEccentricity * 1.0e-7;
		
		VK_local = (7.192e-3 - 3.147e-3 * J1_local) * U5_local - 4.344e-3 * U1_local;
		VK_local = VK_local + (J1_local * (1.97e-4 * J1_local - 6.75e-4) - 2.0428e-2) * U6_local;
		VK_local = VK_local + 3.4036e-2 * UA_local * U1_local + (7.269e-3 + 6.72e-4 * J1_local) * U9_local * U1_local;
		VK_local = VK_local + 5.614e-3 * UC_local * U1_local + 2.964e-3 * UE_local * U1_local + 3.7761e-2 * U9_local * U2_local;
		VK_local = VK_local + 6.158e-3 * UB_local * U2_local - 6.603e-3 * UA_local * U2_local - 5.356e-3 * U9_local * U3_local;
		VK_local = VK_local + 2.722e-3 * UB_local * U3_local + 4.483e-3 * UA_local * U3_local;
		VK_local = VK_local - 2.642e-3 * UC_local * U3_local + 4.403e-3 * U9_local * U4_local;
		VK_local = VK_local - 2.536e-3 * UB_local * U4_local + 5.547e-3 * UA_local * U4_local - 2.689e-3 * UC_local * U4_local;
		perturbationMeanAnomaly = perturbationMeanLongitude - (deg2rad(VK_local) / planetaryOrbitalElements[planetNumber][4]);
		
		perturbationSemiMajorAxis = 205. * UA_local - 263. * U6_local + 693. * UC_local + 312. * UE_local + 147. * UG_local + 299. * U9_local * U1_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis + 181. * UC_local * U1_local + 204. * UB_local * U2_local + 111. * UD_local * U2_local - 337. * UA_local * U2_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis - 111. * UC_local * U2_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis * 1.0e-6;
	  } else if (planetNumber == 5) {
        //Saturn
		UI_local = sin(3. * J3_local);
		UJ_local = cos(3. * J3_local);
		UK_local = sin(4. * J3_local);
		UL_local = cos(4. * J3_local);
		VI_local = cos(2. * J5_local);
		UN_local = sin(5. * J7_local);
		J8_local = J4_local - J3_local;
		UO_local = sin(2. * J8_local);
		UP_local = cos(2. * J8_local);
		UQ_local = sin(3. * J8_local);
		UR_local = cos(3. * J8_local);
		
		perturbationMeanLongitude = 7.581e-3 * U7_local - 7.986e-3 * U8_local - 1.48811e-1 * U9_local;
		perturbationMeanLongitude = perturbationMeanLongitude - (8.14181e-1 - (1.815e-2 - 1.6714e-2 * J1_local) * J1_local) * U5_local;
		perturbationMeanLongitude = perturbationMeanLongitude - (1.0497e-2 - (1.60906e-1 - 4.1e-3 * J1_local) * J1_local) * U6_local;
		perturbationMeanLongitude = perturbationMeanLongitude - 1.5208e-2 * UD_local - 6.339e-3 * UF_local - 6.244e-3 * U1_local;
		perturbationMeanLongitude = perturbationMeanLongitude - 1.65e-2 * UB_local * U1_local - 4.0786e-2 * UB_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (8.931e-3 + 2.728e-3 * J1_local) * U9_local * U1_local - 5.775e-3 * UD_local * U1_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (8.1344e-2 + 3.206e-3 * J1_local) * UA_local * U1_local + 1.5019e-2 * UC_local * U1_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (8.5581e-2 + 2.494e-3 * J1_local) * U9_local * U2_local + 1.4394e-2 * UC_local * U2_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (2.5328e-2 - 3.117e-3 * J1_local) * UA_local * U2_local + 6.319e-3 * UE_local * U2_local;
		perturbationMeanLongitude = perturbationMeanLongitude + 6.369e-3 * U9_local * U3_local + 9.156e-3 * UB_local * U3_local + 7.525e-3 * UQ_local * U3_local;
		perturbationMeanLongitude = perturbationMeanLongitude - 5.236e-3 * UA_local * U4_local - 7.736e-3 * UC_local * U4_local - 7.528e-3 * UR_local * U4_local;
		perturbationMeanLongitude = deg2rad(perturbationMeanLongitude);
		
		perturbationEccentricity = (-7927. + (2548. + 91. * J1_local) * J1_local) * U5_local;
		perturbationEccentricity = perturbationEccentricity + (13381. + (1226. - 253. * J1_local) * J1_local) * U6_local + (248. - 121. * J1_local) * U7_local;
		perturbationEccentricity = perturbationEccentricity - (305. + 91. * J1_local) * VI_local + 412. * UB_local + 12415. * U1_local;
		perturbationEccentricity = perturbationEccentricity + (390. - 617. * J1_local) * U9_local * U1_local + (165. - 204. * J1_local) * UB_local * U1_local;
		perturbationEccentricity = perturbationEccentricity + 26599. * UA_local * U1_local - 4687. * UC_local * U1_local - 1870. * UE_local * U1_local - 821. * UG_local * U1_local;
		perturbationEccentricity = perturbationEccentricity - 377. * VH_local * U1_local + 497. * UP_local * U1_local + (163. - 611. * J1_local) * U2_local;
		perturbationEccentricity = perturbationEccentricity - 12696. * U9_local * U2_local - 4200. * UB_local * U2_local - 1503. * UD_local * U2_local - 619. * UF_local * U2_local;
		perturbationEccentricity = perturbationEccentricity - 268. * UN_local * U2_local - (282. + 1306. * J1_local) * UA_local * U2_local;
		perturbationEccentricity = perturbationEccentricity + (-86. + 230. * J1_local) * UC_local * U2_local + 461. * UO_local * U2_local - 350. * U3_local;
		perturbationEccentricity = perturbationEccentricity + (2211. - 286. * J1_local) * U9_local * U3_local - 2208. * UB_local * U3_local - 568. * UD_local * U3_local;
		perturbationEccentricity = perturbationEccentricity - 346. * UF_local * U3_local - (2780. + 222. * J1_local) * UA_local * U3_local;
		perturbationEccentricity = perturbationEccentricity + (2022. + 263. * J1_local) * UC_local * U3_local + 248. * UE_local * U3_local + 242. * UQ_local * U3_local;
		perturbationEccentricity = perturbationEccentricity + 467. * UR_local * U3_local - 490. * U4_local - (2842. + 279. * J1_local) * U9_local * U4_local;
		perturbationEccentricity = perturbationEccentricity + (128. + 226. * J1_local) * UB_local * U4_local + 224. * UD_local * U4_local;
		perturbationEccentricity = perturbationEccentricity + (-1594. + 282. * J1_local) * UA_local * U4_local + (2162. - 207. * J1_local) * UC_local * U4_local;
		perturbationEccentricity = perturbationEccentricity + 561. * UE_local * U4_local + 343. * UG_local * U4_local + 469. * UQ_local * U4_local - 242. * UR_local * U4_local;
		perturbationEccentricity = perturbationEccentricity - 205. * U9_local * UI_local + 262. * UD_local * UI_local + 208. * UA_local * UJ_local - 271. * UE_local * UJ_local;
		perturbationEccentricity = perturbationEccentricity - 382. * UE_local * UK_local - 376. * UD_local * UL_local;
		perturbationEccentricity = perturbationEccentricity * 1.0e-7;
		
		VK_local = (7.7108e-2 + (7.186e-3 - 1.533e-3 * J1_local) * J1_local) * U5_local;
		VK_local = VK_local - 7.075e-3 * U9_local;
		VK_local = VK_local + (4.5803e-2 - (1.4766e-2 + 5.36e-4 * J1_local) * J1_local) * U6_local;
		VK_local = VK_local - 7.2586e-2 * U2_local - 7.5825e-2 * U9_local * U1_local - 2.4839e-2 * UB_local * U1_local;
		VK_local = VK_local - 8.631e-3 * UD_local * U1_local - 1.50383e-1 * UA_local * U2_local;
		VK_local = VK_local + 2.6897e-2 * UC_local * U2_local + 1.0053e-2 * UE_local * U2_local;
		VK_local = VK_local - (1.3597e-2 + 1.719e-3 * J1_local) * U9_local * U3_local + 1.1981e-2 * UB_local * U4_local;
		VK_local = VK_local - (7.742e-3 - 1.517e-3 * J1_local) * UA_local * U3_local;
		VK_local = VK_local + (1.3586e-2 - 1.375e-3 * J1_local) * UC_local * U3_local;
		VK_local = VK_local - (1.3667e-2 - 1.239e-3 * J1_local) * U9_local * U4_local;
		VK_local = VK_local + (1.4861e-2 + 1.136e-3 * J1_local) * UA_local * U4_local;
		VK_local = VK_local - (1.3064e-2 + 1.628e-3 * J1_local) * UC_local * U4_local;
		perturbationMeanAnomaly = perturbationMeanLongitude - (deg2rad(VK_local) / planetaryOrbitalElements[planetNumber][4]);
		perturbationSemiMajorAxis = 572. * U5_local - 1590. * UB_local * U2_local + 2933. * U6_local - 647. * UD_local * U2_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis + 33629. * UA_local - 344. * UF_local * U2_local - 3081. * UC_local + 2885. * UA_local * U2_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis - 1423. * UE_local + (2172. + 102. * J1_local) * UC_local * U2_local - 671. * UG_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis + 296. * UE_local * U2_local - 320. * VH_local - 267. * UB_local * U3_local + 1098. * U1_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis - 778. * UA_local * U3_local - 2812. * U9_local * U1_local + 495. * UC_local * U3_local + 688. * UB_local * U1_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis + 250. * UE_local * U3_local - 393. * UD_local * U1_local - 856. * U9_local * U4_local - 228. * UF_local * U1_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis + 441. * UB_local * U4_local + 2138. * UA_local * U1_local + 296. * UC_local * U4_local - 999. * UC_local * U1_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis + 211. * UE_local * U4_local - 642. * UE_local * U1_local - 427. * U9_local * UI_local - 325. * UG_local * U1_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis + 398. * UD_local * UI_local - 890. * U2_local + 344. * UA_local * UJ_local + 2206. * U9_local * U2_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis - 427. * UE_local * UJ_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis * 1.0e-6;
		
		perturbationHeliocentricEclipticLatitude = 7.47e-4 * UA_local * U1_local + 1.069e-3 * UA_local * U2_local + 2.108e-3 * UB_local * U3_local;
		perturbationHeliocentricEclipticLatitude = perturbationHeliocentricEclipticLatitude + 1.261e-3 * UC_local * U3_local + 1.236e-3 * UB_local * U4_local - 2.075e-3 * UC_local * U4_local;
		perturbationHeliocentricEclipticLatitude = deg2rad(perturbationHeliocentricEclipticLatitude);
	  } else if (planetNumber == 6) {
        //Uranus
		JA_local = J4_local - J2_local;
		JB_local = J4_local - J3_local;
		JC_local = J8_local - J4_local;
		perturbationMeanLongitude = (8.64319e-1 - 1.583e-3 * J1_local) * VJ_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (8.2222e-2 - 6.833e-3 * J1_local) * UU_local + 3.6017e-2 * UV_local;
		perturbationMeanLongitude = perturbationMeanLongitude - 3.019e-3 * UW_local + 8.122e-3 * sin(J6_local);
		perturbationMeanLongitude = deg2rad(perturbationMeanLongitude);
		VK_local = 1.20303e-1 * VJ_local + 6.197e-3 * UV_local;
		VK_local = VK_local + (1.9472e-2 - 9.47e-4 * J1_local) * UU_local;
		perturbationMeanAnomaly = perturbationMeanLongitude - (deg2rad(VK_local) / planetaryOrbitalElements[planetNumber][4]);
		perturbationEccentricity = (163. * J1_local - 3349.) * VJ_local + 20981. * UU_local + 1311. * UW_local;
		perturbationEccentricity = perturbationEccentricity * 1.0e-7;
		perturbationSemiMajorAxis = -3.825e-3 * UU_local;
		perturbationLongitude = (-3.8581e-2 + (2.031e-3 - 1.91e-3 * J1_local) * J1_local) * cos(J4_local + JB_local);
		perturbationLongitude = perturbationLongitude + (1.0122e-2 - 9.88e-4 * J1_local) * sin(J4_local + JB_local);
		A_local = (3.4964e-2 - (1.038e-3 - 8.68e-4 * J1_local) * J1_local) * cos(2. * J4_local + JB_local);
        perturbationLongitude = A_local + perturbationLongitude + 5.594e-3 * sin(J4_local + 3. * JC_local) - 1.4808e-2 * sin(JA_local);
        perturbationLongitude = perturbationLongitude - 5.794e-3 * sin(JB_local) + 2.347e-3 * cos(JB_local);
		perturbationLongitude = perturbationLongitude + 9.872e-3 * sin(JC_local) + 8.803e-3 * sin(2. * JC_local);
		perturbationLongitude = perturbationLongitude - 4.308e-3 * sin(3. * JC_local);
		
		UX_local = sin(JB_local);
		UY_local = cos(JB_local);
		UZ_local = sin(J4_local);
		VA_local = cos(J4_local);
		VB_local = sin(2. * J4_local);
		VC_local = cos(2. * J4_local);
		perturbationHeliocentricEclipticLatitude = (4.58e-4 * UX_local - 6.42e-4 * UY_local - 5.17e-4 * cos(4. * JC_local)) * UZ_local;
		perturbationHeliocentricEclipticLatitude = perturbationHeliocentricEclipticLatitude - (3.47e-4 * UX_local + 8.53e-4 * UY_local + 5.17e-4 * sin(4. * JB_local)) * VA_local;
		perturbationHeliocentricEclipticLatitude = perturbationHeliocentricEclipticLatitude + 4.03e-4 * (cos(2. * JC_local) * VB_local + sin(2. * JC_local) * VC_local);
		perturbationHeliocentricEclipticLatitude = deg2rad(perturbationHeliocentricEclipticLatitude);
		
		perturbationRadiusVector = -25948. + 4985. * cos(JA_local) - 1230. * VA_local + 3354. * UY_local;
		perturbationRadiusVector = perturbationRadiusVector + 904. * cos(2. * JC_local) + 894. * (cos(JC_local) - cos(3. * JC_local));
		perturbationRadiusVector = perturbationRadiusVector + (5795. * VA_local - 1165. * UZ_local + 1388. * VC_local) * UX_local;
		perturbationRadiusVector = perturbationRadiusVector + (1351. * VA_local + 5702. * UZ_local + 1388. * VB_local) * UY_local;
		perturbationRadiusVector = perturbationRadiusVector * 1.0e-6;
	  } else if (planetNumber == 7) {
        //Neptune
		JA_local = J8_local - J2_local;
		JB_local = J8_local - J3_local;
		JC_local = J8_local - J4_local;
		perturbationMeanLongitude = (1.089e-3 * J1_local - 5.89833e-1) * VJ_local;
		perturbationMeanLongitude = perturbationMeanLongitude + (4.658e-3 * J1_local - 5.6094e-2) * UU_local - 2.4286e-2 * UV_local;
		perturbationMeanLongitude = deg2rad(perturbationMeanLongitude);
		
		VK_local = 2.4039e-2 * VJ_local - 2.5303e-2 * UU_local + 6.206e-3 * UV_local;
		
		VK_local = VK_local - 5.992e-3 * UW_local;
		perturbationMeanAnomaly = perturbationMeanLongitude - (deg2rad(VK_local) / planetaryOrbitalElements[planetNumber][4]);
		perturbationEccentricity = 4389. * VJ_local + 1129. * UV_local + 4262. * UU_local + 1089. * UW_local;
		perturbationEccentricity = perturbationEccentricity * 1.0e-7;
		perturbationSemiMajorAxis = 8189. * UU_local - 817. * VJ_local + 781. * UW_local;
		perturbationSemiMajorAxis = perturbationSemiMajorAxis * 1.0e-6;
		VD_local = sin(2. * JC_local);
		VE_local = cos(2. * JC_local);
		VF_local = sin(J8_local);
		VG_local = cos(J8_local);
		perturbationLongitude = -9.556e-3 * sin(JA_local) - 5.178e-3 * sin(JB_local);
		perturbationLongitude = perturbationLongitude + 2.572e-3 * VD_local - 2.972e-3 * VE_local * VF_local - 2.833e-3 * VD_local * VG_local;
		perturbationHeliocentricEclipticLatitude = 3.36e-4 * VE_local * VF_local + 3.64e-4 * VD_local * VG_local;
		perturbationHeliocentricEclipticLatitude = deg2rad(perturbationHeliocentricEclipticLatitude);
		perturbationRadiusVector = -40596. + 4992. * cos(JA_local) + 2744. * cos(JB_local);
		perturbationRadiusVector = perturbationRadiusVector + 2044. * cos(JC_local) + 1051. * VE_local;
		perturbationRadiusVector = perturbationRadiusVector * 1.0e-6;
	  }
	}
	SP_eccentricity = planetaryOrbitalElements[planetNumber][4] + perturbationEccentricity;
	SP_meanAnomaly = planetAnomalies[planetNumber] + perturbationMeanAnomaly;
	doAnomaly(rad2deg(SP_meanAnomaly), SP_eccentricity);
	radiusVectorCorrected = (planetaryOrbitalElements[planetNumber][7] + perturbationSemiMajorAxis) * (1.0 - SP_eccentricity * SP_eccentricity) / (1.0 + SP_eccentricity * cos(trueAnomaly));
	LP_local = rad2deg(trueAnomaly) + planetaryOrbitalElements[planetNumber][3] + rad2deg(perturbationMeanLongitude - perturbationMeanAnomaly);
	LP_local = deg2rad(LP_local);
	LongitudeAscendingNode = deg2rad(planetaryOrbitalElements[planetNumber][6]);
	LO_local = LP_local - LongitudeAscendingNode;
	SO_local = sin(LO_local);
	CO_local = cos(LO_local);
	inclination = deg2rad(planetaryOrbitalElements[planetNumber][5]);
	radiusVectorCorrected = radiusVectorCorrected + perturbationRadiusVector;
	SP_local = SO_local * sin(inclination);
	Y_local = SO_local * cos(inclination);
	PS_local = asin(SP_local) + perturbationHeliocentricEclipticLatitude;
	SP_local = sin(PS_local);
	PD_local = atan(Y_local / CO_local) + LongitudeAscendingNode + deg2rad(perturbationLongitude);
	if (CO_local < 0.) PD_local = PD_local + FPI;
	if (PD_local > F2PI) PD_local = PD_local - F2PI;
	cosine_PS_local = cos(PS_local);
	RD_local = radiusVectorCorrected * cosine_PS_local;
	LL_local = PD_local - earthEclipticLongitude;
	distanceEarthCorrected = radiusVectorEarth * radiusVectorEarth + radiusVectorCorrected * radiusVectorCorrected - 2.0 * radiusVectorEarth * radiusVectorCorrected * cosine_PS_local * cos(LL_local);
	distanceEarthCorrected = sqrt(distanceEarthCorrected); //Earth-planet distance in azimuthRising
	lightTravelTime = distanceEarthCorrected * 5.775518e-3;
	if (K_local == 1) {
      heliocenttricEclipticLongitude = PD_local; //heliocentric ecliptic longitude (radians)
	  distanceEarthNotCorrected = distanceEarthCorrected; //distance from Earth in azimuthRising
	  heliocenttricEclipticLatitude = PS_local; //heliocentric latitude in radians
	  radiusVectorPlanet = radiusVectorCorrected; //radius vector (azimuthRising)
	}
  }

  L1_local = sin(LL_local);
  L2_local = cos(LL_local);
  if (planetNumber < 3) {
    geocentricEclipticLongitude = atan(-1.0 * RD_local * L1_local / (radiusVectorEarth - RD_local * L2_local)) + earthEclipticLongitude + FPI;
  } else {
    geocentricEclipticLongitude = atan(radiusVectorEarth * L1_local / (RD_local - radiusVectorEarth * L2_local)) + PD_local;
  }
  geocentricEclipticLongitude = inRange2PI(geocentricEclipticLongitude); //geocentric ecliptic longitude
  geocentricEclipticLatitude = atan(RD_local * SP_local * sin(geocentricEclipticLongitude - PD_local) / (cosine_PS_local * radiusVectorEarth * L1_local)); //geocentric ecliptic latitude
  //end of standard routine here
  //EclLongitude = geocentricEclipticLongitude, EclLatitude = geocentricEclipticLatitude
  double EPtemp = geocentricEclipticLongitude;
  double BPtemp = geocentricEclipticLatitude;
  doNutation();
  geocentricEclipticLongitude = geocentricEclipticLongitude + deg2rad(nutationInLongitude);
  A_local = earthEclipticLongitude + FPI - geocentricEclipticLongitude;
  double B_local = cos(A_local);
  double C_local = sin(A_local);
  geocentricEclipticLongitude = geocentricEclipticLongitude - (9.9387e-5 * B_local / cos(geocentricEclipticLatitude));
  geocentricEclipticLatitude = geocentricEclipticLatitude - (9.9387e-5 * C_local * sin(geocentricEclipticLatitude));
  //setEcliptic(geocentricEclipticLongitude, geocentricEclipticLatitude);
  EclLongitude = geocentricEclipticLongitude;
  EclLatitude = geocentricEclipticLatitude;
  doEcliptic2RAdec();
  EclLongitude = EPtemp;
  EclLatitude = BPtemp;
  return true;
}

double SiderealPlanets::getHelioLong(void) {
  return rad2deg(heliocenttricEclipticLongitude);
}

double SiderealPlanets::getHelioLat(void) {
  return rad2deg(heliocenttricEclipticLatitude);
}

double SiderealPlanets::getRadiusVec(void) {
  return (radiusVectorPlanet);
}

double SiderealPlanets::getDistance(void) {
  return (distanceEarthNotCorrected);
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
  double horizonVerticalDisplacement = 1.454441e-2;
  double tmpGMT = GMTtime;
  GMTtime = (12.0 + (TimeZoneOffset + DSToffset));
  doSun();
  GMTtime = tmpGMT;
  if (doRiseSetTimes(rad2deg(horizonVerticalDisplacement)) == false) return false;
  
  double LA_local = localSiderealTimeRising; //localSiderealTime of rising - first guesstimate
  double LB_local = localSiderealTimeSetting; //localSiderealTime of setting - first guesstimate
  double GU_local = doLST2GMT(LA_local);
  double GD_local = doLST2GMT(LB_local);
  
  GMTtime = GU_local;
  double DN_local = mjd1900;
  double A_local = GMTtime + TimeZoneOffset + DSToffset;
  if (A_local > 24.) mjd1900 -= 1;
  if (A_local < 0.)  mjd1900 += 1;
  doSun();
  mjd1900 = DN_local;
  GMTtime = tmpGMT;
  if (doRiseSetTimes(rad2deg(horizonVerticalDisplacement)) == false) return false;
  LA_local = localSiderealTimeRising; //AA_local = azimuthRising;
  
  GMTtime = GD_local;
  A_local = GMTtime + TimeZoneOffset + DSToffset;
  if (A_local > 24.) mjd1900 -= 1;
  if (A_local < 0.)  mjd1900 += 1;
  doSun();
  mjd1900 = DN_local;
  GMTtime = tmpGMT;
  if (doRiseSetTimes(rad2deg(horizonVerticalDisplacement)) == false) return false;
  
  LB_local = localSiderealTimeSetting; //AB_local = azimuthSetting;
  localSiderealTimeRising = LA_local;
  return true;
}

double SiderealPlanets::getSunriseTime(void) {
  return getRiseTime();
}

double SiderealPlanets::getSunsetTime(void) {
  return getSetTime();
}

boolean SiderealPlanets::doMoonRiseSetTimes(void) {
  double DN_local, horizonVerticalDisplacement, A_local, TH_local, AA_local, AB_local, GU_local, GD_local;
  double tmpGMT = GMTtime;
  GMTtime = (12.0 + (TimeZoneOffset + DSToffset)); //Set to local mid-day
  
  //local rise-set routine
  doMoon(); //Already does nutation too
  TH_local = 2.7249e-1 * sin(moonHorizontalParallax);
  horizonVerticalDisplacement = TH_local + 9.8902e-3 - moonHorizontalParallax;
  // return if moon doesn't cross horizon
  if (doRiseSetTimes(rad2deg(horizonVerticalDisplacement)) == false) {
	  GMTtime = tmpGMT;
	  return false;
  }

  double LA_local = localSiderealTimeRising; //localSiderealTime of rising - first guesstimate
  double LB_local = localSiderealTimeSetting; //localSiderealTime of setting - first guesstimate
  for(int K_local=1; K_local <= 3; K_local++) {
    // local sidereal time to local civil time
    GU_local = doLST2GMT(LA_local);
    // local sidereal time to local civil time
    GD_local = doLST2GMT(LB_local);
	//find a better time of rising
    GMTtime = GU_local;
    // find time
    DN_local = mjd1900;
    A_local = GMTtime + TimeZoneOffset + DSToffset;
    if (A_local > 24.) mjd1900 -= 1;
    if (A_local < 0.) mjd1900 += 1;
    //local rise-set routine
    doMoon(); //Already does nutation too
    TH_local = 2.7249e-1 * sin(moonHorizontalParallax);
    horizonVerticalDisplacement = TH_local + 9.8902e-3 - moonHorizontalParallax;
    if (doRiseSetTimes(rad2deg(horizonVerticalDisplacement)) == false) {
		mjd1900=DN_local;
		GMTtime = tmpGMT;
		return false;
	}
    mjd1900=DN_local;
    //find a better time of setting
    LA_local = localSiderealTimeRising;
    //AA_local = azimuthRising;
    GMTtime = GD_local;
    // find time
    DN_local = mjd1900;
    A_local = GMTtime + TimeZoneOffset + DSToffset;
    if (A_local > 24.) mjd1900 -= 1;
    if (A_local < 0.) mjd1900 += 1;
    //local rise-set routine
    doMoon(); //Already does nutation too
    TH_local = 2.7249e-1 * sin(moonHorizontalParallax);
    horizonVerticalDisplacement = TH_local + 9.8902e-3 - moonHorizontalParallax;
    if (doRiseSetTimes(rad2deg(horizonVerticalDisplacement)) == false) {
		mjd1900=DN_local;
		GMTtime = tmpGMT;
		return false;
	}
    mjd1900=DN_local;
    LB_local = localSiderealTimeSetting;
    //AB_local = azimuthSetting;
  }
  //azimuthRising = AA_local;
  //azimuthSetting = AB_local;
  localSiderealTimeRising = LA_local;
  localSiderealTimeSetting = LB_local;
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
