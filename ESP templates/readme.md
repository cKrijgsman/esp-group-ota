#ESP32 templates

In this folder resides two Arduino projects that are group-OTA enabled. In the folder `LOLIND32-ESP32-standard_node`,
one can find the empty templated that only consists of the OTA code with no other behaviour.
The `komorebi_larva_code` contains the Komorebi Larava behaviour combined with the OTA code.
The version of the Komorebi behaviour in this code is from `09/01/21`.
For additional instructions and information on this code see https://dietervandoren.net/artefacts/komorebi-larva/

### prerequisites
To compile the OTA code for the `esp32`, an additional library is required. This is the `ArduinoOTA` by `Arduino, Juraj Andrassy`.
For instructions on how to install this library, see the sections **Installation** and **ESP8266 and ESP32 support** on https://github.com/jandrassy/ArduinoOTA

### Use of the code
To get started with the OTA code, you will need to change a few variables.

`SECRET_SSID`: This is the name (SSID) of the wifi point the Arduino should connect to.

`SECRET_PASS`: This is the password of the wifi point to connect to.

The OTA code uses some flash memory to save states between reboots. Therefore you need to indicate what section of the memory the OTA can use.\
`FLASH_START`: This is the start memory address of the OTA code. If your code doesn't use Flash memory, you don't need to change this.
