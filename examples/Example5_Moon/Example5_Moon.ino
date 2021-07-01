/* Sidereal Planets Library - Moon functions
 * Version 1.0.0 - July 1, 2021
 * Example5_Moon
*/

#include <SiderealPlanets.h>

// Need the following define for SAMD processors
#if defined (ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

SiderealPlanets myAstro;

void setup() {
  Serial.begin(9600);
  delay(2000); //SAMD boards may need a long time to init SerialUSB
  Serial.println("Sidereal Planets Moon Functions");
  if (myAstro.begin() == false) {
	Serial.println("Target board does not handle Real Double numbers! Stopping...");
	while(1); //Freeze
  }
  Serial.println("\nBecause these calculations are dependent on both time and location,");
  Serial.println("those values have to be set first.");
  Serial.println("We can use the decimalDegrees() function to make this easier.");
  Serial.println("We define:");
  Serial.println("DST off, Time Zone = 0");
  Serial.println("GMT date: Feb 25, 1984,  Local Time: 00:00:00");
  myAstro.setTimeZone(0);
  myAstro.rejectDST();
  myAstro.setGMTdate(1984,2,25);
  myAstro.setLocalTime(0,0,0.0);
  
  Serial.println("\nNow calculate the Moon's position:");
  myAstro.doMoon();
  Serial.print("Right Ascension = 17:19:20.65  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  Serial.println("");
  Serial.print("Declination = -23:52:22.51  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println();
  Serial.println("\n");
  
  Serial.println("We can also calculate Moon Rise/Set times.");
  Serial.println("We set: DST off, Time Zone = 0, Date: Jan 7,1984");
  Serial.println("Longitude = 0:0:0; Latitude = 30:0:0");
  
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(30,0,0), myAstro.decimalDegrees(0,0,0));
  myAstro.setGMTdate(1984,1,7);
  myAstro.doMoonRiseSetTimes();
  Serial.println("\nOur results should be or are:");
  Serial.print("Local MoonRise=9:58:1.46  ==> ");
  myAstro.printDegMinSecs(myAstro.getMoonriseTime());
  Serial.println("");
  Serial.print("Local MoonSet=21:8:59.44  ==> ");
  myAstro.printDegMinSecs(myAstro.getMoonsetTime());
  Serial.println("");
}

void loop() {
  while(1); //Freeze
}