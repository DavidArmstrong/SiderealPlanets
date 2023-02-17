/* Sidereal Planets Library - Sun functions
 * Version 1.2.0 - February 17, 2023
 * Example7_Planets
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
  Serial.println("Sidereal Planets Functions");
  myAstro.begin();
  Serial.println("\nBecause these calculations are dependent on both time and location,");
  Serial.println("those values have to be set first.");
  Serial.println("We can use the decimalDegrees() function to make this easier.");
  Serial.println("We define:");
  Serial.println("DST off, time zone = -5");
  Serial.println("GMT date: May 30, 1984  GMT Time: 00:00:00");
  myAstro.setTimeZone(-5);
  myAstro.rejectDST();
  myAstro.setGMTdate(1984,5,30);
  myAstro.setGMTtime(0,0,0);
  
  Serial.println("Computing Mercury Parameters");
  myAstro.doMercury();
  Serial.print("Helio Ecliptic Long: -34:46:20.12  ==> ");
  myAstro.printDegMinSecs(myAstro.getHelioLong()); Serial.println("");
  Serial.print("Helio Ecliptic Lat: -6:57:5.34  ==> ");
  myAstro.printDegMinSecs(myAstro.getHelioLat()); Serial.println("");
  Serial.print("Radius vector: 0.401741  ==> ");
  Serial.println(myAstro.getRadiusVec(), 9);
  Serial.print("Distance from Earth: 0.999923  ==> ");
  Serial.println(myAstro.getDistance(), 9);
  Serial.print("Geo Ecliptic Long: 45:55:55.89  ==> ");
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude()); Serial.println("");
  Serial.print("Geo Ecliptic Lat: -2:47:16.47  ==> ");
  myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  Serial.print("RA: 2:57:8.28  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println("");
  Serial.print("Dec: 13:56:8.81  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  
  Serial.println("Computing Saturn Parameters");
  myAstro.setGMTdate(1984,5,30);
  myAstro.setGMTtime(0,0,0);
  myAstro.doSaturn();
  Serial.print("Helio Ecliptic Long: 223:55:33.24  ==> ");
  myAstro.printDegMinSecs(myAstro.getHelioLong()); Serial.println("");
  Serial.print("Helio Ecliptic Lat: 2:19:49.17  ==> ");
  myAstro.printDegMinSecs(myAstro.getHelioLat()); Serial.println("");
  Serial.print("Radius vector: 9.866205  ==> ");
  Serial.println(myAstro.getRadiusVec(), 9);
  Serial.print("Distance from Earth: 8.957211  ==> ");
  Serial.println(myAstro.getDistance(), 9);
  Serial.print("Geo Ecliptic Long: 221:11:41.66  ==> ");
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude()); Serial.println("");
  Serial.print("Geo Ecliptic Lat: 2:34:1.15  ==> ");
  myAstro.printDegMinSecs(myAstro.getEclipticLatitude()); Serial.println("");
  Serial.print("RA: 14:38:19.72  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println("");
  Serial.print("Dec: -12:44:52.50  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Computing Uranus Parameters");
  Serial.println("GMT date: May 23, 1992  GMT Time: 00:00:00");
  myAstro.setGMTdate(1992,10,23);
  myAstro.setGMTtime(0,0,0);
  myAstro.doUranus();
  Serial.print("Geo Ecliptic Long: 284:25:28.66  ==> ");
  myAstro.printDegMinSecs(myAstro.getEclipticLongitude());
  Serial.println("");
  Serial.print("RA: 19:02:51.09  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec());
  Serial.println("");
  Serial.print("Dec: -23:04:30.68  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
}

void loop() {
  while(1); //Freeze
}