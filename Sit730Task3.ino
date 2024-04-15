#include <WiFiNINA.h>
#include <Wire.h>
#include <BH1750.h>

const char* ssid = "Saif"; 
const char* password = "Ubitseca1.!"; 

WiFiClient client; 

char HOST_NAME[] = "maker.ifttt.com"; 
String PATH_NAME = "/trigger/sunlight_exposure_started/with/key/bUTjgRvuHfUQsKRGoXzYff"; 
BH1750 lightMeter;

bool inSunlight = false;
unsigned long sunlightStart; 
bool dayDone = false; 

void setup() {
  WiFi.begin(ssid, password);

  Serial.begin(9600); 
  while (!Serial); 
  Wire.begin();
  lightMeter.begin();
}

float readLightLevel() {
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  return lux;
}

void sendWebhook(String message) {
  if (client.connect(HOST_NAME, 80)) {
    Serial.println("Connected to server");
    client.print("GET " + PATH_NAME + "?value1=" + message + " HTTP/1.1\r\n");
    client.print("Host: " + String(HOST_NAME) + "\r\n");
    client.print("Connection: close\r\n"); 
    client.println();
    delay(1000); 

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }

    client.stop();
    Serial.println();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection to server failed");
  }
}

void loop() {
  float lux = readLightLevel();
  if (lux <= 2 && dayDone) {
    dayDone = false;
  }
  if (lux > 5 && !inSunlight && !dayDone) {
    inSunlight = true;
    sunlightStart = millis();
    sendWebhook("Terrarium in sunlight, 3 minutes remaining"); // Notification for sunlight start - 3min used for demo - real time requires 2hrs
  }

  if (inSunlight) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - sunlightStart;

    if (elapsedTime >= 3 * 60 * 1000) { 
      sendWebhook("3 minutes reached, cover terrarium"); 
      inSunlight = false;
      dayDone = true;
    }
  }
  delay(60000); 
}
