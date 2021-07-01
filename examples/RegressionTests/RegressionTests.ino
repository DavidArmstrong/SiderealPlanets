/* Sidereal Planets regression test sketch
 * Version 1.0.0 - July 1, 2021
 * RegressionTests - Used to validate library functionality
 * These are based on test data published in:
 * 'Astronomy With Your Personal Computer', Second Edition, 
 * by Peter Duffet-Smith. New York: Cambridge University Press, 1990.
*/

#include <SiderealPlanets.h>

// Need the following define for SAMD processors
#if defined (ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

SiderealPlanets myAstro;

void setup() {
  Serial.begin(9600);
  delay(5000); //SAMD boards may need a long time to init SerialUSB
  Serial.println("Sidereal Planets Regression Tests");

  if (myAstro.begin() == false) {
	Serial.println("Target board does not handle Real Double numbers!");
  } else {
	Serial.println("Sidereal Planets is okay with this board.");
  }
  Serial.println("\nCompute decimal degrees from DMS input");
  Serial.println("degrees 238 min 55 sec 10.02 = 238.91945");
  Serial.println(myAstro.decimalDegrees(238, 55, 10.02), 6);
  Serial.println("degrees 0 min -1 sec 20.42 = -0.022338");
  Serial.println(myAstro.decimalDegrees(0, -1, 20.42), 8);
  
  Serial.println("\nModified Julian Date from 0.5 January 1900");
  Serial.println("mjd1900 for 2/2/1989 = 32539.5");
  myAstro.setGMTdate(1989,2,2);
  Serial.println(myAstro.modifiedJulianDate1900());
  Serial.print("Julian Date = ");
  Serial.println(myAstro.modifiedJulianDate1900()+2415020.0);
  Serial.println("mjd1900 for 1/0.5/1900 = 0");
  myAstro.setGMTdate(1900,1,1);
  Serial.println(myAstro.modifiedJulianDate1900()-0.5);
  Serial.print("Julian Date = ");
  Serial.println(myAstro.modifiedJulianDate1900()+2415020.0-0.5);
  Serial.println("mjd1900 for 10/10/1582 = bad = -9999999.00");
  myAstro.setGMTdate(1582,10,10);
  Serial.println(myAstro.modifiedJulianDate1900());
  Serial.print("Julian Date = ");
  Serial.println(myAstro.modifiedJulianDate1900()+2415020.0);
  Serial.println("mjd1900 for 1/1.5/-4713 = -2415020.0");
  myAstro.setGMTdate(-4713,1,1);
  Serial.println(myAstro.modifiedJulianDate1900()+0.5);
  Serial.print("Julian Date = ");
  Serial.println(myAstro.modifiedJulianDate1900()+2415020.0+0.5);
  
  Serial.println("\nCompute Sidereal Times");
  Serial.println("DST on, Time Zone = -10, Long = -148d 31 min 52.33 sec");
  Serial.println("Date: Mar 11, 1990  Local Time: 8:21:43.7");
  Serial.println("RESULTS...UT=17:21:43.7 GST=4:38:9.22 LST=18:44:1.73");
  myAstro.setLatLong(51.178889, myAstro.decimalDegrees(-148,31,52.33));
  myAstro.setTimeZone(-10);
  myAstro.setDST();
  myAstro.setGMTdate(1990,3,11);
  myAstro.setLocalTime(8,21,43.7);
  myAstro.printDegMinSecs(myAstro.getGMT());
  myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  Serial.println("");
  Serial.print("And LST back to Local Time: ");
  myAstro.printDegMinSecs(myAstro.doLST2LT(myAstro.getLocalSiderealTime()));
  Serial.println("");
  Serial.print("And LST back to GMT: ");
  myAstro.printDegMinSecs(myAstro.doLST2GMT(myAstro.getLocalSiderealTime()));
  Serial.println("\n");
  
  Serial.println("RA/Dec to Azimuth/Altitude...");
  Serial.println("DST off, Time Zone = -5, Long = -77d 0 min 0 sec");
  Serial.println("Lat = 38d 55 min 0 sec");
  Serial.println("Date: Feb 1 ,1984  Local Time: 7:23:00");
  Serial.println("RESULTS...UT=12:23:00.0 GST=21:06:37.98 LST=15:58:37.98");
  myAstro.rejectDST();
  myAstro.setTimeZone(-5);
  myAstro.setLatLong(myAstro.decimalDegrees(38,55,0), myAstro.decimalDegrees(-77,0,0));
  myAstro.setGMTdate(1984,2,1);
  myAstro.setLocalTime(7,23,0);
  myAstro.printDegMinSecs(myAstro.getGMT());
  myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  Serial.println("");
  Serial.print("And LST back to Local Time: ");
  myAstro.printDegMinSecs(myAstro.doLST2LT(myAstro.getLocalSiderealTime()));
  Serial.println("");
  Serial.print("And LST back to GMT: ");
  myAstro.printDegMinSecs(myAstro.doLST2GMT(myAstro.getLocalSiderealTime()));
  Serial.println("");
  
  Serial.println("\nRA/Declination to Azimuth/Altitude coordinates");
  Serial.println("RA = 20 hr 40 min 5.2 sec, Dec = -22d 12 min 0.0 sec");
  Serial.println("RESULTS...Az=119:18:14.59 Alt=0:16:15.88");
  myAstro.setRAdec(myAstro.decimalDegrees(20,40,5.2), myAstro.decimalDegrees(-22,12,0));
  myAstro.doRAdec2AltAz();
  myAstro.printDegMinSecs(myAstro.getAzimuth());
  myAstro.printDegMinSecs(myAstro.getAltitude());
  Serial.println();
  Serial.println("And back again...");
  myAstro.setAltAz(myAstro.decimalDegrees(0,16,15.88), myAstro.decimalDegrees(119,18,14.59));
  myAstro.doAltAz2RAdec();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Precess from 1/0.9/1950 to 2/4/1990");
  Serial.println("Input RA = 12 hr 12 min 12 sec, Dec = 23d 23 min 23 sec");
  Serial.println("Output RA = 12 hr 14 min 13.93 sec, Dec = 23d 10 min 0.64 sec");
  myAstro.setGMTdate(1950,1,1);
  myAstro.setRAdec(myAstro.decimalDegrees(12,12,12.), myAstro.decimalDegrees(23,23,23.));
  myAstro.doPrecessTo2000();
  myAstro.setGMTdate(1990,2,4);
  myAstro.doPrecessFrom2000();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  Serial.println("Precess from 2/4/1990 to 1/0.9/1950");
  Serial.println("Input RA = 12 hr 14 min 13.93 sec, Dec = 23d 10 min 0.64 sec");
  Serial.println("Output RA = 12 hr 12 min 12 sec, Dec = 23d 23 min 23 sec");
  myAstro.setGMTdate(1990,2,4);
  myAstro.setRAdec(myAstro.decimalDegrees(12,14,13.93), myAstro.decimalDegrees(23,10,0.64));
  myAstro.doPrecessTo2000();
  myAstro.setGMTdate(1950,1,1);
  myAstro.doPrecessFrom2000();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  Serial.println("Precess from 1/0.9/1950 to 1/1/2050");
  Serial.println("Input RA = 12 hr 12 min 12 sec, Dec = 23d 23 min 23 sec");
  Serial.println("Output RA = 12 hr 17 min 15.84 sec, Dec = 22d 50 min 2.86 sec");
  myAstro.setGMTdate(1950,1,1);
  myAstro.setRAdec(myAstro.decimalDegrees(12,12,12.), myAstro.decimalDegrees(23,23,23.));
  myAstro.doPrecessTo2000();
  myAstro.setGMTdate(2050,1,1);
  myAstro.doPrecessFrom2000();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  Serial.println("Precess from 1/1/2050 to 1/0.9/1950");
  Serial.println("Input RA = 12 hr 17 min 15.84 sec, Dec = 22d 50 min 2.86 sec");
  Serial.println("Output RA = 12 hr 12 min 12 sec, Dec = 23d 23 min 23 sec");
  myAstro.setGMTdate(2050,1,1);
  myAstro.setRAdec(myAstro.decimalDegrees(12,17,15.84), myAstro.decimalDegrees(22,50,2.86));
  myAstro.doPrecessTo2000();
  myAstro.setGMTdate(1950,1,1);
  myAstro.doPrecessFrom2000();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Lunar Parallax 2/26/1979, 10:45:00 LT, Time Zone = -6, Elevation = 60m");
  Serial.println("DST Off, Lat = 50d 0 min 0 sec, Long = -100d 0 min 0.0 sec");
  myAstro.setLatLong(myAstro.decimalDegrees(50,0,0), myAstro.decimalDegrees(-100,0,0.0));
  myAstro.setElevationM(60.0);
  myAstro.rejectDST();
  myAstro.setTimeZone(-6);
  myAstro.setGMTdate(1979,2,26);
  myAstro.setLocalTime(10,45,0.0);
  myAstro.printDegMinSecs(myAstro.getGMT());
  myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  Serial.println("");
  Serial.println("Input RA = 22 hr 35 min 19. sec, Dec = -7d 41 min 13. sec");
  Serial.println("Output RA = 22 hr 36 min 43.22 sec, Dec = -8d 32 min 17.4 sec");
  myAstro.setRAdec(myAstro.decimalDegrees(22,35,19.), myAstro.decimalDegrees(-7,41,13.));
  myAstro.setEquatHorizontalParallax(myAstro.decimalDegrees(1,1,9)); //Function only for testing
  myAstro.doLunarParallax();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Anti-Refraction 3/23/1987, 1:1:24 LT, Time Zone = 0, Elevation 60m");
  Serial.println("DST Off, Lat = 51d 12 min 13 sec, Long = 00d 10 min 12.0 sec");
  myAstro.setLatLong(myAstro.decimalDegrees(51,12,13), myAstro.decimalDegrees(0,10,12.0));
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setGMTdate(1987,3,23);
  myAstro.setLocalTime(1,1,24.0);
  myAstro.setElevationM(60.0);
  myAstro.printDegMinSecs(myAstro.getGMT());
  myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  Serial.println("");
  Serial.println("Input RA = 23 hr 13 min 44.74 sec, Dec = 40d 19 min 45.77 sec");
  Serial.println("Pressure 1012 millibars, Temperature 21.7 C");
  Serial.println("Output RA = 23 hr 14 min 0.0 sec, Dec = 40d 10 min 0.01 sec");
  myAstro.setRAdec(myAstro.decimalDegrees(23,13,44.74), myAstro.decimalDegrees(40,19,45.77));
  myAstro.doRAdec2AltAz();
  Serial.print("Uncorrected Altitude should be 4d 32 min 13.68 sec: ");
  myAstro.printDegMinSecs(myAstro.getAltitude());
  Serial.println();
  myAstro.doAntiRefractionC(1012., 21.7);
  //myAstro.doAntiRefractionF(29.88434, 71.06);
  myAstro.doAltAz2RAdec();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Refraction 3/23/1987, 1:1:24 LT, Time Zone = 0, Elevation = 60m");
  Serial.println("DST Off, Lat = 51d 12 min 13 sec, Long = 00d 10 min 12.0 sec");
  myAstro.setLatLong(myAstro.decimalDegrees(51,12,13), myAstro.decimalDegrees(0,10,12.0));
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setGMTdate(1987,3,23);
  myAstro.setLocalTime(1,1,24.0);
  myAstro.setElevationM(60.0);
  myAstro.printDegMinSecs(myAstro.getGMT());
  myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  Serial.println("");
  Serial.println("Input RA = 23 hr 14 min 0.0 sec, Dec = 40d 10 min 0.0 sec");
  Serial.println("Pressure 1012 millibars, Temperature 21.7 C");
  Serial.println("Output RA = 23 hr 13 min 44.74 sec, Dec = 40d 19 min 45.77 sec");
  myAstro.setRAdec(myAstro.decimalDegrees(23,14,0.0), myAstro.decimalDegrees(40,10,0.0));
  myAstro.doRAdec2AltAz();
  Serial.print("Uncorrected Altitude should be 4d 22 min 2.41 sec: ");
  myAstro.printDegMinSecs(myAstro.getAltitude());
  Serial.println();
  myAstro.doRefractionC(1012., 21.7);
  //myAstro.doRefractionF(29.88434, 71.06);
  myAstro.doAltAz2RAdec();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Mean Anomaly = 220d 23m 10s; Eccentricity = 0.0167183");
  Serial.println("Eccentric Anomaly = 219d 46m 23.76s; True Anomaly = -140d 50m 8.41s");
  myAstro.printDegMinSecs(myAstro.doAnomaly(myAstro.decimalDegrees(220,23,10.0), 0.0167183));
  myAstro.printDegMinSecs(myAstro.getTrueAnomaly());
  Serial.println("\n");
  Serial.println("Mean Anomaly = 45d 0m 0s; Eccentricity = 0.96629");
  Serial.println("Eccentric Anomaly = 99d 35m 25.95s; True Anomaly = 167d 22m 11.55s");
  myAstro.printDegMinSecs(myAstro.doAnomaly(myAstro.decimalDegrees(45,0,0.0), 0.96629));
  myAstro.printDegMinSecs(myAstro.getTrueAnomaly());
  Serial.println("\n");
  
  Serial.println("Nutation: Date: 4,2,1989");
  Serial.println("In Longitude = 0d 0m 8.30s; In Obliquity = 0d 0m 8.26s");
  myAstro.setGMTdate(1989,2,4);
  myAstro.doNutation();
  myAstro.printDegMinSecs(myAstro.getDP());
  myAstro.printDegMinSecs(myAstro.getDO());
  Serial.println("\n");
  Serial.println("Nutation: Date: 1,1,2000");
  Serial.println("In Longitude = -0d 0m 13.96s; In Obliquity = -0d 0m 5.76s");
  myAstro.setGMTdate(2000,1,1);
  myAstro.doNutation();
  myAstro.printDegMinSecs(myAstro.getDP());
  myAstro.printDegMinSecs(myAstro.getDO());
  Serial.println("\n");
  Serial.println("Nutation: Date: 23,4,1995");
  Serial.println("In Longitude = 0d 0m 9.53s; In Obliquity = -0d 0m 7.25s");
  myAstro.setGMTdate(1995,4,23);
  myAstro.doNutation();
  myAstro.printDegMinSecs(myAstro.getDP());
  myAstro.printDegMinSecs(myAstro.getDO());
  Serial.println("\n");
  
  Serial.println("Obliquity: Date: 4,2,1989");
  Serial.println("Obliquity = 23d 26m 34.79s");
  myAstro.setGMTdate(1989,2,4);
  myAstro.printDegMinSecs(myAstro.doObliquity());
  Serial.println("\n");
  
  Serial.println("Ecliptic to RA/Dec: Date: 28,5,2004");
  Serial.println("Ecliptic Long = 277d 0m 4.40s; Ecliptic Lat = -66d 24m 13.10s");
  Serial.println("RA = 0h 0m 5.5s; Dec = -87d 12m 12s");
  myAstro.setGMTdate(2004,5,28);
  myAstro.setEcliptic(myAstro.decimalDegrees(277,0,4.40), myAstro.decimalDegrees(-66,24,13.10));
  myAstro.doEcliptic2RAdec();
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Sun: Local Date: 23,8,1984");
  Serial.println("No DST; Time Zone = -5; Local Time = 19h 0m 0s");
  Serial.println("So GMT Date = 24.8.1984");
  Serial.println("Apparent Ecliptic Longitude: 151d 0m 12.39s");
  Serial.println("RA = 10h 12m 11.37s; Dec = 11d 7m 9.01s");
  myAstro.setTimeZone(-5);
  myAstro.rejectDST();
  myAstro.setGMTdate(1984,8,24);
  myAstro.setLocalTime(19,0,0.0);
  myAstro.printDegMinSecs(myAstro.getGMT());
  myAstro.doSun();
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\ndouble check...");
  Serial.println("No DST; Time Zone = 0; Local Time = 0h 0m 0s");
  myAstro.setTimeZone(0);
  myAstro.rejectDST();
  myAstro.setGMTdate(1984,8,24);
  myAstro.setLocalTime(0,0,0.0);
  myAstro.printDegMinSecs(myAstro.getGMT());
  myAstro.doSun();
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");

  Serial.println("Rise/Set times: DST Off, Time Zone = 4, Date: Sep 23, 1992");
  Serial.println("Long = 62d 10 min 12 sec; Lat = -20d 0 min 3.4 sec");
  Serial.println("RA = 12h 16m 0.0s; Dec = 14d 34m 0.0s");
  Serial.println("Vertical Displacement: 34 min");
  Serial.println("Local Rise=6:17:47.81 Local Set=17:37:31.22");
  myAstro.rejectDST();
  myAstro.setTimeZone(4);
  myAstro.setLatLong(myAstro.decimalDegrees(-20,0,3.4), myAstro.decimalDegrees(62,10,12));
  myAstro.setGMTdate(1992,9,23);
  myAstro.setRAdec(myAstro.decimalDegrees(12,16,0.0), myAstro.decimalDegrees(14,34,0.0));
  myAstro.doRiseSetTimes(myAstro.decimalDegrees(0,34,0.0));
  myAstro.printDegMinSecs(myAstro.getRiseTime());
  myAstro.printDegMinSecs(myAstro.getSetTime());
  Serial.println("\n");
  
  Serial.println("Sun Rise/Set Times: DST Off, Time Zone = 0, Date: Oct 1, 1984");
  Serial.println("Long = 0d 0 min 0 sec; Lat = 52d 0 min 0 sec");
  Serial.println("Local Rise=6:0:58.12, Local Set=17:37:10.86");
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(52,0,0), myAstro.decimalDegrees(0,0,0));
  myAstro.setGMTdate(1984,10,1);
  myAstro.doSunRiseSetTimes();
  myAstro.printDegMinSecs(myAstro.getSunriseTime());
  myAstro.printDegMinSecs(myAstro.getSunsetTime());
  Serial.println("\n");
  
  int IP;
  Serial.println("Pelements: Jupiter, Date: Jul 24, 1992");
  myAstro.setGMTdate(1992,7,24);
  myAstro.doPlanetElements();
  IP = 4;
  myAstro.printDegMinSecs(myAstro.getPL(IP,1));
  myAstro.printDegMinSecs(myAstro.getPL(IP,2));
  myAstro.printDegMinSecs(myAstro.getPL(IP,3)); Serial.println("");
  Serial.println(floor((myAstro.getPL(IP,4) * 1e6) + 0.5) / 1e6, 8);
  myAstro.printDegMinSecs(myAstro.getPL(IP,5));
  myAstro.printDegMinSecs(myAstro.getPL(IP,6)); Serial.println("");
  Serial.println(floor((myAstro.getPL(IP,7) * 1e6) + 0.5) / 1e6, 8);
  myAstro.printDegMinSecs(myAstro.getPL(IP,8) / 3600.0);
  Serial.println(myAstro.getPL(IP,9));
  Serial.println("\n");

  Serial.println("Pelements: Mercury, Date: Jan 21, 1984");
  myAstro.setGMTdate(1984,1,21);
  myAstro.doPlanetElements();
  IP = 1;
  myAstro.printDegMinSecs(myAstro.getPL(IP,1));
  myAstro.printDegMinSecs(myAstro.getPL(IP,2));
  myAstro.printDegMinSecs(myAstro.getPL(IP,3)); Serial.println("");
  Serial.println(floor((myAstro.getPL(IP,4) * 1e6) + 0.5) / 1e6, 8);
  myAstro.printDegMinSecs(myAstro.getPL(IP,5));
  myAstro.printDegMinSecs(myAstro.getPL(IP,6)); Serial.println("");
  Serial.println(floor((myAstro.getPL(IP,7) * 1e6) + 0.5) / 1e6, 8);
  myAstro.printDegMinSecs(myAstro.getPL(IP,8) / 3600.0);
  Serial.println(myAstro.getPL(IP,9));
  Serial.println("\n");
  
  Serial.println("Pelements: Saturn, Date: Jul 24, 1992");
  myAstro.setGMTdate(1992,7,24);
  myAstro.doPlanetElements();
  IP = 5;
  myAstro.printDegMinSecs(myAstro.getPL(IP,1));
  myAstro.printDegMinSecs(myAstro.getPL(IP,2));
  myAstro.printDegMinSecs(myAstro.getPL(IP,3)); Serial.println("");
  Serial.println(floor((myAstro.getPL(IP,4) * 1e6) + 0.5) / 1e6, 8);
  myAstro.printDegMinSecs(myAstro.getPL(IP,5));
  myAstro.printDegMinSecs(myAstro.getPL(IP,6)); Serial.println("");
  Serial.println(floor((myAstro.getPL(IP,7) * 1e6) + 0.5) / 1e6, 8);
  myAstro.printDegMinSecs(myAstro.getPL(IP,8) / 3600.0);
  Serial.println(myAstro.getPL(IP,9));
  Serial.println("\n");

  Serial.println("Planets: Date: May 30, 1984");
  myAstro.setGMTdate(1984,5,30);
  myAstro.setGMTtime(0,0,0);
  Serial.println("Mercury");
  Serial.println("Helio Ecliptic Long: -34:46:20.12, Lat: -6:57:5.34");
  Serial.println("Radius vector: 0.401741, Distance from Earth: 0.999923");
  Serial.println("Geo Ecliptic Long: 45:55:55.89, Lat: -2:47:16.47");
  Serial.println("RA: 2:57:8.28, Dec: 13:56:8.81");
  myAstro.doMercury();
  myAstro.printDegMinSecs(myAstro.getHelioLong());
  myAstro.printDegMinSecs(myAstro.getHelioLat()); Serial.println("");
  Serial.println(myAstro.getRadiusVec(), 9);
  Serial.println(myAstro.getDistance(), 9);
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  myAstro.setGMTdate(1984,5,30);
  myAstro.setGMTtime(0,0,0);
  Serial.println("Saturn");
  Serial.println("Helio Ecliptic Long: 223:55:33.24, Lat: 2:19:49.17");
  Serial.println("Radius vector: 9.866205, Distance from Earth: 8.957211");
  Serial.println("Geo Ecliptic Long: 221:11:41.66, Lat: 2:34:1.15");
  Serial.println("RA: 14:38:19.72, Dec: -12:44:52.50");
  myAstro.doSaturn();
  myAstro.printDegMinSecs(myAstro.getHelioLong());
  myAstro.printDegMinSecs(myAstro.getHelioLat()); Serial.println("");
  Serial.println(myAstro.getRadiusVec(), 9);
  Serial.println(myAstro.getDistance(), 9);
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Uranus");
  //Serial.println("Helio Ecliptic Long: 223:55:33.24, Lat: 2:19:49.17");
  //Serial.println("Radius vector: , Distance from Earth: ");
  Serial.println("Geo Ecliptic Long: 284:25:28.66");
  Serial.println("RA: 19:02:51.09, Dec: -23:04:30.68");
  myAstro.setGMTdate(1992,10,23);
  myAstro.setGMTtime(0,0,0);
  myAstro.doUranus();
  //myAstro.printDegMinSecs(myAstro.getHelioLong());
  //myAstro.printDegMinSecs(myAstro.getHelioLat()); Serial.println("");
  //Serial.println(myAstro.getRadiusVec(), 9);
  //Serial.println(myAstro.getDistance(), 9);
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Moon: no DST, Time Zone = 0, LT = 0, Date: Feb 25,1984");
  myAstro.setTimeZone(0);
  myAstro.rejectDST();
  myAstro.setGMTdate(1984,2,25);
  myAstro.setLocalTime(0,0,0.0);
  Serial.println("Geo Ecliptic Long: 260:42:46.34, Lat: -0:45:28.16");
  Serial.println("RA: 17:19:20.65, Dec: -23:52:22.51");
  myAstro.doMoon();
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Moon: DST, Time Zone = 6, LT = 11:42:20, Date: Aug 14, 1989");
  myAstro.setTimeZone(6);
  myAstro.setDST();
  myAstro.setGMTdate(1989,8,14);
  myAstro.setLocalTime(11,42,20.0);
  //Serial.println("Geo Ecliptic Long: 283:24:1.33, Lat: -3:34:4.63");
  Serial.println("RA: 18:59:49.08, Dec: -26:19:3.37");
  myAstro.doMoon();
  //myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  //myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Moon: No DST, Time Zone = -6, LT = 10:45:00, Date: 2/26/1979");
  Serial.println("Lat = 50:00:00 sec, Long = -100:00:0.0 sec");
  myAstro.setTimeZone(-6);
  myAstro.rejectDST();
  myAstro.setGMTdate(1979,2,26);
  myAstro.setLocalTime(10,45,0.0);
  myAstro.setLatLong(myAstro.decimalDegrees(50,0,0), myAstro.decimalDegrees(-100,0,0.0));
  Serial.println("Geo Ecliptic Long: 283:24:1.33, Lat: -3:34:4.63");
  Serial.println("RA: 22:35:19.0, Dec: -7:41:13.0");
  myAstro.doMoon();
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println();
  Serial.print("Equatorial Horizontal Parallax = 1:1:9 ==> ");
  myAstro.printDegMinSecs(myAstro.getEquatHorizontalParallax());
  Serial.println("\n");
  
  Serial.println("Moon Rise/Set: no DST, Time Zone = 0,  Date: Jan 7,1984");
  Serial.println("Geo Long: 0:0:0.0, Lat: 30:0:0.0");
  Serial.println("local Rise=9:58:1.46 local Set=21:8:59.44");
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(30,0,0), myAstro.decimalDegrees(0,0,0));
  myAstro.setGMTdate(1984,1,7);
  myAstro.doMoonRiseSetTimes();
  myAstro.printDegMinSecs(myAstro.getMoonriseTime());
  myAstro.printDegMinSecs(myAstro.getMoonsetTime());
  Serial.println("\n");
  
  Serial.println("Moon Rise/Set: no DST, Time Zone = 0,  Date: Jan 11,1984");
  Serial.println("Geo Long: 0:0:0.0, Lat: 30:0:0.0");
  Serial.println("local Rise=11:54:54.02 local Set=xx:xx:xx.xx");
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(30,0,0), myAstro.decimalDegrees(0,0,0));
  myAstro.setGMTdate(1984,1,11);
  myAstro.doMoonRiseSetTimes();
  myAstro.printDegMinSecs(myAstro.getMoonriseTime());
  myAstro.printDegMinSecs(myAstro.getMoonsetTime());
  Serial.println("\n");
  
  Serial.println("Moon Rise/Set: no DST, Time Zone = -5,  date: Feb 1,1984");
  Serial.println("Geo Long: -77:0:0.0, Lat: 38:55:0.0");
  Serial.println("local Rise=07:23:25.21 local Set=17:09:13.51");
  myAstro.rejectDST();
  myAstro.setTimeZone(-5);
  myAstro.setLatLong(myAstro.decimalDegrees(38,55,0), myAstro.decimalDegrees(-77,0,0));
  myAstro.setGMTdate(1984,2,1);
  myAstro.doMoonRiseSetTimes();
  myAstro.printDegMinSecs(myAstro.getMoonriseTime());
  myAstro.printDegMinSecs(myAstro.getMoonsetTime());
  Serial.println("\n");
}

void loop() {
	delay(10000);
}
