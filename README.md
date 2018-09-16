# hackair_kniwwelino
[Kniwwelino](http://www.kniwwelino.lu/) sketch to display results from a [hackair home sensor](http://www.hackair.eu/hackair-home-v2/).

Lastest value from the hackair database (search for a nearby hackair home sensor on https://platform.hackair.eu/ or operate one yourself, you can order an assembled one currently at https://www.hackair.eu/product/hackair-home-sensor/) is fetched every 10 minutes. Index is used to set RGB LED (red, yellow, orange, red), click of button A shows PM2.5 value on matrix, click of button B PM10 value, cf. http://www.hackair.eu/about-hackair/about-air-quality/ for explanation of PM10 and PM25. When now values could be fetched or the last fetched reading is too old, RGB LED is switched off.

You can change the sensorid used by pressing button B and keep it pressed and clicking on button A. Relese button B then. You have then entered setup mode and "F" ist displayed. When you immediatly click button B now, the currently used sensorid is shown once and setup mode is left.
Via clicking button A you can go from 0 to 9 and F. When clicking button B either the current digit is added to the new sensorid and setup continues with the next digit or when "F" is shown the sofar entered digits are the new sensorid which is shown once, saved and used and setup mode is left.

Caution: Work in progress
