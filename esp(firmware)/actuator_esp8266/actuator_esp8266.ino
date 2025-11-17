// ========================================
// SERVO ESP8266 - main.ino
// ========================================

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include "config.h"

Servo myServo;

String currentMode = "automatic";

int currentPosition = 90;
bool isInitialPhase = true;
bool movingUp = true;
bool servoShouldMove = true;

int manualTargetPosition = 90;

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
  
  // check for commands from cloud every 3 seconds
  if (currentTime - lastCommandCheck >= COMMAND_CHECK_INTERVAL) {
    lastCommandCheck = currentTime;
    checkCloudCommand();
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
  
  // start at 90 degrees
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
  
  // send GET request
  http.begin(client, COMMAND_CHECK_URL);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_ANON_KEY));
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("  ✓ Response code: ");
    Serial.println(httpResponseCode);
    
    // parse response
    parseCloudCommand(response);
    
  } else {
    Serial.print("  ✗ Error checking command. Code: ");
    Serial.println(httpResponseCode);
    Serial.println("  → Continuing in current mode");
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
    String mode = doc["mode"];
    String command = doc["command"];
    
    // check if mode changed
    if (mode != currentMode) {
      Serial.println("\n  ╔═══════════════════════════════╗");
      Serial.print("  ║  MODE CHANGED: ");
      Serial.print(mode);
      Serial.println("          ║");
      Serial.println("  ╚═══════════════════════════════╝");
      currentMode = mode;
    }
    
    // manual mode
    if (mode == "manual") {
      int manualPos = doc["manual_position"];
      
      Serial.println("\n  🎮 MANUAL MODE");
      Serial.print("  → Target Position: ");
      Serial.print(manualPos);
      Serial.println("°");
      
      // move to manual position if different from current
      if (manualPos != currentPosition) {
        Serial.print("  → Moving from ");
        Serial.print(currentPosition);
        Serial.print("° to ");
        Serial.print(manualPos);
        Serial.println("°");
        
        myServo.write(manualPos);
        currentPosition = manualPos;
        
        Serial.print("  ✓ Position set to ");
        Serial.print(currentPosition);
        Serial.println("°");
      } else {
        Serial.print("  → Holding at ");
        Serial.print(currentPosition);
        Serial.println("°");
      }
    }
    // automatic mode
    else if (mode == "automatic") {
      bool thresholdsMet = doc["thresholds_met"];
      
      // display sensor data if available
      if (doc.containsKey("current_readings")) {
        float temp = doc["current_readings"]["temperature"] | 0.0;
        float humidity = doc["current_readings"]["humidity"] | 0.0;
        float gas = doc["current_readings"]["gas"] | 0.0;
        
        Serial.println("\n  📊 Sensor Data:");
        Serial.print("    Temp: ");
        Serial.print(temp);
        Serial.print("°C, Humidity: ");
        Serial.print(humidity);
        Serial.print("%, Gas: ");
        Serial.println(gas);
      }
      
      Serial.print("\n  🤖 AUTOMATIC MODE - Command: ");
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
          Serial.print("  → Remains STOPPED at ");
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
        }
        
        // move to next position in automatic pattern
        moveToNextPosition();
      }
    }
  } else {
    Serial.println("  ⚠ Cloud returned error");
  }
}

void moveToNextPosition() {
  int nextPosition;
  
  // phase 1: initial movement from 90° to 0°
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
      if (currentPosition < 180) {
        nextPosition = currentPosition + 30;
        if (nextPosition > 180) nextPosition = 180;
      } else {
        // reached 180°, reverse
        nextPosition = 150;
        movingUp = false;
      }
    } else {
      if (currentPosition > 0) {
        nextPosition = currentPosition - 30;
        if (nextPosition < 0) nextPosition = 0;
      } else {
        // reached 0°, reverse
        nextPosition = 30;
        movingUp = true;
      }
    }
  }
  
  // move servo
  myServo.write(nextPosition);
  currentPosition = nextPosition;
  
  Serial.print("  ▶ Moved to ");
  Serial.print(currentPosition);
  Serial.println("°");
}