/* Sidereal Planets Library - Sun functions
 * Version 1.2.0 - February 17, 2023
 * Example9_AutoDST
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
  Serial.println("Sidereal Planets Automatic DST Function\n");
  myAstro.begin();
  Serial.println("useAutoDST() activates an internal algorithm that determines when Daylight");
  Serial.println("Savings Time (DST) is in effect in the United States.  Do not call this");
  Serial.println("if you require manual control of DST.  (i.e. You live in Arizona, or are");
  Serial.println("outside the United States.)  Instead, use setDST() or rejectDST() as");
  Serial.println("needed. The function will return a true flag if Daylight Savings Time is");
  Serial.println("considered to be in effect for the current GMT date set and current");
  Serial.println("GMT time set. Both GMT date and GMT time settings must be called before");
  Serial.println("this function is called to get a correct value returned.\n");
  Serial.println("Example Time Zone = -6, GMT date: September 15, 2021, GMT Time: 7:00:00");
  myAstro.setTimeZone(-6);
  myAstro.setGMTdate(2021,9,15);
  myAstro.setGMTtime(7,0,0.0);
  Serial.print("DST flag should be true(1)  ==> ");
  Serial.println(myAstro.useAutoDST()); Serial.println();
  Serial.println("Example Time Zone = -8, GMT date: February 15, 1979, GMT Time: 10:45:00");
  myAstro.setTimeZone(-8);
  myAstro.setGMTdate(1979,2,15);
  myAstro.setGMTtime(10,45,0.0);
  Serial.print("DST flag should be false(0)  ==> ");
  Serial.println(myAstro.useAutoDST()); Serial.println();
}

void loop() {
  while(1); //Freeze
}