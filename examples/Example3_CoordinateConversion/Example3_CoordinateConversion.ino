/* Sidereal Planets Library - Coordinate Conversion functions
 * Version 1.2.0 - February 17, 2023
 * Example3_CoordinateConversion
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
  Serial.println("Sidereal Planets Coordinate Conversion Functions");
  myAstro.begin();
  Serial.println("\nBecause these conversions are dependent on both time and location,");
  Serial.println("those values have to be set first.");
  Serial.println("We can use the decimalDegrees() function to make this easier.");
  Serial.println("We define:");
  Serial.println("DST off, time zone = -5");
  Serial.println("Lat = 38d 55 min 0 sec, Long = -77d 0 min 0 sec");
  Serial.println("GMT date: Feb 1,1984  Local Time: 7:23:00");
  myAstro.rejectDST();
  myAstro.setTimeZone(-5);
  myAstro.setLatLong(myAstro.decimalDegrees(38,55,0), myAstro.decimalDegrees(-77,0,0));
  myAstro.setGMTdate(1984,2,1);
  myAstro.setLocalTime(7,23,0);
  Serial.print("Latitude: ");
  myAstro.printDegMinSecs(myAstro.getLatitude());
  Serial.println("");
  Serial.print("Longitude: ");
  myAstro.printDegMinSecs(myAstro.getLongitude());
  Serial.println("");
  
  Serial.println("\nFrom that, we calculate:");
  Serial.print("GMT=12:23:00.0  ==> ");
  myAstro.printDegMinSecs(myAstro.getGMT());
  Serial.println("");
  Serial.print("Greenwich Sidereal Time=21:06:37.98  ==> ");
  myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  Serial.println("");
  Serial.print("Local Siderial Time=15:58:37.98  ==> ");
  myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  Serial.println("\n");
  
  Serial.println("Now convert RA/Declination to Azimuth/Altitude coordinates");
  Serial.println("RA = 20 hr 40 min 5.2 sec, Dec = -22d 12 min 0.0 sec");
  
  myAstro.setRAdec(myAstro.decimalDegrees(20,40,5.2), myAstro.decimalDegrees(-22,12,0));
  myAstro.doRAdec2AltAz();
  
  Serial.println("And the Results are:");
  Serial.print("Azimuth=119:18:14.59  ==> ");
  myAstro.printDegMinSecs(myAstro.getAzimuth());
  Serial.println("");
  Serial.print("Altitude=0:16:15.88  ==> ");
  myAstro.printDegMinSecs(myAstro.getAltitude());
  Serial.println("\n");
  
  Serial.println("And we can take the Azimuth/Altitude, and compute RA/Dec:");
  myAstro.setAltAz(myAstro.decimalDegrees(0,16,15.88), myAstro.decimalDegrees(119,18,14.59));
  myAstro.doAltAz2RAdec();
  Serial.print("Right Ascension  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  Serial.println("");
  Serial.print("Declination  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println();
}

void loop() {
  while(1); //Freeze
}