# hackair_kniwwelino
[Kniwwelino](http://www.kniwwelino.lu/) sketch to display results from a [hackair home sensor](http://www.hackair.eu/hackair-home-v2/).

Lastest value from the hackair database (search for a nearby hackair home sensor on https://platform.hackair.eu/ or operate one yourself, you can order an assembled one currently at https://www.hackair.eu/product/hackair-home-sensor/) is fetched every 10 minutes. Index is used to set RGB LED (red, yellow, orange, red), click of button A shows PM2.5 value on matrix, click of button B PM10 value, cf. http://www.hackair.eu/about-hackair/about-air-quality/ for explanation of PM10 and PM25. When no values could be fetched (eg no wifi, no ntp time yet, no internet connection, invalid sensorid) or the last fetched reading is too old, RGB LED is switched off.

You can change the sensorid used by pressing button B and keep it pressed and clicking on button A. Relese button B then. You have then entered setup mode and "F" ist displayed. When you immediatly click button B now, the currently used sensorid is shown once and setup mode is left.
Via clicking button A you can go from 0 to 9 and F. When clicking button B either the current digit is added to the new sensorid and setup continues with the next digit or when "F" is shown the so far entered digits are used as the new sensorid which is shown once, saved, used and setup mode is left.

Caution: Work in progress

For installation follow instructions from https://github.com/LIST-LUXEMBOURG/KniwwelinoLib (esp. don't use the 2.3.2 board package from the board manager url http://doku.kniwwelino.lu/_media/en/download/package_esp8266_kniwwelino_index.json as instructed on https://doku.kniwwelino.lu/en/installationarduino since 2.3.2 is too old for SSL CA validation as used here; but the other hints on the latter page for using ArduinoIDE are valid and helpful). 
