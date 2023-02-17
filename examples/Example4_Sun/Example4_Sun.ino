/* Sidereal Planets Library - Sun functions
 * Version 1.2.0 - February 17, 2023
 * Example4_Sun
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
  Serial.println("Sidereal Planets Sun Functions");
  myAstro.begin();
  Serial.println("\nBecause these calculations are dependent on both time and location,");
  Serial.println("those values have to be set first.");
  Serial.println("We can use the decimalDegrees() function to make this easier.");
  Serial.println("We define:");
  Serial.println("DST off, time zone = -5");
  Serial.println("GMT date: August 24, 1984  Local Time: 19:00:00");
  myAstro.setTimeZone(-5);
  myAstro.rejectDST();
  myAstro.setGMTdate(1984,8,24);
  myAstro.setLocalTime(19,0,0.0);
  
  Serial.println("\nNow calculate the Sun's position:");
  myAstro.doSun();
  Serial.print("Right Ascension = 10:12:11.37  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  Serial.println("");
  Serial.print("Declination = 11:7:9.01  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println();
  Serial.println("\n");
  
  Serial.println("We can also calculate Sun Rise/Set times.");
  Serial.println("We set: DST off, time zone = 0, date: Oct 1, 1984");
  Serial.println("Longitude = 0:0:0; Latitude = 52:0:0");
  
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(52,0,0), myAstro.decimalDegrees(0,0,0));
  myAstro.setGMTdate(1984,10,1);
  myAstro.doSunRiseSetTimes();
  Serial.println("\nOur results should be/are:");
  Serial.print("Local SunRise=6:0:58.12  ==> ");
  myAstro.printDegMinSecs(myAstro.getSunriseTime());
  Serial.println("");
  Serial.print("Local SunSet=17:37:10.86  ==> ");
  myAstro.printDegMinSecs(myAstro.getSunsetTime());
  Serial.println("");
}

void loop() {
  while(1); //Freeze
}