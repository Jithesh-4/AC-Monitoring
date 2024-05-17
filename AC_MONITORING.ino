#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "DHT.h"

#define WIFI_SSID "car"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyAoFkx_LXkjP6-WorpjPPzxHe3dFwGaJ3c"
#define DATABASE_URL "https://ac-monitoring-510cf-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


#define DPIN 2       
#define DTYPE DHT11
DHT dht(DPIN,DTYPE);

int rpm_sens = 4;
int air_sens = 34;

int rpm;
int air;

void setup() {

  Serial.begin(115200);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  
  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase sign up successful");
  } 
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void loop() {
  
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    float tc = dht.readTemperature(false);
    float tf = dht.readTemperature(true);
    float hu = dht.readHumidity();

    rpm = digitalRead(rpm_sens);
    air = analogRead(air_sens);

    if (Firebase.RTDB.setFloat(&fbdo, "sens/temp", tc) &&
        Firebase.RTDB.setFloat(&fbdo, "sens/hum", hu) &&
        Firebase.RTDB.setFloat(&fbdo, "sens/air", air) &&
        Firebase.RTDB.setFloat(&fbdo, "sens/rpm", rpm)) {
      Serial.println("Uploaded");
    } 
    
    else {
      Serial.println(fbdo.errorReason());
    }
  }
  delay(300);
}