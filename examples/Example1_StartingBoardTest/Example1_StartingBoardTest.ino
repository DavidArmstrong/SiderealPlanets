/* Sidereal Planets Library Board Test
 * Version 1.2.0 - February 17, 2023
 * Example1_StartingBoardTest
*/

// Need the following define for SAMD processors
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

void setup() {
  Serial.begin(9600);
  delay(2000); //SAMD boards may need a long time to init SerialUSB
  Serial.println("Sidereal Planets Board Test");
  Serial.println("Check the precision of the Sidereal Planets Library on this processor.\n");

  // Real Number check
  float  fpi = 3.14159265358979;
  double dpi = 3.14159265358979;
  if (dpi == (double)fpi ) { //double and float are the same here!
    Serial.println("Target board does not handle Real Double numbers.");
	Serial.println("There will be some loss of precision with this board using the library.");
  } else {
	Serial.println("Target board will handle real double numbers.");
	Serial.println("Go ahead and use this platform for sketches using the Sidereal Planets Library.");
  }
}

void loop() {
  while(1); //Freeze
}
