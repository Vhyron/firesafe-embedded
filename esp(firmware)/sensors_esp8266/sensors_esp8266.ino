// ========================================
// SENSOR ESP8266 - main.ino
// DHT11 + MQ2 Gas Sensor
// ========================================

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include "config.h"

DHT dht(DHT_PIN, DHT_TYPE);

float temperature = 0.0;
float humidity = 0.0;
int mq2Value = 0;

bool isCalibrated = false;
float mq2Baseline = 0.0;

unsigned long lastSendTime = 0;
unsigned long calibrationStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  initializeSensors();
  connectToWiFi();
  calibrateSensors();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi disconnected. Reconnecting...");
    connectToWiFi();
  }

  // read sensors and send data every 3 seconds
  unsigned long currentTime = millis();
  
  if (currentTime - lastSendTime >= READING_INTERVAL) {
    lastSendTime = currentTime;
    
    readSensors();

    sendDataToCloud();
  }
  
  delay(100);
}

void initializeSensors() {
  Serial.println("→ Initializing sensors...");
  
  dht.begin();
  Serial.println("  ✓ DHT11 initialized on pin D4");
  
  pinMode(MQ2_PIN, INPUT);
  Serial.println("  ✓ MQ2 initialized on pin A0");
}

void connectToWiFi() {
  Serial.print("→ Connecting to WiFi: ");
  Serial.print(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n  ✓ WiFi connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n  ✗ WiFi connection failed!");
    Serial.println("  Please check your credentials and reset the board.");
    while(1) { delay(1000); }
  }
}

void calibrateSensors() {
  Serial.println("\n→ Calibrating MQ2 sensor...");
  Serial.print("  Please wait ");
  Serial.print(CALIBRATION_TIME);
  Serial.print(" seconds for sensor warm-up...");
  
  calibrationStartTime = millis();
  int dotCount = 0;
  float mq2Sum = 0.0;
  int mq2Count = 0;
  
  while (millis() - calibrationStartTime < (CALIBRATION_TIME * 1000)) {
    // MQ2 baseline
    int reading = analogRead(MQ2_PIN);
    mq2Sum += reading;
    mq2Count++;
    
    if (dotCount % 10 == 0) {
      Serial.print(".");
    }
    dotCount++;
    
    delay(100);
  }
  
  // calculate baseline
  mq2Baseline = mq2Sum / mq2Count;
  
  Serial.println("\n  ✓ Calibration complete!");
  Serial.print("  MQ2 Baseline: ");
  Serial.println(mq2Baseline);
  
  isCalibrated = true;
}

void readSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("⚠ DHT11 read error! Using last valid reading.");
  }
  
  mq2Value = analogRead(MQ2_PIN);
  
  // readings with timestamp
  Serial.println("\n┌─────────────────────────────────┐");
  Serial.print("│ READING at ");
  Serial.print(millis() / 1000);
  Serial.println("s");
  Serial.println("├─────────────────────────────────┤");
  Serial.print("│ Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("│ Humidity:    ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("│ MQ2 Gas:     ");
  Serial.print(mq2Value);
  Serial.print(" (Baseline: ");
  Serial.print(mq2Baseline);
  Serial.println(")");
  Serial.println("└─────────────────────────────────┘");
}

void sendDataToCloud() {
  if (!isCalibrated) {
    Serial.println("⚠ Sensors not calibrated yet. Skipping send.");
    return;
  }
  
  Serial.println("→ Sending data to cloud...");
  
  WiFiClientSecure client;
  client.setInsecure();
  
  HTTPClient http;
  
  // JSON payload
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["sensor_1"] = temperature;
  jsonDoc["sensor_2"] = humidity;
  jsonDoc["sensor_3"] = mq2Value;
  
  String payload;
  serializeJson(jsonDoc, payload);
  
  Serial.print("  Payload: ");
  Serial.println(payload);
  
  // send HTTP POST request
  http.begin(client, EDGE_FUNCTION_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_ANON_KEY));
  
  int httpResponseCode = http.POST(payload);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("  ✓ Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("  Response: ");
    Serial.println(response);
    Serial.println();
  } else {
    Serial.print("  ✗ Error sending data. Code: ");
    Serial.println(httpResponseCode);
    Serial.print("  Error: ");
    Serial.println(http.errorToString(httpResponseCode));
    Serial.println();
  }
  
  http.end();
}