#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 4       
#define DHTTYPE DHT11  

// Konfigurasi WiFi
const char* WIFI_SSID = "J";  
const char* WIFI_PASSWORD = "JihanAurelia";  

// Konfigurasi Ubidots
const char* UBIDOTS_TOKEN = "BBUS-GlLTphIA15BbfK5st6xDSCSoVvVUPt";  
const char* DEVICE_LABEL = "esp32-sic6-2";  
const char* UBIDOTS_URL = "https://industrial.api.ubidots.com/api/v1.6/devices/";

// Konfigurasi API Service untuk MongoDB
const char* API_URL = "http://192.168.29.181:5000/api/sensor"; // Sesuaikan dengan alamat server Flask

DHT dht(DHTPIN, DHTTYPE);

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nERROR: WiFi NOT Connected! Retrying in 10 seconds...");
    delay(10000);
    connectWiFi();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nBooting ESP32...");

  connectWiFi();
  dht.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Lost. Reconnecting...");
    connectWiFi();
  }

  delay(5000);

  float temperature = dht.readTemperature();  
  float humidity = dht.readHumidity();       
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("ERROR: Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  Serial.print("Heat Index: ");
  Serial.print(heatIndex);
  Serial.println("°C");

  sendDataToUbidots(temperature, humidity, heatIndex);
  sendDataToAPI(temperature, humidity, heatIndex);
}

void sendDataToUbidots(float temp, float hum, float hIndex) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ERROR: WiFi Disconnected. Unable to send data.");
    return;
  }

  HTTPClient http;
  String url = String(UBIDOTS_URL) + String(DEVICE_LABEL) + "/?token=" + UBIDOTS_TOKEN;
  
  Serial.println("Sending data to: " + url);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"temp\": {\"value\": " + String(temp) + "},";
  payload += "\"humidity\": {\"value\": " + String(hum) + "},";
  payload += "\"heat_index\": {\"value\": " + String(hIndex) + "}";
  payload += "}";

  Serial.print("Sending JSON payload: ");
  Serial.println(payload);

  int httpResponseCode = http.POST(payload);

  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response from Ubidots: " + response);
  } else {
    Serial.println("ERROR: Failed to send data to Ubidots.");
  }

  http.end();
}

void sendDataToAPI(float temp, float hum, float hIndex) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ERROR: WiFi Disconnected. Unable to send data.");
    return;
  }

  HTTPClient http;
  String url = API_URL;
  
  Serial.println("Sending data to API: " + url);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"temperature\": " + String(temp) + ",";
  payload += "\"humidity\": " + String(hum) + ",";
  payload += "\"heat_index\": " + String(hIndex);
  payload += "}";

  Serial.print("Sending JSON payload to API: ");
  Serial.println(payload);

  int httpResponseCode = http.POST(payload);

  Serial.print("API Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response from API: " + response);
  } else {
    Serial.println("ERROR: Failed to send data to API.");
  }

  http.end();
}
