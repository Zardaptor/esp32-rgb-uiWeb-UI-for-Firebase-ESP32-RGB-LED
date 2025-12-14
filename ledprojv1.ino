#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_NeoPixel.h>
#include "firebase_secrets.h"

// Wi-Fi credentials
const char* ssid = "YourWiFiName";           // <--- change
const char* password = "YourWiFiPassword";   // <--- change

#define LED_PIN     8
#define LED_COUNT   1
Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Current RGB values
int r = 0, g = 0, b = 0;
String lastData = "";

void setup() {
  Serial.begin(115200);
  pixel.begin();
  pixel.setBrightness(50);
  pixel.show();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Firebase config
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = ""; // not needed with admin SDK
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready()) {
    if (Firebase.RTDB.getJSON(&fbdo, "/led")) {
      FirebaseJson &json = fbdo.jsonObject();
      String newData;
      json.toString(newData, true);
      if (newData != lastData) {
        lastData = newData;
        int newR, newG, newB;
        json.get("r", newR);
        json.get("g", newG);
        json.get("b", newB);

        r = constrain(newR, 0, 255);
        g = constrain(newG, 0, 255);
        b = constrain(newB, 0, 255);

        Serial.printf("Updating LED: R=%d, G=%d, B=%d\n", r, g, b);
        pixel.setPixelColor(0, pixel.Color(r, g, b));
        pixel.show();
      }
    } else {
      Serial.println("Firebase read failed: " + fbdo.errorReason());
    }
  }
  delay(300); // polling interval
}
