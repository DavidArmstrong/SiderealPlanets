/* Sidereal Planets Library - Time functions
 * Version 1.2.0 - February 17, 2023
 * Example2_Time
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
  Serial.println("Sidereal Planets Time Functions");
  myAstro.begin();
  Serial.println("\nIt is best to start by setting the Latitude and Longitude.");
  Serial.println("We can use the decimalDegrees() function to make this easier.");
  myAstro.setLatLong(51.178889, myAstro.decimalDegrees(-148,31,52.33));
  
  Serial.println("Next is setting the Time Zone offset from GMT.");
  myAstro.setTimeZone(-10);
  
  Serial.println("And setting whether Daylight Savings time is used or not.");
  myAstro.setDST();
  
  Serial.println("Then comes setting the GMT date.");
  Serial.println("The library puposely does not input the local date,");
  Serial.println("which may be different.");
  myAstro.setGMTdate(1990,3,11);
  
  Serial.println("Finally, we can set either the local time, or GMT.");
  myAstro.setLocalTime(8,21,43.7);
  
  Serial.println("The settings chosen here are:");
  Serial.println("DST on, time zone = -10, Long = -148d 31 min 52.33 sec");
  Serial.println("GMT date: Mar 11,1990  local time: 8:21:43.7");
  Serial.println("\nWith those settings in place, we now compute the results:");
  Serial.print("GMT=17:21:43.7  ==> ");
  myAstro.printDegMinSecs(myAstro.getGMT());
  Serial.println("");
  Serial.print("Greenwich Sidereal Time=4:38:9.22  ==> ");
  myAstro.printDegMinSecs(myAstro.getGMTsiderealTime());
  Serial.println("");
  Serial.print("Local Siderial Time=18:44:1.73  ==> ");
  myAstro.printDegMinSecs(myAstro.getLocalSiderealTime());
  Serial.println("");
  Serial.print("\nAnd we can take LST back to local time: ");
  myAstro.printDegMinSecs(myAstro.doLST2LT(myAstro.getLocalSiderealTime()));
  Serial.println("");
  Serial.print("\nAnd LST back to GMT: ");
  myAstro.printDegMinSecs(myAstro.doLST2GMT(myAstro.getLocalSiderealTime()));
  Serial.println("");
}

void loop() {
  while(1); //Freeze
}