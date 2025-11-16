// ========================================
// SERVO ESP8266 - main.ino
// Servo Motor Actuator Control
// ========================================

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include "config.h"

Servo myServo;

int currentPosition = 90;
bool isInitialPhase = true;
bool movingUp = true;

bool servoShouldMove = true;
unsigned long lastCommandCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  connectToWiFi();
  initializeServo();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi disconnected. Reconnecting...");
    connectToWiFi();
  }

  unsigned long currentTime = millis();
  
  if (currentTime - lastCommandCheck >= COMMAND_CHECK_INTERVAL) {
    lastCommandCheck = currentTime;
    checkCloudCommand();
    
    if (servoShouldMove) {
      moveToNextPosition();
    }
  }
  
  delay(100);
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

void initializeServo() {
  Serial.println("\n→ Initializing servo...");
  
  myServo.attach(SERVO_PIN, 500, 2400);
  delay(100);
  
  myServo.write(90);
  currentPosition = 90;
  delay(500);
}

void checkCloudCommand() {
  Serial.println("\n┌─────────────────────────────────┐");
  Serial.print("│ Checking cloud at ");
  Serial.print(millis() / 1000);
  Serial.println("s");
  Serial.println("└─────────────────────────────────┘");
  
  WiFiClientSecure client;
  client.setInsecure();
  
  HTTPClient http;
  
  // send GET request to check for command
  http.begin(client, COMMAND_CHECK_URL);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_ANON_KEY));
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("  ✓ Response code: ");
    Serial.println(httpResponseCode);
    
    parseCloudCommand(response);
    
  } else {
    Serial.print("  ✗ Error checking command. Code: ");
    Serial.println(httpResponseCode);
    Serial.println("  → Defaulting to MOVE mode");
    servoShouldMove = true;
  }
  
  http.end();
}

void parseCloudCommand(String response) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    Serial.print("  ✗ JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }
  
  if (doc["success"] == true) {
    String command = doc["command"];
    bool thresholdsMet = doc["thresholds_met"];
    
    float temp = doc["current_readings"]["temperature"] | 0.0;
    float humidity = doc["current_readings"]["humidity"] | 0.0;
    float gas = doc["current_readings"]["gas"] | 0.0;
    
    Serial.println("\n  → Sensor Data Received:");
    Serial.print("    Temperature: ");
    Serial.print(temp);
    Serial.println("°C");
    Serial.print("    Humidity:    ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("    Gas Level:   ");
    Serial.println(gas);
    
    Serial.print("\n  → Command: ");
    Serial.println(command);
    Serial.print("  → Thresholds met: ");
    Serial.println(thresholdsMet ? "YES ✓" : "NO ✗");
    
    // update servo state
    bool previousState = servoShouldMove;
    
    if (command == "stop") {
      servoShouldMove = false;
      if (previousState != servoShouldMove) {
        Serial.println("\n  ╔═══════════════════════════════╗");
        Serial.println("  ║  ⏸  SERVO STOPPED            ║");
        Serial.println("  ║  All thresholds met!         ║");
        Serial.print("  ║  Holding at: ");
        Serial.print(currentPosition);
        Serial.println("°               ║");
        Serial.println("  ╚═══════════════════════════════╝");
      } else {
        Serial.print("  → Servo remains STOPPED at ");
        Serial.print(currentPosition);
        Serial.println("°");
      }
    } else if (command == "move") {
      servoShouldMove = true;
      if (previousState != servoShouldMove) {
        Serial.println("\n  ╔═══════════════════════════════╗");
        Serial.println("  ║  ▶  SERVO RESUMING           ║");
        Serial.println("  ║  Thresholds not met          ║");
        Serial.println("  ╚═══════════════════════════════╝");
      } else {
        Serial.println("  → Servo continues MOVING");
      }
    }
  } else {
    Serial.println("  ⚠ Cloud returned error or no data");
    Serial.println("  → Defaulting to MOVE mode");
    servoShouldMove = true;
  }
}

void moveToNextPosition() {
  int nextPosition;
  
  // phase 1: starts at 90°
  if (isInitialPhase) {
    if (currentPosition > 0) {
      nextPosition = currentPosition - 30;
    } else {
      // reached 0°, switch to looping phase
      nextPosition = 30;
      movingUp = true;
      isInitialPhase = false;
      Serial.println("  ✓ Initial phase complete. Starting 0° ↔ 180° loop");
    }
  }
  // phase 2: loop between 0° and 180°
  else {
    if (movingUp) {
      // moving from 0° to 180°
      if (currentPosition < 180) {
        nextPosition = currentPosition + 30;
        if (nextPosition > 180) nextPosition = 180;
      } else {
        // reached 180°, reverse direction
        nextPosition = 150;
        movingUp = false;
      }
    } else {
      // moving from 180° to 0°
      if (currentPosition > 0) {
        nextPosition = currentPosition - 30;
        if (nextPosition < 0) nextPosition = 0;
      } else {
        // reached 0°, reverse direction
        nextPosition = 30;
        movingUp = true;
      }
    }
  }
  
  myServo.write(nextPosition);
  currentPosition = nextPosition;
  
  Serial.print("  ▶ Moved to ");
  Serial.print(currentPosition);
  Serial.println("°");
}