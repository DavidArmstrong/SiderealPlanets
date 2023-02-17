/* Sidereal Planets Library - Sun functions
 * Version 1.2.0 - February 17, 2023
 * Example8_Refraction
*/

#include <SiderealPlanets.h>

// Need the following define for SAMD processors
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

SiderealPlanets myAstro;

void setup() {
  Serial.begin(9600);
  delay(2000); //SAMD boards may need a long time to init SerialUSB
  Serial.println("Sidereal Planets Refraction Functions");
  myAstro.begin();
  Serial.println("\nBecause these calculations are dependent on both time and location,");
  Serial.println("those values have to be set first.");
  Serial.println("We can use the decimalDegrees() function to make this easier.");
  Serial.println("We define:");
  Serial.println("GMT Date: 3/23/1987, Local Time: 1:1:24, Time Zone = 0, Elevation = 60m");
  Serial.println("DST Off, Lat = 51:12:13, Long = 00:10:12.0");
  myAstro.setLatLong(myAstro.decimalDegrees(51,12,13), myAstro.decimalDegrees(0,10,12.0));
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setGMTdate(1987,3,23);
  myAstro.setLocalTime(1,1,24.0);
  myAstro.setElevationM(60.0);
  //myAstro.printDegMinSecs(myAstro.getGMT());
  //myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  //myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  //Serial.println("");
  Serial.println("Observed Input RA = 23:14:0.0, Dec = 40:10:0.0");
  myAstro.setRAdec(myAstro.decimalDegrees(23,14,0.0), myAstro.decimalDegrees(40,10,0.0));
  myAstro.doRAdec2AltAz();
  Serial.print("Uncorrected Altitude should be 4:22:2.41  ==> ");
  myAstro.printDegMinSecs(myAstro.getAltitude());
  Serial.println();
  Serial.println("Do Refraction Calculation on observed Altitude and Aximuth");
  Serial.println("coordinates with: Pressure 1012 millibars, Temperature 21.7 C");
  myAstro.doRefractionC(1012., 21.7);
  //myAstro.doRefractionF(29.88434, 71.06);
  myAstro.doAltAz2RAdec();
  Serial.println("The calculated output are the coordinates that one would find on a star chart.");
  Serial.print("RA = 23:13:44.74  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println();
  Serial.print("Dec = 40:19:45.77  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("The Anti-Refraction function takes the coordinates as seen on a star chart,");
  Serial.println("and calculates the coordinates of that position in the sky.");
  Serial.println("We define:");
  Serial.println("GMT Date: 3/23/1987, 1:1:24 LT, Time Zone = 0, Elevation 60m");
  Serial.println("DST Off, Lat = 51 deg 12 min 13 sec, Long = 00 deg 10 min 12.0 sec");
  myAstro.setLatLong(myAstro.decimalDegrees(51,12,13), myAstro.decimalDegrees(0,10,12.0));
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setGMTdate(1987,3,23);
  myAstro.setLocalTime(1,1,24.0);
  myAstro.setElevationM(60.0);
  //myAstro.printDegMinSecs(myAstro.getGMT());
  //myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  //myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  //Serial.println("");
  Serial.println("Star Chart RA = 23:13:44.74, Dec = 40:19:45.77");
  myAstro.setRAdec(myAstro.decimalDegrees(23,13,44.74), myAstro.decimalDegrees(40,19,45.77));
  myAstro.doRAdec2AltAz();
  Serial.print("Uncorrected Altitude should be 4:32:13.68  ==> ");
  myAstro.printDegMinSecs(myAstro.getAltitude());
  Serial.println();
  Serial.println("Do Anti-Refraction Calculation on observed Altitude and Aximuth");
  Serial.println("coordinates with: Pressure 1012 millibars, Temperature 21.7 C");
  myAstro.doAntiRefractionC(1012., 21.7);
  //myAstro.doAntiRefractionF(29.88434, 71.06);
  myAstro.doAltAz2RAdec();
  Serial.print("RA = 23:14:0.0  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println();
  Serial.print("Dec = 40:10:0.01  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
}

void loop() {
  while(1); //Freeze
}