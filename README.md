# Welcome, This is Light Stick Lux Flux Project

---
## What is Lux Flux?
Is an interactive Juggling props for Diabolo Sticks. It has the following features.

* Audio Rhythmic Light. For each sound peak could change to different color. Up to 10 configurable color sequence. 
*  Motion Sensitive. Light Stick can change color when moving in clockwise or anticlockwise. There is also a movement gesture to turn off lights.
*  There could be programmed light sequences from an external app or PC
* Led sticks could be sync with Wi-Fi. So there is only need to program one led stick and the other one will have the same color sequence.

## IMPORTANT

This repo works correctly with the following conditions 

+ ESP8266 RTOS SDK commit 9f37d6900f --> master branch
+ LWIP    commit ffd1059
+ mbedtls commit 9ef92c55
+ cJSON   branch v1.7.12

Hardware Design is made on KiCad and uses library from other repo. 
[Kicad-ESP8266](https://github.com/jdunmire/kicad-ESP8266)

----
## Developers
* Emilio Tonix : Tech Lead , hardware desing. 
* Ruth Solis Velasco : Ledsticks Sync Feature Developer
* Gisel Jimena Sanchez Salsalar: IMU calibration, movement gestures. 