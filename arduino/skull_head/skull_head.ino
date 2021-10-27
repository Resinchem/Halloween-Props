/*
====================================================================================
Main .ino file for Skull head build as described at: 
https://resinchemtech.blogspot.com/2021/10/halloween-props-with-moving-head-and.html
Created by: ResinChem Tech
License: Creative Commons Attribution-NonCommercial 4.0
Initial creation: October 25, 2021
====================================================================================
*/
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <FS.h>
#include "Credentials.h"          //File must exist in same folder as .ino.  Edit as needed for project
#include "Settings.h" 
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  #include <PubSubClient.h>
#endif

//GLOBAL VARIABLES
bool mqttConnected = false;       //Will be enabled if defined and successful connnection made.  This var should be checked upon any MQTT actin.
long lastReconnectAttempt = 0;    //If MQTT connected lost, attempt reconnenct
uint16_t ota_time = ota_boot_time_window;
uint16_t ota_time_elapsed = 0;           // Counter when OTA active
int motionState = 0;
byte curHeadPos = 0;
unsigned long nextRunTime = 0;
unsigned long nextBlinkTime = 0;


//Setup Local Access point if enabled via WIFI Mode
#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2)
  const char* APssid = AP_SSID;        
  const char* APpassword = AP_PWD;  
#endif
//Setup Wifi if enabled via WIFI Mode
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2)
  #include "Credentials.h"                    // Edit this file in the same directory as the .ino file and add your own credentials
  const char *ssid = SID;
  const char *password = PW;
#endif
//Setup MQTT if enabled - only available when WiFi is also enabled
#if (WIFIMODE == 1 || WIFIMODE == 2) && (MQTTMODE == 1)    // MQTT only available when on local wifi
  const char *mqttUser = MQTTUSERNAME;
  const char *mqttPW = MQTTPWD;
  const char *mqttClient = MQTTCLIENT;
  const char *mqttTopicSub = MQTT_TOPIC_SUB;
 // const char *mqttTopicPub = MQTT_TOPIC_PUB;
#endif

WiFiClient espClient;
ESP8266WebServer server;
Servo servo;
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  PubSubClient client(espClient);
#endif

SoftwareSerial mySoftwareSerial(AUDIO_RX_PIN, AUDIO_TX_PIN); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

//------------------------------
// Setup WIFI
//-------------------------------
void setup_wifi() {
  WiFi.setSleepMode(WIFI_NONE_SLEEP);  //Disable WiFi Sleep
  delay(200);
  // WiFi - AP Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2) 
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(APssid, APpassword);    // IP is usually 192.168.4.1
#endif
  // WiFi - Local network Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2) 
  byte count = 0;
  WiFi.begin(ssid, password);
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.print("Connecting to WiFi");
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.print(".");
    #endif
    // Stop if cannot connect
    if (count >= 60) {
      // Could not connect to local WiFi 
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println();
        Serial.println("Could not connect to WiFi.");   
      #endif  
      return;
    }
    delay(500);
    count++;
  }
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println();
    Serial.println("Successfully connected to Wifi");
    IPAddress ip = WiFi.localIP();
    Serial.println(WiFi.localIP());
  #endif
#endif   
};

//------------------------------
// Setup MQTT
//-------------------------------
void setup_mqtt() {
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  byte mcount = 0;
  //char topicPub[32];
  client.setServer(MQTTSERVER, MQTTPORT);
  client.setCallback(callback);
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.print("Connecting to MQTT broker.");
  #endif
  while (!client.connected( )) {
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.print(".");
    #endif
    client.connect(mqttClient, mqttUser, mqttPW);
    if (mcount >= 60) {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.println();
        Serial.println("Could not connect to MQTT broker. MQTT disabled.");
      #endif
      // Could not connect to MQTT broker
      return;
    }
    delay(500);
    mcount++;
  }
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println();
    Serial.println("Successfully connected to MQTT broker.");
  #endif
  client.subscribe(MQTT_TOPIC_SUB"/#");
  client.publish(MQTT_TOPIC_PUB"/mqtt", "connected", true);
  mqttConnected = true;
#endif
}

void reconnect() 
{
  int retries = 0;
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  while (!client.connected()) {
    if(retries < 150)
    {
      #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
        Serial.print("Attempting MQTT connection...");
      #endif
      if (client.connect(mqttClient, mqttUser, mqttPW)) 
      {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.println("connected");
        #endif
        // ... and resubscribe
        client.subscribe(MQTT_TOPIC_SUB"/#");
      } 
      else 
      {
        #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
        #endif
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if(retries > 149)
    {
    ESP.restart();
    }
  }
#endif
}
// --- MQTT Message Processing
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = (char*)payload;
  // Eye color
  if (strcmp(topic, MQTT_TOPIC_SUB"/eyecolor") == 0) {
    setEyeColor(message);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/blink") == 0) {
    int blinkSpeed = message.toInt();
    blinkEyes(blinkSpeed);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/movehead") == 0) {
  // Move head
    int moveTo = message.toInt();
    moveHead(moveTo);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/playsound") == 0) {
    // Play Audio Sound
    int whichTrack = message.toInt();
    playAudio(whichTrack);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/audiovol") == 0) {
    int newVol = message.toInt();
    setAudioVolume(newVol);  
  }
}

// ============================================
//   MAIN SETUP
// ============================================
void setup() {
  // Serial monitor
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.begin(115200);
    Serial.println("Booting...");
  #endif
  setup_wifi();
  setup_mqtt();

  //-----------------------------
  // Setup OTA Updates
  //-----------------------------
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
  });
  ArduinoOTA.begin();
  // Setup handlers for web calls for OTAUpdate and Restart
  server.on("/restart",[](){
    server.send(200, "text/html", "<h1>Restarting...</h1>");
    delay(1000);
    ESP.restart();
  });
  server.on("/otaupdate",[]() {
    server.send(200, "text/html", "<h1>Ready for upload...<h1><h3>Start upload from IDE now</h3>");
    ota_flag = true;
    ota_time = ota_time_window;
    ota_time_elapsed = 0;
  });
  server.begin();

  //Initialize servo
  servo.attach(SERVO_PIN);
  
  //Setup LEDs
  pinMode(EYE_RED_PIN, OUTPUT);
  pinMode(EYE_GREEN_PIN, OUTPUT);
  pinMode(EYE_BLUE_PIN, OUTPUT);

  //Motion Sensor
  pinMode(MOTION_PIN, INPUT);

  //Default boot eye color
  setEyeColor(eye_color_idle);

// Set default servo pos to middle (90 for 180 servo) 
  servo.write(headPos);
  curHeadPos = headPos;
// Test Audio
  mySoftwareSerial.begin(9600);
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  #endif
  if (!myDFPlayer.begin(mySoftwareSerial, false)) {  //Use softwareSerial to communicate with mp3.
    #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
    #endif
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  #if defined(SERIAL_DEBUG) && (SERIAL_DEBUG == 1)
    Serial.println(F("DFPlayer Mini online."));
  #endif

  // Assure audioVolume from Settings.h is in acceptable range or fix
  if (audioVolume > 30) {
    audioVolume = 30;
  } else if (audioVolume < 0) {
    audioVolume = 0;
  }
  
  myDFPlayer.volume(audioVolume);  //Set volume value. From 0 to 30
  myDFPlayer.play(1);  //Play the first mp3

// set random seed for eye blink
  randomSeed(analogRead(0));  
}
// ==============================================
//  MAIN LOOP HERE
//===============================================
void loop() {

   //Handle OTA updates when OTA flag set via HTML call to http://ip_address/otaupdate
  if (ota_flag) {
    uint16_t ota_time_start = millis();
    while (ota_time_elapsed < ota_time) {
      ArduinoOTA.handle();  
      ota_time_elapsed = millis()-ota_time_start;   
      delay(10); 
    }
    ota_flag = false;
  }
  //Handle any web calls
  server.handleClient();
  //MQTT Calls
  #if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
    client.loop();
  #endif
 // =======================
 // Start rest of loop here
 // =======================
  unsigned long curTime = millis();
  if (autoMotion) {
    motionState = digitalRead(MOTION_PIN);
    if (motionState == HIGH)  {
      if (millis() > nextRunTime) {
        nextRunTime = millis() + motion_reset_time;
        rotateHead();
      } 
    } else if (motionState == LOW) {
      //Blink eyes
      if (autoBlink) {
        if (curTime - nextBlinkTime >= blink_time_max) {
          // Set random blink time between blink_time_min and blink_time_max
          int randBlinkOffset = random((blink_time_min - blink_time_max), 1);   
          nextBlinkTime = (curTime + randBlinkOffset);
           setEyeColor("black");
          delay(100);
          setEyeColor(eye_color_idle);
          delay(300);
        }        
      }
    }    
  }
}

// =============================
//  Misc procedures
// =============================
void rotateHead() {
  // Called by auto-motion. Not used if autoMotion false
  int pos;
  unsigned long timer = 0;
  //turn eyes blue (during motion)
  setEyeColor("blue");
  delay(250);
  if (rotate_dir == 0) {
    //rotate head to the right
    for (pos = curHeadPos; pos >= min_rotate; pos -= 5) {
      servo.write(pos);
      delay(step_delay); 
    }
  } else if (rotate_dir == 1) {
    //rotate head to the left
    for (pos = curHeadPos; pos <= max_rotate; pos +=5) {
      servo.write(pos);
      delay(step_delay);
    }
  }
  curHeadPos = pos;

  //turn eyes to active color
  setEyeColor(eye_color_active);
  //play audio file
  timer = millis();
  myDFPlayer.play(1);  //Play the first mp3
  //return head to center pos
  delay(head_reset_time);
  resetHeadPos();

  //reset eyes to idle color
  setEyeColor(eye_color_idle);
}

void resetHeadPos() {
  // Called by auto-motion. Not used if autoMotion false
  int pos;
  if (headPos < curHeadPos) {
    for (pos = curHeadPos; pos > headPos; pos -= 5) {
      servo.write(pos);
      delay(step_delay);
    }
  } else if (headPos > curHeadPos) {
    for (pos = curHeadPos; pos < headPos; pos += 5) {
      servo.write(pos);
      delay(step_delay);
    }
  }
  //set everything to the exact boot/start pos
  servo.write(headPos);
  curHeadPos = headPos;
}

void setEyeColor(String color) {
  if (color == "black") {
    digitalWrite(EYE_RED_PIN, LOW);
    digitalWrite(EYE_GREEN_PIN, LOW);
    digitalWrite(EYE_BLUE_PIN, LOW);
  } else if (color == "red") {
    digitalWrite(EYE_RED_PIN, HIGH);
    digitalWrite(EYE_GREEN_PIN, LOW);
    digitalWrite(EYE_BLUE_PIN, LOW);
  } else if (color == "green") {
    digitalWrite(EYE_RED_PIN, LOW);
    digitalWrite(EYE_GREEN_PIN, HIGH);
    digitalWrite(EYE_BLUE_PIN, LOW);
  } else if (color == "blue") {
    digitalWrite(EYE_RED_PIN, LOW);
    digitalWrite(EYE_GREEN_PIN, LOW);
    digitalWrite(EYE_BLUE_PIN, HIGH);
  } else if (color == "yellow") {
    digitalWrite(EYE_RED_PIN, HIGH);
    digitalWrite(EYE_GREEN_PIN, HIGH);
    digitalWrite(EYE_BLUE_PIN, LOW);
  } else if (color == "magenta") {
    digitalWrite(EYE_RED_PIN, HIGH);
    digitalWrite(EYE_GREEN_PIN, LOW);
    digitalWrite(EYE_BLUE_PIN, HIGH);
  } else if (color == "cyan") {
    digitalWrite(EYE_RED_PIN, LOW);
    digitalWrite(EYE_GREEN_PIN, HIGH);
    digitalWrite(EYE_BLUE_PIN, HIGH);
  } else if (color == "white") {
    digitalWrite(EYE_RED_PIN, HIGH);
    digitalWrite(EYE_GREEN_PIN, HIGH);
    digitalWrite(EYE_BLUE_PIN, HIGH);
  }
}

// ==================================
//  MQTT Issued Commands
// ==================================
void moveHead(int degree) { 
  // Valid values for listed servo are 0-180. Update below if using 360 servo.
  int finalPos = degree;
  if (finalPos > 180) {
    finalPos = 180; 
  } else if (finalPos < 0) {
    finalPos = 0;
  }
  
  int pos;

  if (headPos > finalPos) {
    for (pos = headPos; pos >= finalPos; pos -= 5) {
      servo.write(pos);
      delay(step_delay);
    }
    headPos = pos;
  } else if (headPos < finalPos) {
    for (pos = headPos; pos <= finalPos; pos += 5) {
      servo.write(pos);
      delay(step_delay);
    }
    headPos = pos;
  }
}

void blinkEyes(int blinkTime) {
  String prevColor = curEyeColor();
  int delayTime = blinkTime;
  if (delayTime < 0) {
    delayTime = 0;
  } else if (delayTime > 2000) {  //max of 2 seconds to prevent watchdog reset
    delayTime = 2000;
  }
  setEyeColor("black");
  delay(delayTime);
  setEyeColor(prevColor);
}

void setAudioVolume(int volume) {
  int newVol = 0;
  // Validate volume is in valid range 0-30
  if (volume > 30) {
    newVol = 30; 
  } else if (volume < 0) {
    newVol = 0;
  } else {
    newVol = volume;
  }
  myDFPlayer.volume(newVol);  
}

void playAudio(int trackNum) {
  myDFPlayer.play(trackNum);
}

String curEyeColor() {
  //Used by MQTT Blink to reset eye color
  String retVal = eye_color_idle;
  if ((digitalRead(EYE_RED_PIN)) && !(digitalRead(EYE_GREEN_PIN)) && !(digitalRead(EYE_BLUE_PIN))) {
    retVal = "red";
  } else if (!(digitalRead(EYE_RED_PIN)) && (digitalRead(EYE_GREEN_PIN)) && !(digitalRead(EYE_BLUE_PIN))) {
    retVal = "green";
  } else if (!(digitalRead(EYE_RED_PIN)) && !(digitalRead(EYE_GREEN_PIN)) && (digitalRead(EYE_BLUE_PIN))) {
    retVal = "blue";
  } else if ((digitalRead(EYE_RED_PIN)) && (digitalRead(EYE_GREEN_PIN)) && !(digitalRead(EYE_BLUE_PIN))) {
    retVal = "yellow";
  } else if ((digitalRead(EYE_RED_PIN)) && !(digitalRead(EYE_GREEN_PIN)) && (digitalRead(EYE_BLUE_PIN))) {
    retVal = "magenta";
  } else if (!(digitalRead(EYE_RED_PIN)) && (digitalRead(EYE_GREEN_PIN)) && (digitalRead(EYE_BLUE_PIN))) {
    retVal = "cyan";
  } else if ((digitalRead(EYE_RED_PIN)) && (digitalRead(EYE_GREEN_PIN)) && (digitalRead(EYE_BLUE_PIN))) {
    retVal = "white";
  }
  return retVal;
}
