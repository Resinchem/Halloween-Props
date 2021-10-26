// ===============================================================================
// Update/Add any #define values to match your build and board type if not using D1 Mini
// =================================================================
// This version uses independent pins for each LED "eye" and the RX (GPIO3) and TX (GPIO1)
// for the audio connection.  SERIAL_DEBUG MUST be disabled when this setup is used.

// pin definitions
#define SERVO_PIN D4
#define MOTION_PIN D5
#define LEFT_EYE_RED_PIN D1
#define LEFT_EYE_GREEN_PIN D2
#define LEFT_EYE_BLUE_PIN D3
#define RIGHT_EYE_RED_PIN D6
#define RIGHT_EYE_GREEN_PIN D7
#define RIGHT_EYE_BLUE_PIN D8
#define RX_PIN 3                             // SERIAL_DEBUG must be 0 when using these pins!
#define TX_PIN 1

#define WIFIMODE 2                            // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both
#define MQTTMODE 1                            // 0 = Disable MQTT, 1 = Enable (will only be enabled if WiFi mode = 1 or 2 - broker must be on same network)
#define SERIAL_DEBUG 0                        // 0 = Disable (must be disabled if using RX/TX pins), 1 = enable
#define MQTTCLIENT "goblinhead"               // MQTT Client Name
#define MQTT_TOPIC_SUB "cmnd/goblinhead"      // Default MQTT subscribe topic
#define MQTT_TOPIC_PUB "stat/goblinhead"      // Default MQTT publish topic
#define OTA_HOSTNAME "GoblinHeadOTA"          // Hostname to broadcast as port in the IDE of OTA Updates

// ---------------------------------------------------------------------------------------------------------------
// Options - Defaults upon boot-up or any other custom ssttings
// ---------------------------------------------------------------------------------------------------------------
// OTA Settings
bool ota_flag = true;                    // Must leave this as true for board to broadcast port to IDE upon boot
uint16_t ota_time_elapsed = 0;           // Counter when OTA active
uint16_t ota_boot_time_window = 2500;    // minimum time on boot for IP address to show in IDE ports, in millisecs
uint16_t ota_time_window = 20000;        // time to start file upload when ota_flag set to true (after initial boot), in millsecs

// Other options
int headPos = 90;                        // Default head postion on boot/reset - for 180 servo, 90 would be midpoint
String eye_color_idle = "blue";          // Default boot color - also default eye color when idle (no motion) when autoMotion true

bool autoMotion = true;                  // Activate on detected motion

// These only apply if autoMotion is true
uint16_t head_reset_time = 15000;        // time to hold head pos before motion reset in millisecs
bool autoBlink = true;                   // Blink eyes at random interval between max and min blink times
uint16_t blink_time_max = 10000;         // Max time between blinks when no motion in millisecs
uint16_t blink_time_min = 3000;          // Min time between blinks when no motion in millisecs (between 0 and blink_max_time)
String eye_color_active = "red";         // Default eye color when active (motion detected) 
