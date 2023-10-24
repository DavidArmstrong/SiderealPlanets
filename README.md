# SiderealPlanets


Sidereal Planets - A strangely named Arduino Library for various astronomy related functions

  Version 1.4.0 - October 24, 2023

  By David Armstrong
  https://github.com/DavidArmstrong/SiderealPlanets
  See MIT LICENSE.md file


This library provides a core set of astronomy related functions, such as determination of local sidereal time, coordinate conversion, precession, refraction, and coordinate computation for the Sun, Moon, and planets.  It is based on the algoritms published in 'Astronomy With Your Personal Computer', Second Edition, by Peter Duffet-Smith. New York: Cambridge University Press, 1990.

It is recommended that the user run all the example sketches in turn in on the target Arduino board, as they show how to use the library functions together to achieve some desired result.  The RegressionTests sketch was used in development of the library to validate each function as it was added, and is included for reference.  Finally, the DongAndPonyShow sketch is made to be used with TeraTerm and a uBlox based GPS unit.  It demonstrates how all these routines can be used together in a sample application.

Notes:
  1) The library recommends support for double float numbers.  There are currently a number of boards that support true double type numbers, and use the Arduino IDE.  (i.e. The Sparkfun Redboard Turbo was used as the test platform in development.)  However the original UNO board, and similar derivitives, handle double numbers by mapping them to regular float types, and so these boards will loose some calculation accuracy.  Example1 will check to see if that is the case.
  2) The library is very large, about 60K in size.  So it will not fit in the small memory space of an Arduino Uno, for example.  However there are many boards available that do provide sufficient Flash space for this library to run.  Please check the memory constraints of your processor before attempting to load the library onto the system.
  3) While there may be better ways to implement some of the functionality of this library, the design makes it easier to verify the outputs, and easier to use.
  4) A GPS unit is not reqired to use this library.  However using GPS hardware in your project can help automate input of local latitude, longitude, date, and Universal Time. (See the example sketch DogAndPonyShow.ino for a demonstration on doing this.)
  5) The library, by itself, will not control a telescope or act as digital setting circles.  Those use cases can benefit from using this library, but because each hardware setup is different it is not feasible nor practical to extend this library to that level of complexity.
  6) The library name was chosen to be different from any other astronomy type library that may be out there.  Library name uniqueness is important in Arduino sketches.  (That means that there isn't a function called Sidereal Planets here!)

======================================

Basic Library Functions:<br>
-- If a function returns a boolean value of false, it generally means something was wrong with the input given to the function.  Specifics are explained with each function.

boolean begin()<br>
  This initializes the library.  As of version 1.1.0, it will always return a true value.  (This may change in a future version, if needed.)


double decimalDegrees(int degrees, int minutes, float seconds)<br>
  Convert a number that is specified in degrees, minutes, and seconds to decimal degrees.  Note that this function will also handle hours as well as degrees.

void printDegMinSecs(double n)<br>
  Prints a double number of degrees to Serial in the form {deg}:{min}:{seconds}. The seconds may include a fractional part of two digits. If needed, a minus sign is printed in front of the number.  This will also work to print hours.  The numbers printed are not further formatted in any way.

boolean setTimeZone(int zone)<br>
boolean setTimeZone(float zone)<br>
  Sets the offset, in hours, of the current local time zone from GMT.  The input can be either an integer or a float, as not all time zones on the planet are an integral number of hours from GMT.  For the United States, common values are:

  * AST = -4
  * EST = -5
  * CST = -6
  * MST = -7
  * PST = -8
  * AKST = -9
  * HST = -10


boolean useAutoDST()<br>
  This activates an internal algorithm that determines when Daylight Savings Time (DST) is in effect in the United States.  Do not call this if you require manual control of DST.  (i.e. You live in Arizona, or are outside the United States.)  Instead, use setDST() or rejectDST() as needed.  The Time Zone must be set first with setTimeZone(), as setting the Local time will not result in a correct result. Next, both the GMT date and GMT time functions must be called before this function is called to get a correct value returned. The function will return a true flag if Daylight Savings Time is considered to be in effect for the current GMT date set with setGMTdate(), and current GMT time set with setGMTtime(). It is written this way because GPS units provide date and time in GMT, not local time.

void setDST()<br>
  Indicates that Daylight Savings Time (+1 hour) is in effect for all associated calculations.  This overrides the useAutoDST() function.

void rejectDST()<br>
  Indicates that Daylight Savings Time adjustments are not to be made in any calculation.  This overrides the useAutoDST() function.  Note: This setting is the default for the library.

boolean setLatLong(double latitude, double longitude)<br>
  Updates the Earth's latitude and longitude that is used in calculations.  The values for both parameters are in degrees.  Latitude can be from -90. to +90.  Longitude can range from -180. to +180., with West of Greenwich, England being negative values.

boolean setGMTdate(int year, int month, int day)<br>
  Updates the date used in calculations.  This must be the date for the Greenwich Meridian Time zone.  (Note: GPS sensors provide GMT date and time, not local time.  So if the date is being obtained from GPS, that data can be used directly to call this function.)  The date required here may be different from the date of your local time zone.  Validity of values for year, month, and day are checked.  The year cannot be a '0' value, as there was no year 0.

boolean setGMTtime(int hours, int minutes, int seconds)<br>
  Sets the GMT time used in calculations.  If this is called, do not call setLocalTime().  The Time Zone and Daylight Savings Time must be set before calling this function, unless using AutoDST.  (Note: GPS sensors provide GMT date and time, not local time.  So if the time is being obtained from GPS, that data can be used directly to call this function.)

boolean setLocalTime(int hours, int minutes, int seconds)<br>
  Sets the local time used in calculations.  If this is called, do not call setGMTtime(), and do not use the function useAutoDST().  The GMT date, Time Zone and Daylight Savings Time must be set before calling this function, so that GMT time can be calculated from the local time provided.

double getLatitude()<br>
  Returns Earth's latitude that is used in calculations.  The value is in degrees.  Latitude can range from -90. to +90.

double getLongitude()<br>
  Returns Earth's longitude that is used in calculations.  The value is in degrees.  Longitude can range from -180. to +180., with West of Greenwich, England being negative values.

double getGMT()<br>
  Returns GMT as a double number of hours.  Either setGMTtime() or setLocalTime() has to be called first.

double getLT()<br>
  Returns the local time as a double number of hours.  Either setGMTtime() or setLocalTime() has to be called first.

double getLocalSiderealTime()<br>
  Returns a double float of the Local Sidereal Time, in hours since midnight in the current local sidereal day.

double getGMTsiderealTime()<br>
  Returns a double float of the GMT sidereal time, in hours since midnight for the current GMT sidereal day.

boolean setElevationM(double height)<br>
  Sets the observer's elevation in meters above mean sea level.  This is used for parallax computations.

boolean setElevationF(double height)<br>
  Sets the observer's elevation in feet above mean sea level.  This is used for parallax computations.

boolean setRAdec(double RightAscension, double Declination)<br>
  Sets the Right Ascension and Declination for a sky position that will be used by the library.  This must be called before doPrecession() or doRAdec2AltAz(). Right Ascension is in hours, with valid values from 0 to less than 24.  Declination is in degrees, and can range from -90. to +90.

boolean setAltAz(double Altitude, double Azimuth)<br>
  Sets the Altitude and Azimuth for a sky position that will be used by the library.  The values are in decimal dgrees.  This must be called before doRefractionX(), doAntiRefractionX() or doAltAz2RAdec().
  
double getRAdec()<br>
  Returns a double number of the calculated Right Ascension in hours.

double getDeclinationDec()<br>
  Returns a double number of the calculated Declination in degrees.

double getAltitude()<br>
  Returns a double number of the calculated Altitude in degrees.

double getAzimuth()<br>
  Returns a double number of the calculated Azimuth in degrees.

boolean doRAdec2AltAz()<br>
  Takes the current Right Ascension and Declination values, and computes the Altitude and Azimuth for that postion.

boolean doAltAz2RAdec()<br>
  Takes the current Altitude and Azimuth values, and computes the Right Ascension and Declination for that postion.

boolean doPrecessFrom2000()<br>
  Does the precession calculation on the current values for Right Ascension and Declination, which are assumed to be epoch 2000.0. The coordinates are modified to the epoch for the current date and time.  Returns a true value if Precession calculations were successful.

boolean doPrecessTo2000()<br>
  Does the precession calculation on the current values for Right Ascension and Declination, which are assumed to be using the curent date and time for their epoch. The coordinates are transformed to epoch 2000.0 coordinates. Returns a true value if Precession calculations were successful.

boolean doLunarParallax()<br>
  Adjusts the computed Right Ascension and Declination of the Moon based on the observers latitude and height above sea level to account for parallax. The doMoon() function must be called before calling this function.

float getLunarLuminance()<br>
  Returns the percentage of the Moon that is illuminated by the Sun, as seen from Earth. If the doMoon() function is not called before calling this function, it will be called internally.

int getMoonPhase()<br>
  Returns an integer which can be interpreted to represent what the Phase of the Moon is for the last doMoon() call.  If the doMoon() function is not called before calling this function, it will be called internally. The return value represents the following:<br>
  0. New Moon<br>
  1. Waxing Crescent<br>
  2. First Quarter<br>
  3. Waxing Gibbous<br>
  4. Full Moon<br>
  5. Waning Gibbous<br>
  6. Third Quarter<br>
  7. Waning Crescent

boolean doRefractionF(double pressure, double temperature)<br>
  Adjusts altitude of an object for atmospheric refraction, based on the barometric pressure (inches of Mercury) and temperature (Farenheit) provided. The calculated output are the coordinates that one would find on a star chart.

boolean doRefractionC(double pressure, double temperature)<br>
  Adjusts altitude of an object for atmospheric refraction, based on the barometric pressure (millimeters of Mercury) and temperature (Celsius) provided. The calculated output are the coordinates that one would find on a star chart.

boolean doAntiRefractionF(double pressure, double temperature)<br>
  Adjusts altitude of an object with atmospheric refraction effects removed, based on the barometric pressure (inches of Mercury) and temperature (Farenheit) provided.

boolean doAntiRefractionC(double pressure, double temperature)<br>
  Adjusts altitude of an object with atmospheric refraction effects removed, based on the barometric pressure (millimeters of Mercury) and temperature (Celsius) provided.

boolean doSun()<br>
  Computes the position of the Sun.  The functions getRAdec() and getDeclinationDec() can be used afterwards to retrieve the comuputed coordinates of the Sun.

boolean doMoon()<br>
  Computes the position of the Moon.

boolean doMercury()<br>
boolean doVenus()<br>
boolean doMars()<br>
boolean doJupiter()<br>
boolean doSaturn()<br>
boolean doUranus()<br>
boolean doNeptune()<br>
  Computes the position of the associated planet.

boolean doSunRiseSetTimes()<br>
  Computes the local times for sunrise and sunset for the current date.  The results are stored internally, and must be retrieved by calls to getSunriseTime() and getSunsetTime().  If a value of false is returned, the Sun never sets or rises on the current GMT date at this location on the earth.

double getSunriseTime()<br>
  Returns the local time for sunrise for a previously specified date.  The time is in hours since midnight.

double getSunsetTime()<br>
  Returns the local time for sunset for a previously specified date.  The time is in hours since midnight.

boolean doMoonRiseSetTimes()<br>
  Computes the local times for moonrise and moonset for the current date.  The results are stored internally, and must be retrieved by calls to getMoonriseTime() and getMoonsetTime().  If a value of false is returned, the Moon never sets or rises on the current GMT date at this location on the earth.

double getMoonriseTime()<br>
  Returns the local time for moonrise for a previously specified date.  The time is in hours since midnight.

double getMoonsetTime()<br>
  Returns the local time for moonset for a previously specified date.  The time is in hours since midnight.

boolean doRiseSetTimes(double displacement)<br>
  For a provided Right Ascension and Declination, and provided a vertical displacement in degrees, this will calculate the times for rising and setting of that position.  Vertical displacement is used to account for objects that have some visible size to them, like a planet.  For a star, it can be set to zero (0.0 degrees).  The results are stored internally, and must be retrieved by calls to getRiseTime() and getSetTime().  If a value of false is returned, that sky position never sets or rises at this location on the earth.

double getRiseTime()<br>
  Returns the Local Time of the rising of an object, as computed by doRiseSetTimes(), in decimal hours.

double getSetTime()<br>
  Returns the Local Time of the setting of an object, as computed by doRiseSetTimes(), in decimal hours.
