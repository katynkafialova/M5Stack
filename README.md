This software is designed for an M5Stack Core2 developer board. The same algorithm might work for other ESP32 developer boards but many of the lines are Core2 specific. Before compiling or uploading the main code to the board, make sure that all the used libraries and header files are downloaded and included. Double check that the uploading port on Arduino IDE is the same one the Core2 board is connected to. After uploading the software, make sure that the board is connected to Wi-Fi by following the instructions on the welcome screen. This step is important because all communication with your emergency contact is online.

The following, is a list of needed header files:

-          "time.h"
-          "audio.h"
-          "whatsapp.h"

The following, is a list of needed libraries:

-           <M5Core2.h>
-           <HTTPClient.h>
-           <WiFi.h>
-           <math.h>

The program is available for expansion, one of the main identified weaknesses of the program is that there isn't a variety of means to communicate with the user. Some added features to the software can include calling emergency contact, vibration and audio communication with the user. The audio.h header includes funcitons to deal with audio file but not reading them from an SD card. Once that is achieved, audio communication should be straight forward.
