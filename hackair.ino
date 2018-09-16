#include <Kniwwelino.h>
#include <KniwwelinoIcons.h>
#include <KniwwelinoTones.h>
#include <WiFiClientSecure.h>
#include <Timezone.h>
#include <Time.h>

void setup() {
  //Initialize the Kniwwelino Board
  Kniwwelino.begin("Hackair", true, true, false); // Wifi=true, Fastboot=true, MQTT logging false
  Kniwwelino.MATRIXsetScrollSpeed(3);
  Serial.println("RED");
  Kniwwelino.RGBsetColor(255, 0, 0);
  delay(300);
  Serial.println("GREEN");
  Kniwwelino.RGBsetColor(0, 255, 0);
  delay(300);
  Serial.println("BLUE");
  Kniwwelino.RGBsetColor(0, 0, 255);
  delay(300);
  Kniwwelino.RGBclear();
  if (!Kniwwelino.isConnected()) {
    Kniwwelino.RGBsetColorEffect("FF0000", RGB_FLASH , -1);
  }    
}

// unfortunatly timeZone from Kniwwelino not public, cf. https://github.com/LIST-LUXEMBOURG/KniwwelinoLib/issues/3
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone timeZone = Timezone(CEST, CET);

String dummy; // get's overwritten with garbage, why?
String valuePM25;
String valuePM10;
// possible index values: "very good" "good" "medium"? "bad"?
String indexPM25;
String indexPM10;
time_t measTime = 0;
uint16_t sensorid = 726;

time_t readtime(String line) {
  tmElements_t tm;
  const char nameTime[] = "sensors_arduino";
  int pos = line.indexOf(nameTime) + sizeof(nameTime);
  String timeStr = line.substring(pos+1, line.indexOf(",", pos)-1);
  // 2.3.2 board support has no sscanf
  //unsigned int yearTmp;
  //sscanf(timeStr.c_str(), "%u4-%hhu2-%hhu2 %hhu2:%hhu2:%hhu2", &yearTmp, &tm.Month, &tm.Day, &tm.Hour, &tm.Minute, &tm.Second);              
  //tm.Year = yearTmp - 1970;
  tm.Year = timeStr.substring(0,4).toInt() - 1970;
  tm.Month = timeStr.substring(5,7).toInt();
  tm.Day = timeStr.substring(8,10).toInt();
  tm.Hour = timeStr.substring(11,13).toInt();
  tm.Minute = timeStr.substring(14,16).toInt();
  tm.Second = timeStr.substring(17,19).toInt();              
  return timeZone.toLocal(makeTime(tm));  
}

void fetchReadings(uint16_t sensorid) {
  if ( now()-measTime > 10 * 60) { // only fetch values every 10 min
    if (Kniwwelino.isConnected()) {
      WiFiClientSecure client;
      const char* host = "api.hackair.eu";
      const char* fingerprint = "E9 E4 38 EE CB C8 5B FF CB EA E1 75 B3 67 11 9F 31 28 A7 1C";
      if (client.connect(host, 443)) {

        // could and should better check CA as in https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/examples/WiFiClientSecure/WiFiClientSecure.ino        
        if (client.verify(fingerprint, host)) {
          char request[100];
          time_t t = timeZone.toUTC(now());        
          t = t - (15 * 60); // 15min back to get some measurements
          sprintf(request, "/measurements/export?sensor_id=%d&start=%04u-%02u-%02uT%02u%%3A%02u%%3A%02u.000Z", sensorid, year(t), month(t), day(t), hour(t), minute(t), second(t));
          Serial.println(request);
    
          client.print(String("GET ") + request + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "User-Agent: KniwwelinoHackair\r\n" +
             "Connection: close\r\n\r\n");      
    
          int statusCode = 0;
          String reasonPhrase;
          int bodyLen = 0;
          while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
              break;
            } else if (line.startsWith("HTTP/1.1 ")) {
              int pos = line.indexOf(" ", 9);
              statusCode = line.substring(9, pos).toInt();
              reasonPhrase = line.substring(pos+1, line.length()-1);
            } else if (line.startsWith("Content-Length: ")) {
              bodyLen = line.substring(16, line.length()-1).toInt();
            }
          }
          if (statusCode == 200) {
            if (bodyLen > 150 && bodyLen < 8000) {
              client.readStringUntil('\n'); // skip header line
              String line = client.readStringUntil('\n');
              Serial.println(line);

              int pos;
              
              time_t t25 = readtime(line);
              
              const char namePM25V[] = "PM2.5_AirPollutantValue";
              pos = line.indexOf(namePM25V) + sizeof(namePM25V);
              valuePM25 = line.substring(pos, line.indexOf(",", pos));
              
              const char namePM25I[] = "PM2.5_AirPollutantIndex";
              pos = line.indexOf(namePM25I) + sizeof(namePM25I);
              indexPM25 = line.substring(pos, line.indexOf(",", pos));
              // index does not always have quotes, exemplary result:
              //726,sensors_arduino,"2018-09-15 07:17:00",PM2.5_AirPollutantValue,17.96,"micrograms/cubic meter",PM2.5_AirPollutantIndex,good,"48.186890411259,11.365576386452"
              //726,sensors_arduino,"2018-09-15 07:17:00",PM10_AirPollutantValue,18.83,"micrograms/cubic meter",PM10_AirPollutantIndex,"very good","48.186890411259,11.365576386452"
              if (indexPM25.startsWith("\""))
                indexPM25 = indexPM25.substring(1);
              if (indexPM25.endsWith("\""))
                indexPM25 = indexPM25.substring(0, indexPM25.length()-1);

              line = client.readStringUntil('\n');
              Serial.println(line);

              time_t t10 = readtime(line);
              
              const char namePM10V[] = "PM10_AirPollutantValue";
              pos = line.indexOf(namePM10V) + sizeof(namePM10V);
              valuePM10 = line.substring(pos, line.indexOf(",", pos));

              const char namePM10I[] = "PM10_AirPollutantIndex";
              pos = line.indexOf(namePM10I) + sizeof(namePM10I);
              indexPM10 = line.substring(pos, line.indexOf(",", pos));
              if (indexPM10.startsWith("\""))
                indexPM10 = indexPM10.substring(1);
              if (indexPM10.endsWith("\""))
                indexPM10 = indexPM10.substring(0, indexPM10.length()-1);

              measTime = _min(t25, t10);

              Serial.println("success");
              Serial.println(valuePM25);
              Serial.println(indexPM25);
              Serial.println(valuePM10);              
              Serial.println(indexPM10);
              Serial.println(now()-measTime);              
            } else {
              Serial.println("unusual body length:");
              Serial.println(bodyLen);
            }
          } else {
            Serial.println(statusCode);
            Serial.println(reasonPhrase);
          }
        } else {
          Serial.println("certificate doesn't match");
        }                  
        client.stop();
      } else {
        Serial.println("connection failed");
      }      
    }
    if ( now()-measTime > 60 * 60) { // only show values no older than one hour
      Serial.println("invalidating values because too old");
      measTime = 0;
    }
  }  
}

time_t lastTry = 0;
boolean setupMode = false;
String newID;
byte currentDigit;

void loop() {
  if (now()-lastTry > 30) {
    fetchReadings(sensorid);
    lastTry = now();
    if (measTime > 0) {
      Serial.println(indexPM25);
      Serial.println(indexPM10);
      Kniwwelino.RGBsetBrightness(50);
      if (indexPM25.equals("bad") || indexPM10.equals("bad"))
        Kniwwelino.RGBsetColor(255,0,0);
      else if (indexPM25.equals("medium") || indexPM10.equals("medium"))
        Kniwwelino.RGBsetColor(255,200,0);
      else if (indexPM25.equals("good") || indexPM10.equals("good"))
        Kniwwelino.RGBsetColor(255,255,0);
      else
        Kniwwelino.RGBsetColor(0,255,0);      
    } else
      Kniwwelino.RGBclear();
  }
  if (Kniwwelino.BUTTONAclicked()) {    
    Serial.println("A Clicked");
    if (Kniwwelino.BUTTONBdown()) {
      Serial.println("B Button down");
      setupMode = !setupMode;
      if (setupMode) {
        currentDigit = 11;
        newID = "";
      }
    }
    if (!setupMode) {
      Serial.println(valuePM25);
      Kniwwelino.MATRIXwriteOnce(valuePM25);
    } else {
      if (currentDigit == 10)
        currentDigit = 0;
      else
        currentDigit++;
      if (currentDigit > 9)
        Kniwwelino.MATRIXwrite("F");
      else
        Kniwwelino.MATRIXwrite(String(currentDigit));
    }
  }  
  if (Kniwwelino.BUTTONBclicked()) {    
    Serial.println("B Clicked");
    if (!setupMode) {
      Serial.println(valuePM10);
      Kniwwelino.MATRIXwriteOnce(valuePM10);
    } else {
      if (currentDigit > 11)
        currentDigit = 10;
      else
        if (currentDigit == 10) {
          if (newID.length() > 0) {
            sensorid = newID.toInt();
            Serial.println(sensorid);
            Kniwwelino.MATRIXwriteOnce(String(sensorid));
            lastTry = 0;
            measTime = 0;
          }
          setupMode = false;
        } else {
          newID += String(currentDigit);
          currentDigit = 10;
          Kniwwelino.MATRIXwrite("F");
          Serial.println(newID);
        }       
    }
  }
  
  Kniwwelino.loop();
}
