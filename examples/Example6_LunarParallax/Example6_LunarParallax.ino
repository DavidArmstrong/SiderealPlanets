/* Sidereal Planets Library - Lunar Parallax functions
 * Version 1.2.0 - February 17, 2023
 * Example6_LunarParallax
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
  Serial.println("Sidereal Planets Lunar Parallax Functions");
  myAstro.begin();
  Serial.println("\nBecause these calculations are dependent on both time and location,");
  Serial.println("those values have to be set first.");
  Serial.println("We can use the decimalDegrees() function to make this easier.");
  Serial.println("We define:");
  Serial.println("Lat = 50:00:00, Long = -100:00:0.0");
  Serial.println("DST Off, Time Zone = -6, Elevation = 60m");
  Serial.println("GMT date: 2/26/1979,  Local Time: 10:45:00");
  myAstro.setLatLong(myAstro.decimalDegrees(50,0,0), myAstro.decimalDegrees(-100,0,0.0));
  myAstro.setElevationM(60.0);
  myAstro.setTimeZone(-6);
  myAstro.rejectDST();
  myAstro.setGMTdate(1979,2,26);
  myAstro.setLocalTime(10,45,00.0);
  
  Serial.println("\nWe define the Moon's position at this date and time as follows:");
  Serial.println("Input RA = 22 hr 35 min 19. sec, Dec = -7d 41 min 13. sec");
  myAstro.setRAdec(myAstro.decimalDegrees(22,35,19.), myAstro.decimalDegrees(-7,41,13.));
  Serial.println("The doMoon() routine will also compute the Equatorial horizontal parallax");
  Serial.println("internally for us, so we don't have to input it.  (But we do it here for");
  Serial.println("simplicity.)");
  myAstro.setEquatHorizontalParallax(myAstro.decimalDegrees(1,1,9)); //Function only for testing
  Serial.print("Equatorial horizontal parallax = 1:1:9  ==> ");
  myAstro.printDegMinSecs(myAstro.getEquatHorizontalParallax());
  Serial.println();
  Serial.println("\nNow adjust the Moon's position for Parallax:");
  myAstro.doLunarParallax();
  Serial.print("Right Ascension = 22:36:43.22  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  Serial.println("");
  Serial.print("Declination = -8:32:17.4  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println();
}

void loop() {
  while(1); //Freeze
}