// =================================================================
// Update/Add any #define values to match your build and board type 
// =================================================================
// This version shares pins for both LED eyes (no independent eye control)
// Enabling serial debug is optional, but can be used when connected via USB

// pin definitions
#define AUDIO_RX_PIN D1
#define AUDIO_TX_PIN D2
#define SERVO_PIN D4
#define MOTION_PIN D5
#define EYE_RED_PIN D6
#define EYE_GREEN_PIN D8
#define EYE_BLUE_PIN D7

#define WIFIMODE 2                            // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both
#define MQTTMODE 0                            // 0 = Disable MQTT, 1 = Enable (will only be enabled if WiFi mode = 1 or 2 - broker must be on same network)
#define SERIAL_DEBUG 0                        // 0 = Disable (must be disabled if using RX/TX pins), 1 = enable
#define MQTTCLIENT "skullhead"                // MQTT Client Name
#define MQTT_TOPIC_SUB "cmnd/skullhead"       // Default MQTT subscribe topic
#define MQTT_TOPIC_PUB "stat/skullhead"       // Default MQTT publish topic
#define OTA_HOSTNAME "SkullHeadOTA"           // Hostname to broadcast as port in the IDE of OTA Updates


// ---------------------------------------------------------------------------------------------------------------
// Options - Defaults upon boot-up or any other custom ssttings
// ---------------------------------------------------------------------------------------------------------------
// OTA Settings
bool ota_flag = true;                    // Must leave this as true for board to broadcast port to IDE upon boot (OTA Updates)
uint16_t ota_boot_time_window = 2500;    // minimum time on boot for IP address to show in IDE ports, in millisecs
uint16_t ota_time_window = 20000;        // time to start file upload when ota_flag set to true (after initial boot), in millsecs

// Other options
byte headPos = 90;                       // Default head postion on boot/reset - for 180 servo, 90 would be midpoint
int step_delay = 50;                     // Delay, in milliseconds, between step moves during servo rotation (moves in 5 degree steps)
String eye_color_idle = "green";         // Default boot color - also default eye color when idle (no motion) when autoMotion true
int audioVolume = 25;                    // Audio volume 0-30

bool autoMotion = true;                  // Activate on detected motion

// These only apply if autoMotion is true
uint16_t motion_reset_time = 15000;      // time to delay between motion events
uint16_t head_reset_time = 3000;         // time to pause after head moves before returning to start pos, in millisecs
String eye_color_active = "red";         // Default eye color when active (motion detected) 
byte rotate_dir = 1;                     // 0 to have head rotate to the right, 1 to rotate to the left
byte min_rotate = 30;                    // Minimum position, in degrees, when rotating to the right
byte max_rotate = 150;                   // Maximum position, in degrees, when rotating to the left
bool autoBlink = true;                   // Blink eyes at random interval between max and min blink times
uint16_t blink_time_max = 10000;         // Max time between blinks when no motion in millisecs
uint16_t blink_time_min = 3000;          // Min time between blinks when no motion in millisecs (between 0 and blink_max_time)
