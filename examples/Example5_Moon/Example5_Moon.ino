/* Sidereal Planets Library - Moon functions
 * Version 1.5.0 - January 4, 2025
 * Example5_Moon
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
  Serial.println("Sidereal Planets Moon Functions");
  myAstro.begin();
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
  Serial.println("");
  Serial.print("Fraction Illuminated = 37 percent  ==> ");
  Serial.print(myAstro.getLunarLuminance());
  Serial.println("\n");
  Serial.print("Moon Phase = Crescent  ==> ");
  switch (myAstro.getMoonPhase()) {
    case 0: Serial.println("New"); break;
    case 1: Serial.println("Waxing Crescent"); break;
    case 2: Serial.println("First Quarter"); break;
    case 3: Serial.println("Waxing Gibbous"); break;
    case 4: Serial.println("Full"); break;
    case 5: Serial.println("Waning Gibbous"); break;
    case 6: Serial.println("Third Quarter"); break;
    case 7: Serial.println("Waning Crescent"); break;
  }
  Serial.println("\n");
  
  Serial.println("We can also calculate Moon Rise/Set times.");
  Serial.println("Example #1:");
  Serial.println("We set: DST off, Time Zone = 0, Date: Jan 7, 1984");
  Serial.println("Longitude = 0:0:0; Latitude = 30:0:0");
  
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(30,0,0), myAstro.decimalDegrees(0,0,0));
  myAstro.setGMTdate(1984,1,7);
  myAstro.doMoonRiseSetTimes();
  Serial.println("\nOur results should be or are:");
  Serial.print("Local MoonRise=9:58:1.46  ==> ");
  if ( myAstro.getMoonRiseValidFlag() ) {
    myAstro.printDegMinSecs(myAstro.getMoonriseTime());
  } else {
    Serial.print("No valid Moon Rise time.");
  }
  Serial.println("");
  Serial.print("Local MoonSet=21:8:59.44  ==> ");
  if ( myAstro.getMoonSetValidFlag() ) {
    myAstro.printDegMinSecs(myAstro.getMoonsetTime());
  } else {
    Serial.print("No valid Moon Set time.");
  }
  Serial.println("\n");

  Serial.println("Example #2:");
  Serial.println("We set: DST off, Time Zone = 0, Date: Jan 11, 1984");
  Serial.println("Longitude = 0:0:0; Latitude = 35:30:0");
  
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(35,30,0), myAstro.decimalDegrees(0,0,0));
  myAstro.setGMTdate(1984,1,11);
  myAstro.doMoonRiseSetTimes();
  Serial.println("\nOur results should be or are:");
  Serial.print("Local MoonRise=11:52:1.46  ==> ");
  if ( myAstro.getMoonRiseValidFlag() ) {
    myAstro.printDegMinSecs(myAstro.getMoonriseTime());
  } else {
    Serial.print("No valid Moon Rise time.");
  }
  Serial.println("");
  Serial.print("Local MoonSet=XX:XX:XX:  ==> ");
  if ( myAstro.getMoonSetValidFlag() ) {
    myAstro.printDegMinSecs(myAstro.getMoonsetTime());
  } else {
    Serial.print("No valid Moon Set time.");
  }
  Serial.println("\n");

  Serial.println("Example #3:");
  Serial.println("We set: DST off, Time Zone = 0, Date: 2 Feb, 2024");
  Serial.println("Longitude = -0:30:0; Latitude = 35:30:0");
  
  myAstro.rejectDST();
  myAstro.setTimeZone(0);
  myAstro.setLatLong(myAstro.decimalDegrees(35,30,0), -myAstro.decimalDegrees(0,30,0));// (-) before is for negative coord
  myAstro.setGMTdate(2024,2,2);
  myAstro.doMoonRiseSetTimes();
  Serial.println("\nOur results should be or are:");
  Serial.print("Local MoonRise=XX:XX:XX  ==> ");
  if ( myAstro.getMoonRiseValidFlag() ) {
    myAstro.printDegMinSecs(myAstro.getMoonriseTime());
  } else {
    Serial.print("No valid Moon Rise time.");
  }
  Serial.println("");
  Serial.print("Local MoonSet=10:44:10:  ==> ");
  if ( myAstro.getMoonSetValidFlag() ) {
    myAstro.printDegMinSecs(myAstro.getMoonsetTime());
  } else {
    Serial.print("No valid Moon Set time.");
  }
  Serial.println("\n");
}

void loop() {
  while(1); //Freeze
}
