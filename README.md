## Halloween Props

![Thumb_Github_Small](https://user-images.githubusercontent.com/55962781/138795294-b982c090-f13c-4605-89aa-c1dc6cc9a94c.jpg)

This repo contains the code and .stl files for the 3D printed parts to build two different NodeMCU/ESP8266-based motion-activated props with blinking LED eyes, rotating head and audio output. Alternative, the auto-motion activation can be disabled and they can be fully controlled via automation (like Home Assistant) via MQTT.

### To see these in action on Halloween night, along with an overview of the build, see this [YouTube video](https://youtu.be/elebVcLhq3s)

#### For full build details, wiring schematics and more, see the related blog article [Halloween Props with Moving Head and Sound](https://resinchemtech.blogspot.com/2021/11/halloween-props-with-moving-head-and.html)

While similar, there are slight difference in the two builds, features and code between the two, referred to as "Skull Head" (left in the above photo) and "Goblin Head" (right in the above photo).  Common features of both are:

* Motion activated or control via MQTT (MQTT optional, NOT required)
* Can operate "standalone", without WiFi (or MQTT) connectivity, when in autoMotion mode
* Rotating head
* LED "eyes" with 7 different available colors
* Change eye color based on motion (or MQTT)
* Eyes can blink at a random interval within a user-defined range
* Output of .mp3 audio files with a user-defined volume setting
* Over-the-air updates (after initial load via USB)
* All settings, options and other configuration via Credentials and Settings files with no need to change the main .ino file

The differences between the two builds are as follows:

#### Goblin Head
* Independent eye control.  Each eye can be a different color or blink independently.
* This comes at the expense of no Arduino serial monitor, as the RX/TX pins are repurposed for the second LED control

#### Skull Head
* Both eyes function together as one, with single control for color and blinking
* Arduino serial monintor still available for use if needed

Otherwise, the feature set between the two versions are identical.

### See the [wiki](https://github.com/Resinchem/Halloween-Props/wiki) for details on installation, settings and available MQTT topics
