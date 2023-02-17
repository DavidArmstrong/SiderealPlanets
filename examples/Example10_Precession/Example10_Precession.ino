/* Sidereal Planets Library - Sun functions
 * Version 1.2.0 - February 17, 2023
 * Example10_Precession
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
  Serial.println("Sidereal Planets Precession Functions\n");
  myAstro.begin();
  Serial.println("Example Precession from 1/0.9/1950 to 2/4/1990");
  myAstro.setGMTdate(1950,1,1);
  Serial.println("Input RA = 12 hr 12 min 12 sec, Dec = 23d 23 min 23 sec");
  myAstro.setRAdec(myAstro.decimalDegrees(12,12,12.), myAstro.decimalDegrees(23,23,23.));
  Serial.println("First, precess coordinates to epoch 2000.0");
  myAstro.doPrecessTo2000();
  Serial.println("Then we change the date, and precess coordinates to target epoch 1990");
  myAstro.setGMTdate(1990,2,4);
  myAstro.doPrecessFrom2000();
  Serial.print("Output RA = 12 hr 14 min 13.93 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println("");
  Serial.print("Output Dec = 23d 10 min 0.64 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Example Precession from 2/4/1990 to 1/0.9/1950");
  myAstro.setGMTdate(1990,2,4);
  Serial.println("Input RA = 12 hr 14 min 13.93 sec, Dec = 23d 10 min 0.64 sec");
  myAstro.setRAdec(myAstro.decimalDegrees(12,14,13.93), myAstro.decimalDegrees(23,10,0.64));
  Serial.println("First, precess coordinates to epoch 2000.0");
  myAstro.doPrecessTo2000();
  Serial.println("Then we change the date, and precess coordinates to target epoch 1950");
  myAstro.setGMTdate(1950,1,1);
  myAstro.doPrecessFrom2000();
  Serial.print("Output RA = 12 hr 12 min 12 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println("");
  Serial.print("Output Dec = 23d 23 min 23 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Example Precession from 1/0.9/1950 to 1/1/2050");
  myAstro.setGMTdate(1950,1,1);
  Serial.println("Input RA = 12 hr 12 min 12 sec, Dec = 23d 23 min 23 sec");
  myAstro.setRAdec(myAstro.decimalDegrees(12,12,12.), myAstro.decimalDegrees(23,23,23.));
  myAstro.doPrecessTo2000();
  Serial.println("Then we change the date, and precess coordinates to target epoch 2050");
  myAstro.setGMTdate(2050,1,1);
  myAstro.doPrecessFrom2000();
  Serial.print("Output RA = 12 hr 17 min 15.84 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println("");
  Serial.print("Output Dec = 22d 50 min 2.86 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
  
  Serial.println("Example Precession from 1/1/2050 to 1/0.9/1950");
  myAstro.setGMTdate(2050,1,1);
  Serial.println("Input RA = 12 hr 17 min 15.84 sec, Dec = 22d 50 min 2.86 sec");
  myAstro.setRAdec(myAstro.decimalDegrees(12,17,15.84), myAstro.decimalDegrees(22,50,2.86));
  myAstro.doPrecessTo2000();
  Serial.println("Then we change the date, and precess coordinates to target epoch 1950");
  myAstro.setGMTdate(1950,1,1);
  myAstro.doPrecessFrom2000();
  Serial.print("Output RA = 12 hr 12 min 12 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getRAdec()); Serial.println("");
  Serial.print("Output Dec = 23d 23 min 23 sec  ==> ");
  myAstro.printDegMinSecs(myAstro.getDeclinationDec());
  Serial.println("\n");
}

void loop() {
  while(1); //Freeze
}