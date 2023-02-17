/* Sidereal Planets Dog and Pony Show sketch
 * Version 1.2.0 - February 17, 2023
 * Here we 'show off' the Sidereal Planets Library
 *
 * Requirement #1:
 * This sketch is designed to be output to a TeraTerm serial session.
 * TeraTerm information at:
 * https://learn.sparkfun.com/tutorials/terminal-basics/tera-term-windows
 *
 * Requirement #2:
 * To use this sketch, you must have a Ublox GPS receiver on IIC
 * This sketch has been tested with:
 * https://www.sparkfun.com/products/15210
 *
 * Requirement #3:
 * Set the Time Zone offset from UTC below.
*/
const int TZONE  = -8;  /* PST used for default */

#include <math.h>
#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <Wire.h>  // Include Wire if you're using I2C
#include <SiderealPlanets.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_Ublox_GPS

// Need the following define for SAMD processors
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

const char positionmsg1[] = "        Current                Right";
const char positionmsg2[] = "Object  Azimuth    Altitude    Ascension  Declination   Rise     Set\n";
  
SiderealPlanets myAstro;
SFE_UBLOX_GNSS ubloxGPS;
double  GMTyear, GMTmonth, GMTday;
double GMThour, rtcmin, rtcseconds, LST;
double latitude, longitude, elevation;
int line;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(5000); //SAMD boards may need a long time to init SerialUSB
  TERMclear();
  TERMcursor();
  TERMtextcolor('b');
  Serial.println("Sidereal Planets Dog-and-Pony-Show");

  myAstro.begin();
  
  Serial.println("Checking for U-blox based GPS.");
  if (ubloxGPS.begin() == false) {
    Serial.println("U-blox based GPS NOT detected. Stopping....");
	while(1); //Freeze
  }
  
  // GPS online
  Serial.println("U-blox based GPS detected.");
  //Note: NOT recommended to use non-blocking mode
  ubloxGPS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  ubloxGPS.saveConfiguration(); //Save the current settings to flash and BBR
  
  //Set up the display
  TERMclear();
  TERMcursor();
  TERMtextcolor('g');
  Serial.println("Sidereal Planets Dog-and-Pony-Show");
  TERMtextcolor('r');
  Serial.println("GMT         GMT      Time");
  Serial.println("Date        Time     Zone  DST Latitude       Longitude       Elevation\n");
  Serial.println("Local Time: 00:00:00   Local Siderial Time:");
  Serial.println("");
  
  Serial.println(positionmsg1);
  Serial.println(positionmsg2);

  //Time Zone
  myAstro.setTimeZone(TZONE);
  TERMtextcolor('w');
  TERMxy(22,4);
  switch (TZONE) {
    case -10: Serial.print("HST"); break;
    case -9:  Serial.print("AKST"); break;
    case -8:  Serial.print("PST"); break;
    case -7:  Serial.print("MST"); break;
    case -6:  Serial.print("CST"); break;
    case -5:  Serial.print("EST"); break;
    case -4:  Serial.print("AST"); break;
    case 0:   Serial.print("UTC"); break;
  }
  
 }
 
void loop() {
  // Update data on status screen
  GMTyear = (int)ubloxGPS.getYear();
  GMTmonth = (byte)ubloxGPS.getMonth();
  GMTday = (byte)ubloxGPS.getDay();
  GMThour = (byte)ubloxGPS.getHour();
  rtcmin = (byte)ubloxGPS.getMinute();
  rtcseconds = ubloxGPS.getSecond();
  elevation = ubloxGPS.getAltitude() * 3.28084 / 1000.; //Elevation in feet
  latitude = ubloxGPS.getLatitude();
  latitude = latitude / 10000000.;
  longitude = ubloxGPS.getLongitude();
  longitude = longitude / 10000000.;

  myAstro.setLatLong(latitude, longitude);
  myAstro.setElevationF(elevation);
  myAstro.setGMTdate(GMTyear,GMTmonth,GMTday);
  myAstro.setGMTtime(GMThour,rtcmin,rtcseconds);
  boolean DSTFLAG = myAstro.useAutoDST();
  LST = myAstro.getLocalSiderealTime();
  
  line = 4;
  Serial.print("  ");
  TERMxy(0,line);
  printDate();
  // Print Time(s)
  TERMxy(13,line); printTime(myAstro.getGMT());
  TERMxy(28,line); // Daylight Savings time flag
  if (DSTFLAG) {
    Serial.print("Yes");
  } else {
    Serial.print("No ");
  }
  //Latitude, Longitude
  TERMxy(32,line);
  printDegMinSecs(fabs(latitude));
  if (latitude > 0 )
      Serial.print("N ");
    else
      Serial.print("S ");
  TERMxy(47,line);
  printDegMinSecs(fabs(longitude));
  if (longitude > 0 )
      Serial.print("E ");
    else
      Serial.print("W ");
  
  // Elevation from GPS
  TERMxy(63,line); Serial.print(elevation); Serial.print(" ");
  
  line++;
  // LT, LST
  TERMxy(13,line); printTime(myAstro.getLT());
  TERMxy(45,line); printTime(LST);

  line = 9;
  //Objects - 
  //Sirius
  TERMxy(0,line); Serial.print("Sirius ");
  myAstro.setRAdec(myAstro.decimalDegrees(6, 45, 9), myAstro.decimalDegrees(-16, 42, 58));
  printDisplayLine(line);
  
  line++;
  //Polaris
  TERMxy(0,line); Serial.print("Polaris ");
  myAstro.setRAdec(myAstro.decimalDegrees(2, 31, 49.09), myAstro.decimalDegrees(89, 15, 50.8));
  printDisplayLine(line);
  
  line++;
  //Sun
  TERMxy(0,line); Serial.print("Sun ");
  myAstro.doSun();
  myAstro.doRAdec2AltAz();
  TERMxy(9,line); printDegMinSecs2(myAstro.getAzimuth());
  TERMxy(20,line); printDegMinSecs2(myAstro.getAltitude());
  TERMxy(32,line); printDegMinSecs2(myAstro.getRAdec());
  TERMxy(44,line); printDegMinSecs2(myAstro.getDeclinationDec());
  TERMxy(55,line);
  if (myAstro.doSunRiseSetTimes()) {
    printTime(myAstro.getSunriseTime());
    printTime(myAstro.getSunsetTime());
  } else {
    Serial.print("No rise/set times");
  }
  Serial.println("");

  line++;
  //Moon
  TERMxy(0,line); Serial.print("Moon ");
  myAstro.doMoon();
  myAstro.doLunarParallax();
  myAstro.doRAdec2AltAz();
  TERMxy(9,line); printDegMinSecs2(myAstro.getAzimuth());
  TERMxy(20,line); printDegMinSecs2(myAstro.getAltitude());
  TERMxy(32,line); printDegMinSecs2(myAstro.getRAdec());
  TERMxy(44,line); printDegMinSecs2(myAstro.getDeclinationDec());
  TERMxy(55,line);
  if (myAstro.doMoonRiseSetTimes()) {
    printTime(myAstro.getMoonriseTime());
    printTime(myAstro.getMoonsetTime());
  } else {
    Serial.print("No rise/set times");
  }
  Serial.println("");
  
  line++;
  //Mercury
  TERMxy(0,line); Serial.print("Mercury ");
  myAstro.doMercury();
  printDisplayLine(line);

  line++;
  //Venus
  TERMxy(0,line); Serial.print("Venus ");
  myAstro.doVenus();
  printDisplayLine(line);
  
  line++;
  //Mars
  TERMxy(0,line); Serial.print("Mars ");
  myAstro.doMars();
  printDisplayLine(line);
  
  line++;
  //Jupiter
  TERMxy(0,line); Serial.print("Jupiter ");
  myAstro.doJupiter();
  printDisplayLine(line);
  
  line++;
  //Saturn
  TERMxy(0,line); Serial.print("Saturn ");
  myAstro.doSaturn();
  printDisplayLine(line);
  
  line++;
  //Uranus
  TERMxy(0,line); Serial.print("Uranus ");
  myAstro.doUranus();
  printDisplayLine(line);
  
  line++;
  //Neptune
  TERMxy(0,line); Serial.print("Neptune ");
  myAstro.doNeptune();
  printDisplayLine(line);

  delay(2000);
}

void printDisplayLine(int line) {
  myAstro.doRAdec2AltAz();
  TERMxy(9,line); printDegMinSecs2(myAstro.getAzimuth());
  TERMxy(20,line); printDegMinSecs2(myAstro.getAltitude());
  TERMxy(32,line); printDegMinSecs2(myAstro.getRAdec());
  TERMxy(44,line); printDegMinSecs2(myAstro.getDeclinationDec());
  TERMxy(55,line);
  if (myAstro.doRiseSetTimes(0.)) {
    printTime(myAstro.getRiseTime());
    printTime(myAstro.getSetTime());
  } else {
    Serial.print("No rise/set times");
  }
  Serial.println("");
}

void printDegMinSecs(double n) {
  boolean sign = (n < 0.);
  if (sign) n = -n;
  long lsec = n * 360000.0;
  long deg = lsec / 360000;
  long min = (lsec - (deg * 360000)) / 6000;
  float secs = (lsec - (deg * 360000) - (min * 6000)) / 100.;
  if (sign) Serial.print("-");
  Serial.print(deg); Serial.print(":");
  print2digitsUSB(min); Serial.print(":");
  Serial.print(abs(secs)); Serial.print(" ");
}

void printDegMinSecs2(double n) {
  boolean sign = false;
  if (n < 0.) {
    n = -n;
	sign = true;
  }
  long lsec = n * 3600.0;
  long deg = lsec / 3600;
  long min = (lsec - (deg * 3600)) / 60;
  long secs = (lsec - (deg * 3600) - (min * 60));
  if (sign) Serial.print("-");
  Serial.print(deg); Serial.print(":");
  print2digitsUSB(min); Serial.print(":");
  print2digitsUSB(secs); Serial.print(" ");
}

void printTime(double n) {
  boolean sign = (n < 0.);
  if (sign) n = -n;
  long lsec = n * 3600.0;
  long deg = lsec / 3600;
  long min = (lsec - (deg * 3600)) / 60;
  float secs = (lsec - (deg * 3600) - (min * 60));
  if (sign) Serial.print("-");
  print2digitsUSB(deg); Serial.print(":");
  print2digitsUSB(min); Serial.print(":");
  print2digitsUSB(abs(secs)); Serial.print(" ");
}

void printDate() {
  // Print date...
  print2digitsUSB(GMTday);
  Serial.print("/");
  print2digitsUSB(GMTmonth);
  Serial.print("/");
  Serial.print((int)GMTyear);
}

void print2digitsUSB(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

void TERMclear() {
  Serial.print(" ");
  delay(500);
  //Clear VT100 screen and home cursor
  Serial.write(0x1b); // esc
  Serial.write(0x5b); // [
  Serial.write(0x48); // H - Go to home position
  Serial.write(0x1b); // esc
  Serial.write(0x5b); // [
  Serial.write(0x32); // 2
  Serial.write(0x4a); // J - Clear the screen
}

void TERMcursor() {
  //Set VT100 cursor to underline
  Serial.write(0x1b); // esc
  Serial.write(0x5b); // [
  Serial.write(3);
  Serial.print(" q");
}

void TERMxy(int x, int y) {
  delay(40); // Serialx has a small Tx buffer
  // Position cursor on VT100 screen at coordinates x,y
  Serial.write(0x1b); // esc
  Serial.write(0x5b); // [
  Serial.print(y); // vertical
  Serial.write(0x3b); // ; or semi-colon
  Serial.print(x); // horizontal
  Serial.write(0x48); // H - Go to x,y position
}

void TERMtextcolor( char buf ) {
  //Set foreground text color on vt100 terminal screen
  int tmp;
  Serial.write(0x1b); // esc
  Serial.write(0x5b); // [
  switch (buf) {
    case 'r':  case 'R': tmp = 31; break;
    case 'g':  case 'G': tmp = 32; break;
    case 'y':  case 'Y': tmp = 33; break;
    case 'b':  case 'B': tmp = 34; break;
    case 'm':  case 'M': tmp = 35; break;
    case 'c':  case 'C': tmp = 36; break;
    default: tmp = 37;
  }
  Serial.print(tmp); // color code
  Serial.write(0x6d); // m
}
