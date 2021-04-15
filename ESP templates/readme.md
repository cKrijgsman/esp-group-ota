# ESP32 templates

In this folder resides two Arduino projects that are group-OTA enabled. In the folder `ESP32-Template`,
one can find the empty templated that only consists of the OTA code with no other behaviour.
The `komorebi_larva_code` contains the Komorebi Larava behaviour combined with the OTA code.
The version of the Komorebi behaviour in this code is from `09/01/21`.
For additional instructions and information on this code see https://dietervandoren.net/artefacts/komorebi-larva/

### prerequisites
#### ArduinoOTA
To compile the OTA code for the `esp32`, an additional library is required. This is the `ArduinoOTA` by `Arduino, Juraj Andrassy`.
For instructions on how to install this library, see the sections **Installation** and **ESP8266 and ESP32 support** on https://github.com/jandrassy/ArduinoOTA \
NOTE: only follow the instructions for the platform.local.txt, you can ignore the part about 
The library can also be downloaded using the arduino library manager.

If you still get an error during compiling about the `InternalStorage` that means that you missed a step in **ESP8266 and ESP32 support** on https://github.com/jandrassy/ArduinoOTA
Make sure that you have indeed deleted the original `ArduinoOTA` that can be found in the folder `<pathToArduinoData>\packages\esp32\hardware\esp32\1.0.4\libraries`

#### ArduinoHttpClient
You will also need to install the ArduinoHttpClient from the Arduino library manager. This is a package by Arduino.


### Use of the code
To get started with the OTA code, you will need to change a few variables.

`SECRET_SSID`: This is the name (SSID) of the Wi-Fi point the Arduino should connect to.

`SECRET_PASS`: This is the password of the Wi-Fi point to connect to.

The OTA code uses some flash memory to save states between reboots. Therefore, you need to indicate what section of the memory the OTA can use.\
`FLASH_START`: This is the start memory address of the OTA code. If your code doesn't use Flash memory, you don't need to change this.


### Getting the code upload ready

Once the code is done, and you want to upload it go to `Sketch > Export Compiled Binary` This will create a binary file inside your sketch directory.


### Using the template
The template in the `ESP32-Template` folder contains two files. The file named `OTA-code.ino` contains all the actual OTA code. If you want to use the OTA code in your own project then do the following

- Place the `OTA-code.ino` file in the same folder as you project.
- add all the imports that are in `ESP32-Template.ino` to you project.
- In the setup of you project call the OTA setup.

```
otaSetup("MySSID","MyPassword",4000);
```
The `4000` indicates the delay between Wi-Fi checks in milliseconds
