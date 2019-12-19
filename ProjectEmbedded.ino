#include <ArduinoJson.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define rxPin D7
#define txPin D8

#define DATA_LEN 10
#define SINTERVAL 5
#define FAN_PIN 5

const char* endpoint = "http://35.185.181.0:3000/api/pm";
const char* ssid = "Unister";
const char* pass = "Unizer01";
SoftwareSerial sSerial(rxPin, txPin);
int wtf = 0;
char serialBuffer[64];
int count = 0;
int onboardLed = LED_BUILTIN;
int pm25 = 0, pm10 = 0;
bool isAuto = false;

void setup() {
  Serial.begin(9600);
  sSerial.begin(9600);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting ...");
  }
  Serial.println("Connected");

  pinMode(onboardLed, OUTPUT);
  digitalWrite(onboardLed, HIGH);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
}

void loop() {
  while (sSerial.available() > 0) {
    serialBuffer[count++] = sSerial.read();
  }

  if (count >= DATA_LEN) {
    wtf++;
    count = 0;
    if (wtf > SINTERVAL) {
      wtf = 0;
      sscanf(serialBuffer, "!%4d;%4d", &pm10, &pm25);
      Serial.print(pm10);
      Serial.print(" ");
      Serial.println(pm25);
      HTTPClient http;
      http.begin(endpoint); //HTTP Req
      http.addHeader("Content-Type", "application/json");
      char payload[100];
      sprintf(payload, "{ \"pm\": %d }", pm25);
      int httpCode = http.POST(payload);
      Serial.println(httpCode);
      String res = http.getString();
      StaticJsonBuffer<200> jsonBuffer;
      Serial.println(res);
      JsonObject &root = jsonBuffer.parseObject(res);
      Serial.print("Auto: ");
      isAuto = root["autostate"];
      Serial.println(isAuto);
      digitalWrite(FAN_PIN, isAuto ? HIGH : LOW);
      http.end();
    }
  }
}
