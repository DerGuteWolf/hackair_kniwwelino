#include <Kniwwelino.h>
#include <KniwwelinoIcons.h>
#include <KniwwelinoTones.h>
#include <WiFiClientSecure.h>
#include <Timezone.h>
#include <Time.h>
#include <ArduinoJson.h>

#define FILE_HACKAIR "/hackair.json"

uint16_t sensorid = 726;

void setup() {
  //Initialize the Kniwwelino Board
  Kniwwelino.setSilent();
  Kniwwelino.begin("Hackair", true, true, false); // Wifi=true, Fastboot=true, MQTT logging false
  Kniwwelino.MATRIXsetScrollSpeed(3);
  Kniwwelino.MATRIXsetBrightness(MATRIX_DEFAULT_BRIGHTNESS);
  Kniwwelino.RGBsetBrightness(50);
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
  String confJSON = Kniwwelino.FILEread(FILE_HACKAIR);
  if (confJSON.length() > 5) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(confJSON);    
    if (json.success()) {
      if (json.containsKey("sensorid"))
        sensorid = json["sensorid"];
    }
  }  
}

// unfortunatly timeZone from Kniwwelino not public, cf. https://github.com/LIST-LUXEMBOURG/KniwwelinoLib/issues/3
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone timeZone = Timezone(CEST, CET);

String dummy; // get's overwritten with garbage, why?
String valuePM25;
String valuePM10;
// possible index values: "very good" "good" "medium"? "bad"
String indexPM25;
String indexPM10;
time_t measTime = 0;

time_t readtime(String line) {
  tmElements_t tm;
  const char nameTime[] = "sensors_arduino";
  int pos = line.indexOf(nameTime) + sizeof(nameTime);
  String timeStr = line.substring(pos+1, line.indexOf(",", pos)-1);
  // 2.3.2 board support has no sscanf, but need now 2.4.0 for setCACert_P, so could give sscanf another try
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
      unsigned char DST_Root_CA_X3_der_cer[] = { // created with xxd -i DigiCertHighAssuranceEVRootCA.crt.der >cacert.h   ; xxd win version taken from http://ge.tt/5jfutZq/v/0
        0x30, 0x82, 0x03, 0x4a, 0x30, 0x82, 0x02, 0x32, 0xa0, 0x03, 0x02, 0x01,
        0x02, 0x02, 0x10, 0x44, 0xaf, 0xb0, 0x80, 0xd6, 0xa3, 0x27, 0xba, 0x89,
        0x30, 0x39, 0x86, 0x2e, 0xf8, 0x40, 0x6b, 0x30, 0x0d, 0x06, 0x09, 0x2a,
        0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x3f,
        0x31, 0x24, 0x30, 0x22, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x1b, 0x44,
        0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x61,
        0x74, 0x75, 0x72, 0x65, 0x20, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x43,
        0x6f, 0x2e, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
        0x0e, 0x44, 0x53, 0x54, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43, 0x41,
        0x20, 0x58, 0x33, 0x30, 0x1e, 0x17, 0x0d, 0x30, 0x30, 0x30, 0x39, 0x33,
        0x30, 0x32, 0x31, 0x31, 0x32, 0x31, 0x39, 0x5a, 0x17, 0x0d, 0x32, 0x31,
        0x30, 0x39, 0x33, 0x30, 0x31, 0x34, 0x30, 0x31, 0x31, 0x35, 0x5a, 0x30,
        0x3f, 0x31, 0x24, 0x30, 0x22, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x1b,
        0x44, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x20, 0x53, 0x69, 0x67, 0x6e,
        0x61, 0x74, 0x75, 0x72, 0x65, 0x20, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20,
        0x43, 0x6f, 0x2e, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x03,
        0x13, 0x0e, 0x44, 0x53, 0x54, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43,
        0x41, 0x20, 0x58, 0x33, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09,
        0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03,
        0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01,
        0x00, 0xdf, 0xaf, 0xe9, 0x97, 0x50, 0x08, 0x83, 0x57, 0xb4, 0xcc, 0x62,
        0x65, 0xf6, 0x90, 0x82, 0xec, 0xc7, 0xd3, 0x2c, 0x6b, 0x30, 0xca, 0x5b,
        0xec, 0xd9, 0xc3, 0x7d, 0xc7, 0x40, 0xc1, 0x18, 0x14, 0x8b, 0xe0, 0xe8,
        0x33, 0x76, 0x49, 0x2a, 0xe3, 0x3f, 0x21, 0x49, 0x93, 0xac, 0x4e, 0x0e,
        0xaf, 0x3e, 0x48, 0xcb, 0x65, 0xee, 0xfc, 0xd3, 0x21, 0x0f, 0x65, 0xd2,
        0x2a, 0xd9, 0x32, 0x8f, 0x8c, 0xe5, 0xf7, 0x77, 0xb0, 0x12, 0x7b, 0xb5,
        0x95, 0xc0, 0x89, 0xa3, 0xa9, 0xba, 0xed, 0x73, 0x2e, 0x7a, 0x0c, 0x06,
        0x32, 0x83, 0xa2, 0x7e, 0x8a, 0x14, 0x30, 0xcd, 0x11, 0xa0, 0xe1, 0x2a,
        0x38, 0xb9, 0x79, 0x0a, 0x31, 0xfd, 0x50, 0xbd, 0x80, 0x65, 0xdf, 0xb7,
        0x51, 0x63, 0x83, 0xc8, 0xe2, 0x88, 0x61, 0xea, 0x4b, 0x61, 0x81, 0xec,
        0x52, 0x6b, 0xb9, 0xa2, 0xe2, 0x4b, 0x1a, 0x28, 0x9f, 0x48, 0xa3, 0x9e,
        0x0c, 0xda, 0x09, 0x8e, 0x3e, 0x17, 0x2e, 0x1e, 0xdd, 0x20, 0xdf, 0x5b,
        0xc6, 0x2a, 0x8a, 0xab, 0x2e, 0xbd, 0x70, 0xad, 0xc5, 0x0b, 0x1a, 0x25,
        0x90, 0x74, 0x72, 0xc5, 0x7b, 0x6a, 0xab, 0x34, 0xd6, 0x30, 0x89, 0xff,
        0xe5, 0x68, 0x13, 0x7b, 0x54, 0x0b, 0xc8, 0xd6, 0xae, 0xec, 0x5a, 0x9c,
        0x92, 0x1e, 0x3d, 0x64, 0xb3, 0x8c, 0xc6, 0xdf, 0xbf, 0xc9, 0x41, 0x70,
        0xec, 0x16, 0x72, 0xd5, 0x26, 0xec, 0x38, 0x55, 0x39, 0x43, 0xd0, 0xfc,
        0xfd, 0x18, 0x5c, 0x40, 0xf1, 0x97, 0xeb, 0xd5, 0x9a, 0x9b, 0x8d, 0x1d,
        0xba, 0xda, 0x25, 0xb9, 0xc6, 0xd8, 0xdf, 0xc1, 0x15, 0x02, 0x3a, 0xab,
        0xda, 0x6e, 0xf1, 0x3e, 0x2e, 0xf5, 0x5c, 0x08, 0x9c, 0x3c, 0xd6, 0x83,
        0x69, 0xe4, 0x10, 0x9b, 0x19, 0x2a, 0xb6, 0x29, 0x57, 0xe3, 0xe5, 0x3d,
        0x9b, 0x9f, 0xf0, 0x02, 0x5d, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x42,
        0x30, 0x40, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff,
        0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x0e, 0x06, 0x03, 0x55,
        0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30,
        0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xc4, 0xa7,
        0xb1, 0xa4, 0x7b, 0x2c, 0x71, 0xfa, 0xdb, 0xe1, 0x4b, 0x90, 0x75, 0xff,
        0xc4, 0x15, 0x60, 0x85, 0x89, 0x10, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
        0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01,
        0x01, 0x00, 0xa3, 0x1a, 0x2c, 0x9b, 0x17, 0x00, 0x5c, 0xa9, 0x1e, 0xee,
        0x28, 0x66, 0x37, 0x3a, 0xbf, 0x83, 0xc7, 0x3f, 0x4b, 0xc3, 0x09, 0xa0,
        0x95, 0x20, 0x5d, 0xe3, 0xd9, 0x59, 0x44, 0xd2, 0x3e, 0x0d, 0x3e, 0xbd,
        0x8a, 0x4b, 0xa0, 0x74, 0x1f, 0xce, 0x10, 0x82, 0x9c, 0x74, 0x1a, 0x1d,
        0x7e, 0x98, 0x1a, 0xdd, 0xcb, 0x13, 0x4b, 0xb3, 0x20, 0x44, 0xe4, 0x91,
        0xe9, 0xcc, 0xfc, 0x7d, 0xa5, 0xdb, 0x6a, 0xe5, 0xfe, 0xe6, 0xfd, 0xe0,
        0x4e, 0xdd, 0xb7, 0x00, 0x3a, 0xb5, 0x70, 0x49, 0xaf, 0xf2, 0xe5, 0xeb,
        0x02, 0xf1, 0xd1, 0x02, 0x8b, 0x19, 0xcb, 0x94, 0x3a, 0x5e, 0x48, 0xc4,
        0x18, 0x1e, 0x58, 0x19, 0x5f, 0x1e, 0x02, 0x5a, 0xf0, 0x0c, 0xf1, 0xb1,
        0xad, 0xa9, 0xdc, 0x59, 0x86, 0x8b, 0x6e, 0xe9, 0x91, 0xf5, 0x86, 0xca,
        0xfa, 0xb9, 0x66, 0x33, 0xaa, 0x59, 0x5b, 0xce, 0xe2, 0xa7, 0x16, 0x73,
        0x47, 0xcb, 0x2b, 0xcc, 0x99, 0xb0, 0x37, 0x48, 0xcf, 0xe3, 0x56, 0x4b,
        0xf5, 0xcf, 0x0f, 0x0c, 0x72, 0x32, 0x87, 0xc6, 0xf0, 0x44, 0xbb, 0x53,
        0x72, 0x6d, 0x43, 0xf5, 0x26, 0x48, 0x9a, 0x52, 0x67, 0xb7, 0x58, 0xab,
        0xfe, 0x67, 0x76, 0x71, 0x78, 0xdb, 0x0d, 0xa2, 0x56, 0x14, 0x13, 0x39,
        0x24, 0x31, 0x85, 0xa2, 0xa8, 0x02, 0x5a, 0x30, 0x47, 0xe1, 0xdd, 0x50,
        0x07, 0xbc, 0x02, 0x09, 0x90, 0x00, 0xeb, 0x64, 0x63, 0x60, 0x9b, 0x16,
        0xbc, 0x88, 0xc9, 0x12, 0xe6, 0xd2, 0x7d, 0x91, 0x8b, 0xf9, 0x3d, 0x32,
        0x8d, 0x65, 0xb4, 0xe9, 0x7c, 0xb1, 0x57, 0x76, 0xea, 0xc5, 0xb6, 0x28,
        0x39, 0xbf, 0x15, 0x65, 0x1c, 0xc8, 0xf6, 0x77, 0x96, 0x6a, 0x0a, 0x8d,
        0x77, 0x0b, 0xd8, 0x91, 0x0b, 0x04, 0x8e, 0x07, 0xdb, 0x29, 0xb6, 0x0a,
        0xee, 0x9d, 0x82, 0x35, 0x35, 0x10
      };
      unsigned int DST_Root_CA_X3_der_cer_len = 846;
      if (client.setCACert_P(DST_Root_CA_X3_der_cer, DST_Root_CA_X3_der_cer_len)) {
        if (client.connect(host, 443)) {
  
          if (client.verifyCertChain(host)) {     
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
            Serial.println("Certificate verification failed");
          }                  
          client.stop();
        } else {
          Serial.println("connection failed");    
        }
      } else {
        Serial.println("Failed to load root CA certificate!");      
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
      if (measTime > 0) {
        Serial.println(valuePM25);
        Kniwwelino.MATRIXwriteOnce(valuePM25);
      }
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
      if (measTime > 0) {
        Serial.println(valuePM10);
        Kniwwelino.MATRIXwriteOnce(valuePM10);
      }
    } else {
      if (currentDigit > 11)
        currentDigit = 10;
      else
        if (currentDigit == 10) {
          if (newID.length() > 0) {
            sensorid = newID.toInt();
            lastTry = 0;
            measTime = 0;
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.createObject();
            json["sensorid"] = sensorid;
            String confJSON;
            json.printTo(confJSON);
            Kniwwelino.FILEwrite(FILE_HACKAIR, confJSON);
          }
          Serial.println(sensorid);
          Kniwwelino.MATRIXwriteOnce(String(sensorid));
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
