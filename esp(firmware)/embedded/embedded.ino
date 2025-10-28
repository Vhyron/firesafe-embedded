#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include "config.h"

DHT dht(DHT_PIN, DHT_TYPE);
Servo myServo;

float temperature = 0.0;
float humidity = 0.0;
int mq2Value = 0;
bool servoShouldMove = true;
int servoPosition = 0;
bool servoDirection = true;

bool isCalibrated = false;
float mq2Baseline = 0.0;

unsigned long lastSendTime = 0;
unsigned long lastServoUpdate = 0;
unsigned long calibrationStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);  

  initializeSensors();
  connectToWiFi();
  calibrateSensors();
  initializeServo();
  
  Serial.println("\n System Ready!");
  Serial.println("========================================\n");
}

void loop() {
  // ensure wifi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi disconnected. Reconnecting...");
    connectToWiFi();
  }

  // read sensors and send data at intervals
  unsigned long currentTime = millis();
  
  if (currentTime - lastSendTime >= READING_INTERVAL) {
    lastSendTime = currentTime;
    
    // read sensors
    readSensors();
    
    // send data to cloud and get command
    sendDataAndGetCommand();
  }

  // update servo position
  updateServo();
  
  delay(2000);
}

void initializeSensors() {
  Serial.println("→ Initializing sensors...");
  
  dht.begin();
  Serial.println("  ✓ DHT11 initialized");
  
  pinMode(MQ2_PIN, INPUT);
  Serial.println("  ✓ MQ2 initialized");
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
  Serial.println("\n→ Calibrating sensors...");
  Serial.print("  Please wait ");
  Serial.print(CALIBRATION_TIME);
  Serial.print(" seconds for sensor warm-up...");
  
  calibrationStartTime = millis();
  int dotCount = 0;
  float mq2Sum = 0.0;
  int mq2Count = 0;
  
  // calibration loop
  while (millis() - calibrationStartTime < (CALIBRATION_TIME * 1000)) {
    // read MQ2 for baseline
    int reading = analogRead(MQ2_PIN);
    mq2Sum += reading;
    mq2Count++;
    
    // progress
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

void initializeServo() {
  Serial.println("\n→ Initializing servo...");
  
  myServo.attach(SERVO_PIN);
  myServo.write(0); // start at 0 degrees
  servoPosition = 0;
  
  Serial.println("  Servo initialized at 0°");
  Serial.println("  Default state: Moving (180° sweep)");
}

void readSensors() {
  // read dht11
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  // check if dht reading failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("⚠ DHT11 read error! Using last valid reading.");
    // keep previous values
  }
  
  // read mq2 (gas sensor)
  mq2Value = analogRead(MQ2_PIN);
  
  // print readings
  Serial.println("\n--- Sensor Readings ---");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  Serial.print("MQ2 Gas Level: ");
  Serial.print(mq2Value);
  Serial.print(" (Baseline: ");
  Serial.print(mq2Baseline);
  Serial.println(")");
  Serial.println("----------------------\n");
}

void sendDataAndGetCommand() {
  if (!isCalibrated) {
    Serial.println("⚠ Sensors not calibrated yet. Skipping send.");
    return;
  }
  
  Serial.println("→ Sending data to cloud...");
  
  WiFiClientSecure client;
  client.setInsecure(); // skip certificate validation (use only for testing)
  
  HTTPClient http;
  
  // build JSON payload
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["sensor_1"] = temperature;
  jsonDoc["sensor_2"] = humidity;
  jsonDoc["sensor_3"] = mq2Value;
  
  String payload;
  serializeJson(jsonDoc, payload);
  
  Serial.print("  Payload: ");
  Serial.println(payload);
  
  // send http post request
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
    
    // parse response
    parseCloudResponse(response);
    
  } else {
    Serial.print("  ✗ Error sending data. Code: ");
    Serial.println(httpResponseCode);
    Serial.print("  Error: ");
    Serial.println(http.errorToString(httpResponseCode));
  }
  
  http.end();
}

void parseCloudResponse(String response) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    Serial.print("  ✗ JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }
  
  // check if successful
  if (doc["success"] == true) {
    String command = doc["command"];
    bool thresholdsMet = doc["thresholds_met"];
    
    Serial.print("  → Command received: ");
    Serial.println(command);
    Serial.print("  → Thresholds met: ");
    Serial.println(thresholdsMet ? "YES" : "NO");
    
    // update servo state
    if (command == "stop") {
      servoShouldMove = false;
      Serial.println("  ⏸ SERVO STOPPED - All thresholds met!");
    } else if (command == "move") {
      servoShouldMove = true;
      Serial.println("  ▶ SERVO MOVING - Thresholds not met");
    }
  } else {
    Serial.println("  ✗ Cloud returned error");
  }
}

void updateServo() {
  unsigned long currentTime = millis();
  
  if (servoShouldMove) {
    // update servo position at intervals
    if (currentTime - lastServoUpdate >= SERVO_SWEEP_DELAY) {
      lastServoUpdate = currentTime;
      
      // sweep back and forth
      if (servoDirection) {
        servoPosition++;
        if (servoPosition >= SERVO_SWEEP_ANGLE) {
          servoDirection = false;
        }
      } else {
        servoPosition--;
        if (servoPosition <= 0) {
          servoDirection = true;
        }
      }
      
      myServo.write(servoPosition);
    }
  } else {
    // keep servo at current position, no movement
  }
}