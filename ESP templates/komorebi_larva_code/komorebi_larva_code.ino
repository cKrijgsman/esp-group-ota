// Komorebi Larva kit code
// Dieter Vandoren & Matteo Marangoni
//
// Initial release 23/08/20
// Updated 09/01/21:
//   - ESP32 is now officialy supported by Mozzi
//   - optional 16bit external DAC (PT8211) for higher quality sound output
//
// For Wemos Lolin D32 board (ESP32 microcontroller)
//
// Requires Mozzi sound synthesis library https://sensorium.github.io/Mozzi/
//
// Light sensor input on pin 34 (circuit produces ADC input values between 0 and 2400)
//
// When using the board's internal DAC, audio output is on pin 26 (do not connect directly to speaker, amplifier is required)
// For external PT8211 DAC, see instructions
//

// START OF OTA section

// Following imports are for the OTA upload
#include <WiFi.h> //WIFI setup
#include <ArduinoHttpClient.h> //http download
#include <AsyncUDP.h> //sending/receiving UDP packets (commands)
#include <ArduinoOTA.h> // only for InternalStorage
#include <EEPROM.h>

//network setup
#define SECRET_SSID   "Ziggo9794608"
#define SECRET_PASS   "r76Jfaxnhtje"
//server setup
#define SERVER_PATH   "/api/file/%s"        // Set the URI to the .bin firmware
#define SERVER_PORT   3000                  // Commonly 80 (HTTP) | 443 (HTTPS)
#define UDP_BROADCAST_PORT  41234           // broadcast port number
#define UDP_LISNEN_PORT     41222           // command receiving port

/**
   This program uses 65 addresses of Flash memory.
   Indicate from what point it can start
*/
#define FLASH_START 0
// TODO figure out this number.
#define EEPROM_SIZE 256

// END OF OTA section

#define PRINT 0 // toggles debug data serial printing, printing can cause audio glitches

#include <MozziGuts.h>
#include<RollingAverage.h>
#include <Smooth.h>
#include <mozzi_rand.h>
#include <ADSR.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#define CONTROL_RATE 50 // Hz
float smoothness = 0.975f; // drone gain smoothing factor

// string A
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA2(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA3(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA4(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA5(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA6(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA7(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA8(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA9(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA10(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA11(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinA12(SIN2048_DATA);

Smooth <unsigned int> kSmoothA1(smoothness);
Smooth <unsigned int> kSmoothA2(smoothness);
Smooth <unsigned int> kSmoothA3(smoothness);
Smooth <unsigned int> kSmoothA4(smoothness);
Smooth <unsigned int> kSmoothA5(smoothness);
Smooth <unsigned int> kSmoothA6(smoothness);
Smooth <unsigned int> kSmoothA7(smoothness);
Smooth <unsigned int> kSmoothA8(smoothness);
Smooth <unsigned int> kSmoothA9(smoothness);
Smooth <unsigned int> kSmoothA10(smoothness);
Smooth <unsigned int> kSmoothA11(smoothness);
Smooth <unsigned int> kSmoothA12(smoothness);

// A pulse envelopes
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA1;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA2;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA3;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA4;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA5;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA6;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA7;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA8;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA9;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA10;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA11;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLA12;

// string B
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB2(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB3(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB4(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB5(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB6(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB7(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB8(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB9(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB10(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB11(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinB12(SIN2048_DATA);

Smooth <unsigned int> kSmoothB1(smoothness);
Smooth <unsigned int> kSmoothB2(smoothness);
Smooth <unsigned int> kSmoothB3(smoothness);
Smooth <unsigned int> kSmoothB4(smoothness);
Smooth <unsigned int> kSmoothB5(smoothness);
Smooth <unsigned int> kSmoothB6(smoothness);
Smooth <unsigned int> kSmoothB7(smoothness);
Smooth <unsigned int> kSmoothB8(smoothness);
Smooth <unsigned int> kSmoothB9(smoothness);
Smooth <unsigned int> kSmoothB10(smoothness);
Smooth <unsigned int> kSmoothB11(smoothness);
Smooth <unsigned int> kSmoothB12(smoothness);

// B pulse envelopes
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB1;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB2;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB3;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB4;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB5;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB6;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB7;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB8;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB9;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB10;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB11;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLB12;

// string C
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC2(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC3(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC4(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC5(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC6(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC7(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC8(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC9(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC10(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC11(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSinC12(SIN2048_DATA);

Smooth <unsigned int> kSmoothC1(smoothness);
Smooth <unsigned int> kSmoothC2(smoothness);
Smooth <unsigned int> kSmoothC3(smoothness);
Smooth <unsigned int> kSmoothC4(smoothness);
Smooth <unsigned int> kSmoothC5(smoothness);
Smooth <unsigned int> kSmoothC6(smoothness);
Smooth <unsigned int> kSmoothC7(smoothness);
Smooth <unsigned int> kSmoothC8(smoothness);
Smooth <unsigned int> kSmoothC9(smoothness);
Smooth <unsigned int> kSmoothC10(smoothness);
Smooth <unsigned int> kSmoothC11(smoothness);
Smooth <unsigned int> kSmoothC12(smoothness);

// C pulse envelopes
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC1;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC2;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC3;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC4;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC5;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC6;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC7;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC8;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC9;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC10;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC11;
ADSR <CONTROL_RATE, AUDIO_RATE> aEnvLC12;

int gain_max = 255;
int gain_min = 2;
const int osc_count = 12;

float fundamentals[] = {100., 125., 150., 166.666666, 200., 300.};

float freqs_A[] = {400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500};
byte gains_A[osc_count]; // 0-255 8bit for speed
byte smooth_gains_A[osc_count];
float detune_A = 2.; // Hz
bool retune_ready_A;
int drone_levels_A[osc_count]; // 0-1000
int pulseL_levels_A[osc_count];
int pulseM_levels_A[osc_count];
int pulseS_levels_A[osc_count];
int pulseL_treshold_A = 2750; // pulse L
int pulseM_treshold_A = 1300; // pulse M
int pulseS_treshold_A = 800; // pulse S

float freqs_B[] = {500, 625, 750, 875, 1000, 1125, 1250, 1375, 1500, 1625, 1750, 1875};
byte gains_B[osc_count]; // 0-255 8bit for speed
byte smooth_gains_B[osc_count];
float detune_B = 2.; // Hz
bool retune_ready_B;
int drone_levels_B[osc_count]; // 0-1000
int pulseL_levels_B[osc_count];
int pulseM_levels_B[osc_count];
int pulseS_levels_B[osc_count];
int pulseL_treshold_B = 3200; // pulse L
int pulseM_treshold_B = 1650; // pulse M
int pulseS_treshold_B = 780; // pulse S

float freqs_C[] = {750, 900, 1050, 1200, 1350, 1500, 1650, 1800, 1950, 2100, 2250, 2400};
byte gains_C[osc_count]; // 0-255 8bit for speed
byte smooth_gains_C[osc_count];
float detune_C = 2.; // Hz
bool retune_ready_C;
int drone_levels_C[osc_count]; // 0-1000
int pulseL_levels_C[osc_count];
int pulseM_levels_C[osc_count];
int pulseS_levels_C[osc_count];
int pulseL_treshold_C = 4590; // pulse L
int pulseM_treshold_C = 2270; // pulse M
int pulseS_treshold_C = 1230; // pulse S

RollingAverage<int, 32> kLight_rolling;
const int light_pin = 34;
int light_input;
int light_delta;
int delta_average;
int light_average;
int light_max = 1200;
boolean trigger;
float delta_scaler = 1.;

int trigger_treshold = 10; // sets sensitivity to light changes

RollingAverage<int, 64> kTriggers_rolling; // 64 seconds
int triggers_updates_counter;
int triggers_counter;
int triggers_average;

RollingAverage<int, 2048> kDelta_rolling; // 2048 update loops / 50 Hz = 41 seconds

int drone_start = 50;
int drone_range = 2000;
int drone_decrease_A = 4;
int drone_decrease_B = 5;
int drone_decrease_C = 6;
int drone_decrease_rate = 20; // ms
unsigned long decrease_time;
unsigned long drone_mute_count;
unsigned long drone_mute_threshold = 300;
byte mute_A;
byte mute_B;
byte mute_C;

// pulse envelopes levels and timing
byte attack_level_L = 230;
byte decay_level_L = 230;
unsigned int attack_ms_L = 2;
unsigned int decay_ms_L = 0;
unsigned int sustain_ms_L = 5;
unsigned int release_ms_L = 0;

byte attack_level_M = 80;
byte decay_level_M = 80;
unsigned int attack_ms_M = 2;
unsigned int decay_ms_M = 0;
unsigned int sustain_ms_M = 5;
unsigned int release_ms_M = 0;

byte attack_level_S = 10;
byte decay_level_S = 10;
unsigned int attack_ms_S = 2;
unsigned int decay_ms_S = 0;
unsigned int sustain_ms_S = 5;
unsigned int release_ms_S = 0;

int p_count; // serial printing counter


// START OF OTA VARIBLES
/**
   These are the Default values changing them has no effect.
*/
String NODE_NAME = "Caspar";
// Group ID. (max 255)
byte  NODE_GROUP = 1;
String NODE_VERSION = "V1.7";
TaskHandle_t OTPcontrol;
String UPDATE_FILE;
String serverIP;
const char MY_SSID[] = SECRET_SSID;
const char MY_PASS[] = SECRET_PASS;
bool downloading = false;


WiFiClient wifiClient;  // HTTP
int status = WL_IDLE_STATUS;
AsyncUDP udp;
// END OF OTA VARIBLES

// START OF OTA functions
byte getEE(int pos, int def) {
  byte val = EEPROM.read(pos);
  if (val == 255) return def;
  return val;
}

void read_id() {
  // Read name and group from Flash
  // reading name byte-by-byte from EEPROM
  String newName;
  for (int i = FLASH_START; i < FLASH_START + 64; i++) {
    byte readValue = getEE(i, 0);
    newName += String(char(readValue));
    if (readValue == 0) {
      break;
    }
  }
  NODE_NAME = newName.c_str();
  // reading group (default is 0)
  NODE_GROUP = getEE(FLASH_START + 64, 0);
}

void OTASetup() {

  if (PRINT && !EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to init EEPROM");
    delay(1000000);
  }

  xTaskCreatePinnedToCore(OTPcontrolcode, "OTPcontrol", 10000, NULL, 1, &OTPcontrol, 0);
  /* Task function,  name of task, Stack size of task, parameter of the task, priority of the task, Task handle to keep track of created task, pin task to core 0*/
  read_id();
}

void OTPcontrolcode( void * pvParameters ) {

  for(;;) {
    if ((WiFi.status() != WL_CONNECTED) && (CheckWiFiPossibility() == true)) {
      ConnectToWiFi();
      //timerAlarmEnable(timer1);
      if (udp.listen(UDP_LISNEN_PORT)) {

        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());

        udp.onPacket([](AsyncUDPPacket packet) {
          Serial.write(packet.data(), packet.length());
          Serial.println();

          char* DATA = (char*) packet.data();
          String data_string = String(DATA);
          // Go command
          if (data_string.substring(0, 3) == "go|") {
            UPDATE_FILE = data_string.substring(3);
            Serial.println(UPDATE_FILE);
            handleSketchDownload();
          }
          if (downloading) {
            return;
          }
          // Marco Polo
          if (data_string.substring(0, 5) == "Marco") {
            // Get server IP from message.
            IPAddress remote = packet.remoteIP();
            serverIP = String(remote[0]) + String(".") + \
                       String(remote[1]) + String(".") + \
                       String(remote[2]) + String(".") + \
                       String(remote[3]);
            Serial.print("Server IP: ");
            Serial.println(serverIP);
            udp.broadcastTo("Polo", UDP_BROADCAST_PORT);
          }
          // Identify
          if (data_string.substring(0, 9) == "Identify!") {
            broadcastToServer();
          }
          // Set Name
          if (data_string.substring(0, 3) == "nm|") {
            String n = data_string.substring(3, 67);
            Serial.println(n);
            // writing byte-by-byte to EEPROM
            for (int i = FLASH_START; i < FLASH_START + n.length(); i++) {
              EEPROM.write(i, byte(n.charAt(i - FLASH_START)));
            }

            // Add termination bit to the end.
            if (n.length() < 64) {
              EEPROM.write(n.length(), byte(255));
            }

            EEPROM.commit();
            read_id();
            broadcastToServer();
          }
          // Set group
          if (data_string.substring(0, 2) == "g|") {
            int group = data_string.substring(2, 5).toInt();
            EEPROM.write(FLASH_START + 64, byte(group));
            EEPROM.commit();
            read_id();
            broadcastToServer();
          }
          packet.flush();

        });
      }
      broadcastToServer();
    }
    delay(4000);
  }
}

void broadcastToServer() {
  byte mac[6];
  WiFi.macAddress(mac);
  //udp broadcast on port
  String builtString =
    String(mac[5]) + ":" + String(mac[4]) + ":" + String(mac[3]) + ":" + String(mac[2]) + ":" + String(mac[1]) +
    ":" + String(mac[0]) + "|" + String(NODE_NAME) + "|" + String(NODE_GROUP) + "|" + String(NODE_VERSION);
  udp.broadcastTo(builtString.c_str(), UDP_BROADCAST_PORT);
}

void handleSketchDownload() {
  const char *SERVER = serverIP.c_str();  // hostname
  const unsigned short PORT = SERVER_PORT; // Port number
  const char *PATH = SERVER_PATH;       // URI to the .bin firmware
  const unsigned long CHECK_INTERVAL = 6000;  // Time interval between update checks (ms)

  // Time interval check
  static unsigned long previousMillis;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < CHECK_INTERVAL)
    return;
  previousMillis = currentMillis;

  HttpClient client(wifiClient, SERVER, PORT);  // HTTP

  char buff[32];
  snprintf(buff, sizeof(buff), PATH, UPDATE_FILE);

  // Make the GET request
  client.get(buff);

  int statusCode = client.responseStatusCode();
  if (statusCode != 200) {
    client.stop();
    return;
  }
  downloading = true;

  long length = client.contentLength();
  if (length == HttpClient::kNoContentLengthHeader) {
    client.stop();
    if (PRINT) {
      Serial.println("Server didn't provide Content-length header. Can't continue with update.");
    }
    return;
  }
  if (PRINT) {
    Serial.print("Server returned update file of size ");
    Serial.print(length);
    Serial.println(" bytes");
  }

  if (!InternalStorage.open(length)) {
    client.stop();
    if (PRINT) {
      Serial.println("There is not enough space to store the update. Can't continue with update.");
    }
    return;
  }
  byte b;
  while (length > 0) {
    if (!client.readBytes(&b, 1)) // reading a byte with timeout
      break;
    InternalStorage.write(b);
    length--;
  }

  InternalStorage.close();
  client.stop();

  if (length > 0) {
    if (PRINT) {
      Serial.print("Timeout downloading update file at ");
      Serial.print(length);
      Serial.println(" bytes. Can't continue with update.");
    }
    downloading = false;
    return;
  }
  if (PRINT) {
    Serial.println("Sketch update apply and reset.");
    Serial.flush();
  }
  InternalStorage.apply(); // this doesn't return
}

bool CheckWiFiPossibility() {
  int n = WiFi.scanNetworks(1);
  if (PRINT) {
    Serial.println("scan done");
  }
  if (n == 0) {
    if (PRINT) {
      Serial.println("no networks found");
    }
  } else {
    if (PRINT) {
      Serial.print(n);
      Serial.println(" networks found");
    }
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      if (WiFi.SSID(i) == MY_SSID) { //enter the ssid which you want to search
        if (PRINT) {
          Serial.println("The network you are looking for is available");
        }
        return true;
      } else {
        return false;
      }
    }
  }
  if (PRINT) {
    Serial.println("");
  }
}

bool ConnectToWiFi() {
  int i = 0;
  if (PRINT) {
    Serial.println("Initialize WiFi");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(MY_SSID, MY_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (PRINT) {
      Serial.print(".");
    }
    i++;
    if (i == 5) {
      return false;
    }
  }

  if (PRINT) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  return true;
}

void updateControl() {

  decrease_levels();

  trigger = false;

  // ADC input maxes out at 2400, normalize and clip to 0-1200 range (divides by 12)
  light_input = map(analogRead(light_pin), 0, 2400, 0, light_max);
  light_input = min(light_input, light_max);

  light_average = kLight_rolling.next(light_input); // rolling average of light input
  light_delta = abs(light_input - light_average) * delta_scaler; // calculate delta of current reading and average

  // light change trigger?
  if (light_delta >= trigger_treshold) {
    trigger = true;

    for (int i = 0; i < osc_count; i++) { // bump up levels
      if (light_input >= i * light_max / 12 && light_input < (i + 1)*light_max / 12) { // within harmonic range?

        drone_levels_A[i] = min(drone_levels_A[i] + light_delta, 1000); // increase and clip level
        pulseL_levels_A[i] += light_delta;
        pulseM_levels_A[i] += light_delta;
        pulseS_levels_A[i] += light_delta;

        drone_levels_B[i] = min(drone_levels_B[i] + light_delta, 1000);
        pulseL_levels_B[i] += light_delta;
        pulseM_levels_B[i] += light_delta;
        pulseS_levels_B[i] += light_delta;

        drone_levels_C[i] = min(drone_levels_C[i] + light_delta, 1000);
        pulseL_levels_C[i] += light_delta;
        pulseM_levels_C[i] += light_delta;
        pulseS_levels_C[i] += light_delta;
      }
    }
  }

  // average delta values and adapt delta_scaler when low
  delta_average = kDelta_rolling.next(light_delta);
  triggers_updates_counter++;

  if (delta_average <= trigger_treshold) {
    delta_scaler = map(delta_average, trigger_treshold, 0, 1000, 5000);
    delta_scaler /= 1000.;
  } else { // delta_average > trigger_treshold
    delta_scaler = constrain(map(delta_average, trigger_treshold * 5, trigger_treshold * 10, 1000, 100), 100, 1000);
    delta_scaler /= 1000.;
  }

  // calculate gains
  for (int i = 0; i < osc_count; i++) {

    // drone string A
    if (drone_levels_A[i] < drone_start) {
      gains_A[i] = 0;
    } else {
      gains_A[i] = map(drone_levels_A[i], drone_start, 1000, 0, gain_max);
      long exp_scale = (gains_A[i] * gains_A[i] * gains_A[i]) >> 16;
      gains_A[i] = (byte)exp_scale;
    }

    // drone string B
    if (drone_levels_B[i] < drone_start) {
      gains_B[i] = 0;
    } else {
      gains_B[i] = map(drone_levels_B[i], drone_start, 1000, 0, gain_max);
      long exp_scale = (gains_B[i] * gains_B[i] * gains_B[i]) >> 16;
      gains_B[i] = (byte)exp_scale;
    }

    // drone string C
    if (drone_levels_C[i] < drone_start) {
      gains_C[i] = 0;
    } else {
      gains_C[i] = map(drone_levels_C[i], drone_start, 1000, 0, gain_max);
      long exp_scale = (gains_C[i] * gains_C[i] * gains_C[i]) >> 16;
      gains_C[i] = (byte)exp_scale;
    }
  }

  // keep lowest harmonics playing at minimum level, masks poor fade out resolution
  gains_A[0] = (byte)max((int)gains_A[0], gain_min);
  gains_B[0] = (byte)max((int)gains_B[0], gain_min);
  gains_C[0] = (byte)max((int)gains_C[0], gain_min);

  smoothDroneGains();

  // fade out and mute drones, retune strings when unmuting
  long smooth_gains_sum_A = 0;
  long smooth_gains_sum_B = 0;
  long smooth_gains_sum_C = 0;

  for (int i = 0; i < osc_count; i++) {
    smooth_gains_sum_A += smooth_gains_A[i];
    smooth_gains_sum_B += smooth_gains_B[i];
    smooth_gains_sum_C += smooth_gains_C[i];
  }

  if (smooth_gains_sum_A == gain_min) { // drone at minimum?
    retune_ready_A = 1;
  }
  if (retune_ready_A && smooth_gains_sum_A > gain_min) {
    retune_A(fundamentals[random(6)], random(1, 5)); // retune_X(fundamental freq, starting harmonic multiplier)
    retune_ready_A = 0;
    mute_A = 1;
  }

  if (smooth_gains_sum_B == gain_min) { // drone at minimum?
    retune_ready_B = 1;
  }
  if (retune_ready_B && smooth_gains_sum_B > gain_min) {
    retune_B(fundamentals[random(6)], random(1, 5)); // retune_X(fundamental freq, starting harmonic multiplier)
    retune_ready_B = 0;
    mute_B = 1;
  }
  if (smooth_gains_sum_C == gain_min) { // drone at minimum?
    retune_ready_C = 1;
  }
  if (retune_ready_C && smooth_gains_sum_C > gain_min) {
    retune_C(fundamentals[random(6)], random(1, 5)); // retune_X(fundamental freq, starting harmonic multiplier)
    retune_ready_C = 0;
    mute_C = 1;
  }

  if (smooth_gains_sum_A == gain_min && smooth_gains_sum_B == gain_min && smooth_gains_sum_C == gain_min) {
    drone_mute_count++;
    if (drone_mute_count >= drone_mute_threshold) {
      mute_A = 0; mute_B = 0; mute_C = 0;
      drone_mute_count = 0;
    }
  }

  triggerPulses();

  updateEnvelopes();

  // debug printing
  p_count += 1;
  if (PRINT && p_count == 1) { // skip updates to prevent glitches, 1 = print every update loop
    p_count = 0;

    Serial.print(light_input);
    Serial.print("\t");
    Serial.print(trigger);
    Serial.print("\t");
    Serial.print(delta_average);
    Serial.print("\t");
    Serial.print(delta_scaler);
    Serial.println();
  }
}

int updateAudio() {

  int oscA1 = aSinA1.next(); // get next sine oscillator values
  int oscA2 = aSinA2.next();
  int oscA3 = aSinA3.next();
  int oscA4 = aSinA4.next();
  int oscA5 = aSinA5.next();
  int oscA6 = aSinA6.next();
  int oscA7 = aSinA7.next();
  int oscA8 = aSinA8.next();
  int oscA9 = aSinA9.next();
  int oscA10 = aSinA10.next();
  int oscA11 = aSinA11.next();
  int oscA12 = aSinA12.next();

  long drone_A = // drone A submix
    oscA1 * smooth_gains_A[0] +
    oscA2 * smooth_gains_A[1] +
    oscA3 * smooth_gains_A[2] +
    oscA4 * smooth_gains_A[3] +
    oscA5 * smooth_gains_A[4] +
    oscA6 * smooth_gains_A[5] +
    oscA7 * smooth_gains_A[6] +
    oscA8 * smooth_gains_A[7] +
    oscA9 * smooth_gains_A[8] +
    oscA10 * smooth_gains_A[9] +
    oscA11 * smooth_gains_A[10] +
    oscA12 * smooth_gains_A[11] ;

  long pulses_L_A = // pulses A submix
    oscA1 * (int)aEnvLA1.next() +
    oscA2 * (int)aEnvLA2.next() +
    oscA3 * (int)aEnvLA3.next() +
    oscA4 * (int)aEnvLA4.next() +
    oscA5 * (int)aEnvLA5.next() +
    oscA6 * (int)aEnvLA6.next() +
    oscA7 * (int)aEnvLA7.next() +
    oscA8 * (int)aEnvLA8.next() +
    oscA9 * (int)aEnvLA9.next() +
    oscA10 * (int)aEnvLA10.next() +
    oscA11 * (int)aEnvLA11.next() +
    oscA12 * (int)aEnvLA12.next();

  int oscB1 = aSinB1.next();
  int oscB2 = aSinB2.next();
  int oscB3 = aSinB3.next();
  int oscB4 = aSinB4.next();
  int oscB5 = aSinB5.next();
  int oscB6 = aSinB6.next();
  int oscB7 = aSinB7.next();
  int oscB8 = aSinB8.next();
  int oscB9 = aSinB9.next();
  int oscB10 = aSinB10.next();
  int oscB11 = aSinB11.next();
  int oscB12 = aSinB12.next();

  long drone_B =
    oscB1 * smooth_gains_B[0] +
    oscB2 * smooth_gains_B[1] +
    oscB3 * smooth_gains_B[2] +
    oscB4 * smooth_gains_B[3] +
    oscB5 * smooth_gains_B[4] +
    oscB6 * smooth_gains_B[5] +
    oscB7 * smooth_gains_B[6] +
    oscB8 * smooth_gains_B[7] +
    oscB9 * smooth_gains_B[8] +
    oscB10 * smooth_gains_B[9] +
    oscB11 * smooth_gains_B[10] +
    oscB12 * smooth_gains_B[11] ;

  long pulses_L_B =
    oscB1 * (int)aEnvLB1.next() +
    oscB2 * (int)aEnvLB2.next() +
    oscB3 * (int)aEnvLB3.next() +
    oscB4 * (int)aEnvLB4.next() +
    oscB5 * (int)aEnvLB5.next() +
    oscB6 * (int)aEnvLB6.next() +
    oscB7 * (int)aEnvLB7.next() +
    oscB8 * (int)aEnvLB8.next() +
    oscB9 * (int)aEnvLB9.next() +
    oscB10 * (int)aEnvLB10.next() +
    oscB11 * (int)aEnvLB11.next() +
    oscB12 * (int)aEnvLB12.next();

  int oscC1 = aSinC1.next();
  int oscC2 = aSinC2.next();
  int oscC3 = aSinC3.next();
  int oscC4 = aSinC4.next();
  int oscC5 = aSinC5.next();
  int oscC6 = aSinC6.next();
  int oscC7 = aSinC7.next();
  int oscC8 = aSinC8.next();
  int oscC9 = aSinC9.next();
  int oscC10 = aSinC10.next();
  int oscC11 = aSinC11.next();
  int oscC12 = aSinC12.next();

  long drone_C =
    oscC1 * smooth_gains_C[0] +
    oscC2 * smooth_gains_C[1] +
    oscC3 * smooth_gains_C[2] +
    oscC4 * smooth_gains_C[3] +
    oscC5 * smooth_gains_C[4] +
    oscC6 * smooth_gains_C[5] +
    oscC7 * smooth_gains_C[6] +
    oscC8 * smooth_gains_C[7] +
    oscC9 * smooth_gains_C[8] +
    oscC10 * smooth_gains_C[9] +
    oscC11 * smooth_gains_C[10] +
    oscC12 * smooth_gains_C[11] ;

  long pulses_L_C =
    oscC1 * (int)aEnvLC1.next() +
    oscC2 * (int)aEnvLC2.next() +
    oscC3 * (int)aEnvLC3.next() +
    oscC4 * (int)aEnvLC4.next() +
    oscC5 * (int)aEnvLC5.next() +
    oscC6 * (int)aEnvLC6.next() +
    oscC7 * (int)aEnvLC7.next() +
    oscC8 * (int)aEnvLC8.next() +
    oscC9 * (int)aEnvLC9.next() +
    oscC10 * (int)aEnvLC10.next() +
    oscC11 * (int)aEnvLC11.next() +
    oscC12 * (int)aEnvLC12.next();

  if (ESP32_AUDIO_OUT_MODE == INTERNAL_DAC) { // 8 bit internal DAC
    long mix =
      (drone_A >> 7) * mute_A + (pulses_L_A >> 5) +
      (drone_B >> 7) * mute_B + (pulses_L_B >> 5) +
      (drone_C >> 7) * mute_C + (pulses_L_C >> 5) ; // set mix levels by bitshifting (= fast division by powers of 2)
    return (int)(mix >> 6); // bitshift result down to 8 bit DAC output resolution and amplifier input tolerance

  } else {

    if (ESP32_AUDIO_OUT_MODE == PT8211_DAC) { // 16 bit external PT8211 DAC
      long mix =
        (drone_A >> 2) * mute_A + (pulses_L_A ) +
        (drone_B >> 2) * mute_B + (pulses_L_B ) +
        (drone_C >> 2) * mute_C + (pulses_L_C ) ; // set mix levels by bitshifting (= fast division by powers of 2)
      return (int)(mix >> 4); // bitshift result down to 16 bit DAC output resolution and amplifier input tolerance
    }
  }
}

void setup() {
  if (PRINT)Serial.begin(9600);

  // OTA SETUP
  OTASetup();
  
  randSeed();
  startMozzi(CONTROL_RATE);
  setFreqsA(); setFreqsB(); setFreqsC();
  setEnvelopes();
  retune_A(fundamentals[random(6)], random(1, 5));
  retune_B(fundamentals[random(6)], random(1, 5));
  retune_C(fundamentals[random(6)], random(1, 5));
  mute_A = 1; mute_B = 1; mute_C = 1;
}

void loop() {
  audioHook();
}

void decrease_levels() {
  unsigned int time_now = millis();
  if (time_now - decrease_time > drone_decrease_rate) {
    for (int i = 0; i < osc_count; i++) {
      drone_levels_A[i] = max(drone_levels_A[i] - drone_decrease_A, 0); // clip minimum at 0
      drone_levels_B[i] = max(drone_levels_B[i] - drone_decrease_B, 0);
      drone_levels_C[i] = max(drone_levels_C[i] - drone_decrease_C, 0);
    }
    decrease_time = time_now;
  }
}

void setFreqsA() {
  aSinA1.setFreq(freqs_A[0] + map(random(1001), 0., 1000., -1. * detune_A, detune_A)); // add random detune between -detune_A and detune_A
  aSinA2.setFreq(freqs_A[1] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA3.setFreq(freqs_A[2] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA4.setFreq(freqs_A[3] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA5.setFreq(freqs_A[4] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA6.setFreq(freqs_A[5] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA7.setFreq(freqs_A[6] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA8.setFreq(freqs_A[7] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA9.setFreq(freqs_A[8] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA10.setFreq(freqs_A[9] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA11.setFreq(freqs_A[10] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
  aSinA12.setFreq(freqs_A[11] + map(random(1001), 0., 1000., -1. * detune_A, detune_A));
}

void setFreqsB() {
  aSinB1.setFreq(freqs_B[0] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB2.setFreq(freqs_B[1] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB3.setFreq(freqs_B[2] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB4.setFreq(freqs_B[3] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB5.setFreq(freqs_B[4] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB6.setFreq(freqs_B[5] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB7.setFreq(freqs_B[6] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB8.setFreq(freqs_B[7] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB9.setFreq(freqs_B[8] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB10.setFreq(freqs_B[9] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB11.setFreq(freqs_B[10] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
  aSinB12.setFreq(freqs_B[11] + map(random(1001), 0., 1000., -1. * detune_B, detune_B));
}

void setFreqsC() {
  aSinC1.setFreq(freqs_C[0] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC2.setFreq(freqs_C[1] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC3.setFreq(freqs_C[2] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC4.setFreq(freqs_C[3] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC5.setFreq(freqs_C[4] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC6.setFreq(freqs_C[5] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC7.setFreq(freqs_C[6] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC8.setFreq(freqs_C[7] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC9.setFreq(freqs_C[8] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC10.setFreq(freqs_C[9] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC11.setFreq(freqs_C[10] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
  aSinC12.setFreq(freqs_C[11] + map(random(1001), 0., 1000., -1. * detune_C, detune_C));
}

void setEnvelopes() {
  aEnvLA1.setADLevels(attack_level_L, decay_level_L); aEnvLA1.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA2.setADLevels(attack_level_L, decay_level_L); aEnvLA2.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA3.setADLevels(attack_level_L, decay_level_L); aEnvLA3.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA4.setADLevels(attack_level_L, decay_level_L); aEnvLA4.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA5.setADLevels(attack_level_L, decay_level_L); aEnvLA5.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA6.setADLevels(attack_level_L, decay_level_L); aEnvLA6.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA7.setADLevels(attack_level_L, decay_level_L); aEnvLA7.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA8.setADLevels(attack_level_L, decay_level_L); aEnvLA8.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA9.setADLevels(attack_level_L, decay_level_L); aEnvLA9.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA10.setADLevels(attack_level_L, decay_level_L); aEnvLA10.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA11.setADLevels(attack_level_L, decay_level_L); aEnvLA11.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLA12.setADLevels(attack_level_L, decay_level_L); aEnvLA12.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);

  aEnvLB1.setADLevels(attack_level_L, decay_level_L); aEnvLB1.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB2.setADLevels(attack_level_L, decay_level_L); aEnvLB2.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB3.setADLevels(attack_level_L, decay_level_L); aEnvLB3.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB4.setADLevels(attack_level_L, decay_level_L); aEnvLB4.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB5.setADLevels(attack_level_L, decay_level_L); aEnvLB5.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB6.setADLevels(attack_level_L, decay_level_L); aEnvLB6.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB7.setADLevels(attack_level_L, decay_level_L); aEnvLB7.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB8.setADLevels(attack_level_L, decay_level_L); aEnvLB8.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB9.setADLevels(attack_level_L, decay_level_L); aEnvLB9.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB10.setADLevels(attack_level_L, decay_level_L); aEnvLB10.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB11.setADLevels(attack_level_L, decay_level_L); aEnvLB11.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLB12.setADLevels(attack_level_L, decay_level_L); aEnvLB12.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);

  aEnvLC1.setADLevels(attack_level_L, decay_level_L); aEnvLC1.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC2.setADLevels(attack_level_L, decay_level_L); aEnvLC2.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC3.setADLevels(attack_level_L, decay_level_L); aEnvLC3.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC4.setADLevels(attack_level_L, decay_level_L); aEnvLC4.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC5.setADLevels(attack_level_L, decay_level_L); aEnvLC5.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC6.setADLevels(attack_level_L, decay_level_L); aEnvLC6.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC7.setADLevels(attack_level_L, decay_level_L); aEnvLC7.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC8.setADLevels(attack_level_L, decay_level_L); aEnvLC8.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC9.setADLevels(attack_level_L, decay_level_L); aEnvLC9.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC10.setADLevels(attack_level_L, decay_level_L); aEnvLC10.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC11.setADLevels(attack_level_L, decay_level_L); aEnvLC11.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);
  aEnvLC12.setADLevels(attack_level_L, decay_level_L); aEnvLC12.setTimes(attack_ms_L, decay_ms_L, sustain_ms_L, release_ms_L);

}

void updateEnvelopes() {
  aEnvLA1.update();
  aEnvLA2.update();
  aEnvLA3.update();
  aEnvLA4.update();
  aEnvLA5.update();
  aEnvLA6.update();
  aEnvLA7.update();
  aEnvLA8.update();
  aEnvLA9.update();
  aEnvLA10.update();
  aEnvLA11.update();
  aEnvLA12.update();

  aEnvLB1.update();
  aEnvLB2.update();
  aEnvLB3.update();
  aEnvLB4.update();
  aEnvLB5.update();
  aEnvLB6.update();
  aEnvLB7.update();
  aEnvLB8.update();
  aEnvLB9.update();
  aEnvLB10.update();
  aEnvLB11.update();
  aEnvLB12.update();

  aEnvLC1.update();
  aEnvLC2.update();
  aEnvLC3.update();
  aEnvLC4.update();
  aEnvLC5.update();
  aEnvLC6.update();
  aEnvLC7.update();
  aEnvLC8.update();
  aEnvLC9.update();
  aEnvLC10.update();
  aEnvLC11.update();
  aEnvLC12.update();
}

void triggerPulses() {
  for (int i = 0; i < osc_count; i++) {
    if (pulseL_levels_A[i] >= pulseL_treshold_A) { // A fire pulse L?
      pulseL_levels_A[i] = 0;
      switch (i) {
        case 0: aEnvLA1.setADLevels(attack_level_L, decay_level_L); aEnvLA1.noteOn(); break;
        case 1: aEnvLA2.setADLevels(attack_level_L, decay_level_L); aEnvLA2.noteOn(); break;
        case 2: aEnvLA3.setADLevels(attack_level_L, decay_level_L); aEnvLA3.noteOn(); break;
        case 3: aEnvLA4.setADLevels(attack_level_L, decay_level_L); aEnvLA4.noteOn(); break;
        case 4: aEnvLA5.setADLevels(attack_level_L, decay_level_L); aEnvLA5.noteOn(); break;
        case 5: aEnvLA6.setADLevels(attack_level_L, decay_level_L); aEnvLA6.noteOn(); break;
        case 6: aEnvLA7.setADLevels(attack_level_L, decay_level_L); aEnvLA7.noteOn(); break;
        case 7: aEnvLA8.setADLevels(attack_level_L, decay_level_L); aEnvLA8.noteOn(); break;
        case 8: aEnvLA9.setADLevels(attack_level_L, decay_level_L); aEnvLA9.noteOn(); break;
        case 9: aEnvLA10.setADLevels(attack_level_L, decay_level_L); aEnvLA10.noteOn(); break;
        case 10: aEnvLA11.setADLevels(attack_level_L, decay_level_L); aEnvLA11.noteOn(); break;
        case 11: aEnvLA12.setADLevels(attack_level_L, decay_level_L); aEnvLA12.noteOn(); break;
      }
    }
    if (pulseM_levels_A[i] >= pulseM_treshold_A) { // A fire pulse M?
      pulseM_levels_A[i] = 0;
      switch (i) {
        case 0: aEnvLA1.setADLevels(attack_level_M, decay_level_M); aEnvLA1.noteOn(); break;
        case 1: aEnvLA2.setADLevels(attack_level_M, decay_level_M); aEnvLA2.noteOn(); break;
        case 2: aEnvLA3.setADLevels(attack_level_M, decay_level_M); aEnvLA3.noteOn(); break;
        case 3: aEnvLA4.setADLevels(attack_level_M, decay_level_M); aEnvLA4.noteOn(); break;
        case 4: aEnvLA5.setADLevels(attack_level_M, decay_level_M); aEnvLA5.noteOn(); break;
        case 5: aEnvLA6.setADLevels(attack_level_M, decay_level_M); aEnvLA6.noteOn(); break;
        case 6: aEnvLA7.setADLevels(attack_level_M, decay_level_M); aEnvLA7.noteOn(); break;
        case 7: aEnvLA8.setADLevels(attack_level_M, decay_level_M); aEnvLA8.noteOn(); break;
        case 8: aEnvLA9.setADLevels(attack_level_M, decay_level_M); aEnvLA9.noteOn(); break;
        case 9: aEnvLA10.setADLevels(attack_level_M, decay_level_M); aEnvLA10.noteOn(); break;
        case 10: aEnvLA11.setADLevels(attack_level_M, decay_level_M); aEnvLA11.noteOn(); break;
        case 11: aEnvLA12.setADLevels(attack_level_M, decay_level_M); aEnvLA12.noteOn(); break;
      }
    }
    if (pulseS_levels_A[i] >= pulseS_treshold_A) { // A fire pulse S?
      pulseS_levels_A[i] = 0;
      switch (i) {
        case 0: aEnvLA1.setADLevels(attack_level_S, decay_level_S); aEnvLA1.noteOn(); break;
        case 1: aEnvLA2.setADLevels(attack_level_S, decay_level_S); aEnvLA2.noteOn(); break;
        case 2: aEnvLA3.setADLevels(attack_level_S, decay_level_S); aEnvLA3.noteOn(); break;
        case 3: aEnvLA4.setADLevels(attack_level_S, decay_level_S); aEnvLA4.noteOn(); break;
        case 4: aEnvLA5.setADLevels(attack_level_S, decay_level_S); aEnvLA5.noteOn(); break;
        case 5: aEnvLA6.setADLevels(attack_level_S, decay_level_S); aEnvLA6.noteOn(); break;
        case 6: aEnvLA7.setADLevels(attack_level_S, decay_level_S); aEnvLA7.noteOn(); break;
        case 7: aEnvLA8.setADLevels(attack_level_S, decay_level_S); aEnvLA8.noteOn(); break;
        case 8: aEnvLA9.setADLevels(attack_level_S, decay_level_S); aEnvLA9.noteOn(); break;
        case 9: aEnvLA10.setADLevels(attack_level_S, decay_level_S); aEnvLA10.noteOn(); break;
        case 10: aEnvLA11.setADLevels(attack_level_S, decay_level_S); aEnvLA11.noteOn(); break;
        case 11: aEnvLA12.setADLevels(attack_level_S, decay_level_S); aEnvLA12.noteOn(); break;
      }
    }
    if (pulseL_levels_B[i] >= pulseL_treshold_B) { // B fire pulse L?
      pulseL_levels_B[i] = 0;
      switch (i) {
        case 0: aEnvLB1.setADLevels(attack_level_L, decay_level_L); aEnvLB1.noteOn(); break;
        case 1: aEnvLB2.setADLevels(attack_level_L, decay_level_L); aEnvLB2.noteOn(); break;
        case 2: aEnvLB3.setADLevels(attack_level_L, decay_level_L); aEnvLB3.noteOn(); break;
        case 3: aEnvLB4.setADLevels(attack_level_L, decay_level_L); aEnvLB4.noteOn(); break;
        case 4: aEnvLB5.setADLevels(attack_level_L, decay_level_L); aEnvLB5.noteOn(); break;
        case 5: aEnvLB6.setADLevels(attack_level_L, decay_level_L); aEnvLB6.noteOn(); break;
        case 6: aEnvLB7.setADLevels(attack_level_L, decay_level_L); aEnvLB7.noteOn(); break;
        case 7: aEnvLB8.setADLevels(attack_level_L, decay_level_L); aEnvLB8.noteOn(); break;
        case 8: aEnvLB9.setADLevels(attack_level_L, decay_level_L); aEnvLB9.noteOn(); break;
        case 9: aEnvLB10.setADLevels(attack_level_L, decay_level_L); aEnvLB10.noteOn(); break;
        case 10: aEnvLB11.setADLevels(attack_level_L, decay_level_L); aEnvLB11.noteOn(); break;
        case 11: aEnvLB12.setADLevels(attack_level_L, decay_level_L); aEnvLB12.noteOn(); break;
      }
    }
    if (pulseM_levels_B[i] >= pulseM_treshold_B) { // B fire pulse M?
      pulseM_levels_B[i] = 0;
      switch (i) {
        case 0: aEnvLB1.setADLevels(attack_level_L, decay_level_L); aEnvLB1.noteOn(); break;
        case 1: aEnvLB2.setADLevels(attack_level_M, decay_level_M); aEnvLB2.noteOn(); break;
        case 2: aEnvLB3.setADLevels(attack_level_M, decay_level_M); aEnvLB3.noteOn(); break;
        case 3: aEnvLB4.setADLevels(attack_level_M, decay_level_M); aEnvLB4.noteOn(); break;
        case 4: aEnvLB5.setADLevels(attack_level_M, decay_level_M); aEnvLB5.noteOn(); break;
        case 5: aEnvLB6.setADLevels(attack_level_M, decay_level_M); aEnvLB6.noteOn(); break;
        case 6: aEnvLB7.setADLevels(attack_level_M, decay_level_M); aEnvLB7.noteOn(); break;
        case 7: aEnvLB8.setADLevels(attack_level_M, decay_level_M); aEnvLB8.noteOn(); break;
        case 8: aEnvLB9.setADLevels(attack_level_M, decay_level_M); aEnvLB9.noteOn(); break;
        case 9: aEnvLB10.setADLevels(attack_level_M, decay_level_M); aEnvLB10.noteOn(); break;
        case 10: aEnvLB11.setADLevels(attack_level_M, decay_level_M); aEnvLB11.noteOn(); break;
        case 11: aEnvLB12.setADLevels(attack_level_M, decay_level_M); aEnvLB12.noteOn(); break;
      }
    }
    if (pulseS_levels_B[i] >= pulseS_treshold_B) { // B fire pulse S?
      pulseS_levels_B[i] = 0;
      switch (i) {
        case 0: aEnvLB1.setADLevels(attack_level_S, decay_level_S); aEnvLB1.noteOn(); break;
        case 1: aEnvLB2.setADLevels(attack_level_S, decay_level_S); aEnvLB2.noteOn(); break;
        case 2: aEnvLB3.setADLevels(attack_level_S, decay_level_S); aEnvLB3.noteOn(); break;
        case 3: aEnvLB4.setADLevels(attack_level_S, decay_level_S); aEnvLB4.noteOn(); break;
        case 4: aEnvLB5.setADLevels(attack_level_S, decay_level_S); aEnvLB5.noteOn(); break;
        case 5: aEnvLB6.setADLevels(attack_level_S, decay_level_S); aEnvLB6.noteOn(); break;
        case 6: aEnvLB7.setADLevels(attack_level_S, decay_level_S); aEnvLB7.noteOn(); break;
        case 7: aEnvLB8.setADLevels(attack_level_S, decay_level_S); aEnvLB8.noteOn(); break;
        case 8: aEnvLB9.setADLevels(attack_level_S, decay_level_S); aEnvLB9.noteOn(); break;
        case 9: aEnvLB10.setADLevels(attack_level_S, decay_level_S); aEnvLB10.noteOn(); break;
        case 10: aEnvLB11.setADLevels(attack_level_S, decay_level_S); aEnvLB11.noteOn(); break;
        case 11: aEnvLB12.setADLevels(attack_level_S, decay_level_S); aEnvLB12.noteOn(); break;
      }
    }
    if (pulseL_levels_C[i] >= pulseL_treshold_C) { // C fire pulse L?
      pulseL_levels_C[i] = 0;
      switch (i) {
        case 0: aEnvLC1.setADLevels(attack_level_L, decay_level_L); aEnvLC1.noteOn(); break;
        case 1: aEnvLC2.setADLevels(attack_level_L, decay_level_L); aEnvLC2.noteOn(); break;
        case 2: aEnvLC3.setADLevels(attack_level_L, decay_level_L); aEnvLC3.noteOn(); break;
        case 3: aEnvLC4.setADLevels(attack_level_L, decay_level_L); aEnvLC4.noteOn(); break;
        case 4: aEnvLC5.setADLevels(attack_level_L, decay_level_L); aEnvLC5.noteOn(); break;
        case 5: aEnvLC6.setADLevels(attack_level_L, decay_level_L); aEnvLC6.noteOn(); break;
        case 6: aEnvLC7.setADLevels(attack_level_L, decay_level_L); aEnvLC7.noteOn(); break;
        case 7: aEnvLC8.setADLevels(attack_level_L, decay_level_L); aEnvLC8.noteOn(); break;
        case 8: aEnvLC9.setADLevels(attack_level_L, decay_level_L); aEnvLC9.noteOn(); break;
        case 9: aEnvLC10.setADLevels(attack_level_L, decay_level_L); aEnvLC10.noteOn(); break;
        case 10: aEnvLC11.setADLevels(attack_level_L, decay_level_L); aEnvLC11.noteOn(); break;
        case 11: aEnvLC12.setADLevels(attack_level_L, decay_level_L); aEnvLC12.noteOn(); break;
      }
    }
    if (pulseM_levels_C[i] >= pulseM_treshold_C) { // C fire pulse M?
      pulseM_levels_C[i] = 0;
      switch (i) {
        case 0: aEnvLC1.setADLevels(attack_level_M, decay_level_M); aEnvLC1.noteOn(); break;
        case 1: aEnvLC2.setADLevels(attack_level_M, decay_level_M); aEnvLC2.noteOn(); break;
        case 2: aEnvLC3.setADLevels(attack_level_M, decay_level_M); aEnvLC3.noteOn(); break;
        case 3: aEnvLC4.setADLevels(attack_level_M, decay_level_M); aEnvLC4.noteOn(); break;
        case 4: aEnvLC5.setADLevels(attack_level_M, decay_level_M); aEnvLC5.noteOn(); break;
        case 5: aEnvLC6.setADLevels(attack_level_M, decay_level_M); aEnvLC6.noteOn(); break;
        case 6: aEnvLC7.setADLevels(attack_level_M, decay_level_M); aEnvLC7.noteOn(); break;
        case 7: aEnvLC8.setADLevels(attack_level_M, decay_level_M); aEnvLC8.noteOn(); break;
        case 8: aEnvLC9.setADLevels(attack_level_M, decay_level_M); aEnvLC9.noteOn(); break;
        case 9: aEnvLC10.setADLevels(attack_level_M, decay_level_M); aEnvLC10.noteOn(); break;
        case 10: aEnvLC11.setADLevels(attack_level_M, decay_level_M); aEnvLC11.noteOn(); break;
        case 11: aEnvLC12.setADLevels(attack_level_M, decay_level_M); aEnvLC12.noteOn(); break;
      }
    }
    if (pulseS_levels_B[i] >= pulseS_treshold_C) { // C fire pulse S?
      pulseS_levels_C[i] = 0;
      switch (i) {
        case 0: aEnvLC1.setADLevels(attack_level_S, decay_level_S); aEnvLC1.noteOn(); break;
        case 1: aEnvLC2.setADLevels(attack_level_S, decay_level_S); aEnvLC2.noteOn(); break;
        case 2: aEnvLC3.setADLevels(attack_level_S, decay_level_S); aEnvLC3.noteOn(); break;
        case 3: aEnvLC4.setADLevels(attack_level_S, decay_level_S); aEnvLC4.noteOn(); break;
        case 4: aEnvLC5.setADLevels(attack_level_S, decay_level_S); aEnvLC5.noteOn(); break;
        case 5: aEnvLC6.setADLevels(attack_level_S, decay_level_S); aEnvLC6.noteOn(); break;
        case 6: aEnvLC7.setADLevels(attack_level_S, decay_level_S); aEnvLC7.noteOn(); break;
        case 7: aEnvLC8.setADLevels(attack_level_S, decay_level_S); aEnvLC8.noteOn(); break;
        case 8: aEnvLC9.setADLevels(attack_level_S, decay_level_S); aEnvLC9.noteOn(); break;
        case 9: aEnvLC10.setADLevels(attack_level_S, decay_level_S); aEnvLC10.noteOn(); break;
        case 10: aEnvLC11.setADLevels(attack_level_S, decay_level_S); aEnvLC11.noteOn(); break;
        case 11: aEnvLC12.setADLevels(attack_level_S, decay_level_S); aEnvLC12.noteOn(); break;
      }
    }
  }
}

void retune_A(float fund, float startHarm) {
  for (int i = 0; i < osc_count; i++) {
    freqs_A[i] = fund * (startHarm + i);
  }
  setFreqsA();
}

void retune_B(float fund, float startHarm) {
  for (int i = 0; i < osc_count; i++) {
    freqs_B[i] = fund * (startHarm + i);
  }
  setFreqsB();
}

void retune_C(float fund, float startHarm) {
  for (int i = 0; i < osc_count; i++) {
    freqs_C[i] = fund * (startHarm + i);
  }
  setFreqsC();
}

void smoothDroneGains() {
  // smooth drone gain values
  smooth_gains_A[0] = kSmoothA1.next(gains_A[0]);
  smooth_gains_A[1] = kSmoothA2.next(gains_A[1]);
  smooth_gains_A[2] = kSmoothA3.next(gains_A[2]);
  smooth_gains_A[3] = kSmoothA4.next(gains_A[3]);
  smooth_gains_A[4] = kSmoothA5.next(gains_A[4]);
  smooth_gains_A[5] = kSmoothA6.next(gains_A[5]);
  smooth_gains_A[6] = kSmoothA7.next(gains_A[6]);
  smooth_gains_A[7] = kSmoothA8.next(gains_A[7]);
  smooth_gains_A[8] = kSmoothA9.next(gains_A[8]);
  smooth_gains_A[9] = kSmoothA10.next(gains_A[9]);
  smooth_gains_A[10] = kSmoothA11.next(gains_A[10]);
  smooth_gains_A[11] = kSmoothA12.next(gains_A[11]);

  smooth_gains_B[0] = kSmoothB1.next(gains_B[0]);
  smooth_gains_B[1] = kSmoothB2.next(gains_B[1]);
  smooth_gains_B[2] = kSmoothB3.next(gains_B[2]);
  smooth_gains_B[3] = kSmoothB4.next(gains_B[3]);
  smooth_gains_B[4] = kSmoothB5.next(gains_B[4]);
  smooth_gains_B[5] = kSmoothB6.next(gains_B[5]);
  smooth_gains_B[6] = kSmoothB7.next(gains_B[6]);
  smooth_gains_B[7] = kSmoothB8.next(gains_B[7]);
  smooth_gains_B[8] = kSmoothB9.next(gains_B[8]);
  smooth_gains_B[9] = kSmoothB10.next(gains_B[9]);
  smooth_gains_B[10] = kSmoothB11.next(gains_B[10]);
  smooth_gains_B[11] = kSmoothB12.next(gains_B[11]);

  smooth_gains_C[0] = kSmoothC1.next(gains_C[0]);
  smooth_gains_C[1] = kSmoothC2.next(gains_C[1]);
  smooth_gains_C[2] = kSmoothC3.next(gains_C[2]);
  smooth_gains_C[3] = kSmoothC4.next(gains_C[3]);
  smooth_gains_C[4] = kSmoothC5.next(gains_C[4]);
  smooth_gains_C[5] = kSmoothC6.next(gains_C[5]);
  smooth_gains_C[6] = kSmoothC7.next(gains_C[6]);
  smooth_gains_C[7] = kSmoothC8.next(gains_C[7]);
  smooth_gains_C[8] = kSmoothC9.next(gains_C[8]);
  smooth_gains_C[9] = kSmoothC10.next(gains_C[9]);
  smooth_gains_C[10] = kSmoothC11.next(gains_C[10]);
  smooth_gains_C[11] = kSmoothC12.next(gains_C[11]);
}
