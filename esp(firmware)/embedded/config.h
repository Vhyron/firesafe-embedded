#ifndef CONFIG_H
#define CONFIG_H

// wifi credentials
#define WIFI_SSID "Hotspot"
#define WIFI_PASSWORD "bikbikhotspot2025"

// supabase configuration
#define SUPABASE_URL "https://osjffvmvpvkzccjfzhmn.supabase.co"
#define SUPABASE_ANON_KEY "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im9zamZmdm12cHZremNjamZ6aG1uIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjE0MDgyNzgsImV4cCI6MjA3Njk4NDI3OH0.qI0j4UW9do5E8vRpeo5G8IOMsCrW58gHIJ_JYFACFwU"
#define EDGE_FUNCTION_URL "https://osjffvmvpvkzccjfzhmn.supabase.co/functions/v1/process-sensor-data"

// pin configuration
#define DHT_PIN D4 
#define MQ2_PIN A0
#define SERVO_PIN D2

// sensor configuration
#define DHT_TYPE DHT11
#define CALIBRATION_TIME 20
#define READING_INTERVAL 5000

// servo configuration
#define SERVO_SWEEP_ANGLE 180
#define SERVO_SWEEP_DELAY 15

#endif