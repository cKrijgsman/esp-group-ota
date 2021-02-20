

#include <WiFi.h> //WIFI setup
#include <ArduinoHttpClient.h> //http download
#include <AsyncUDP.h> //sending/receiving UDP packets (commands)
#include <ArduinoOTA.h> // only for InternalStorage
#include <EEPROM.h>

//network setup
#define SECRET_SSID   "Ziggo9794608"
#define SECRET_PASS   "r76Jfaxnhtje"
//server setup
#define SERVER_PATH   "/api/file/%s"            // Set the URI to the .bin firmware
#define SERVER_PORT   3000                    // Commonly 80 (HTTP) | 443 (HTTPS)
#define UDP_BROADCAST_PORT  41234           // broadcast port number
#define UDP_LISNEN_PORT     41222           // command receiving port

/**
   This program uses 65 addresses of Flash memory.
   Indicate from what point it can start
*/
#define FLASH_START 0
// TODO figure out this number.
#define EEPROM_SIZE 256


/**
   These are the Default values changing them has no effect.
*/
String NODE_NAME = "Caspar";
byte  NODE_GROUP = 1;
// Version of the code. (max 255)
String NODE_VERSION = "V1.0";

String UPDATE_FILE;
String serverIP;

const char MY_SSID[] = SECRET_SSID;
const char MY_PASS[] = SECRET_PASS;

bool ChekWiFiEnable = false;
bool downloading = false;

//creating a timer
hw_timer_t *timer0 = NULL;
//hw_timer_t * timer1 = NULL;

WiFiClient wifiClient;  // HTTP
int status = WL_IDLE_STATUS;
AsyncUDP udp;

//TimerChekWiFi interrupt
void IRAM_ATTR TimerChekWiFi() {
  ChekWiFiEnable = true;
}

byte getEE(int pos, int def){
  byte val = EEPROM.read(pos);
  if (val == 255) return def;
  return val;
}

void read_id() {
  // Read name and group from Flash
  // reading name byte-by-byte from EEPROM
  String newName; 
  for (int i = FLASH_START; i < FLASH_START + 64; i++) {
    byte readValue = getEE(i,0);
    newName += String(char(readValue));
    Serial.print(String(char(readValue)));
    if (readValue == 0) {
      break;
    }
  }
  Serial.println("");
  NODE_NAME = newName.c_str();
  // reading group (default is 0)
  NODE_GROUP = getEE(FLASH_START + 64,0);
}

void setup() {

  Serial.begin(115200);
  while (!Serial);


  // Start the flash manager
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to init EEPROM");
    delay(1000000);
  }

  //config TimerChekWiFi//
  /* Use 1st timer of 4 */
  /* 1 tick takes 1/(80MHZ/40000) = 500us so we set divider 40000 and count up */
  timer0 = timerBegin(0, 40000, true);

  /* Attach TimerChekWiFi function to timer0 */
  timerAttachInterrupt(timer0, &TimerChekWiFi, true);

  /* Set alarm to call onTimer function every 5 min 1 tick is 500us
    => 1 second is 2000us */
  /* Repeat the alarm (third parameter) */
  timerAlarmWrite(timer0, 10000, true);

  /* Start an alarm */
  timerAlarmEnable(timer0);
  Serial.println("start TimerChekWiFi");

  read_id();
  //------------------------------------------ non OTA setup code --------------------------

}

void loop() {

  if (ChekWiFiEnable == true) {
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
  }

  Serial.print(" \n . ");
  delay(1000);

  // add your normal loop code below ...

}

void broadcastToServer() {
  byte mac[6];
  WiFi.macAddress(mac);
  // print MAC address
  /*Serial.print("MAC: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);*/
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

  Serial.print("Check for update file ");
  Serial.println(buff);

  // Make the GET request
  client.get(buff);

  int statusCode = client.responseStatusCode();
  Serial.print("Update status code: ");
  Serial.println(statusCode);
  if (statusCode != 200) {
    client.stop();
    return;
  }
  downloading = true;

  long length = client.contentLength();
  if (length == HttpClient::kNoContentLengthHeader) {
    client.stop();
    Serial.println("Server didn't provide Content-length header. Can't continue with update.");
    return;
  }
  Serial.print("Server returned update file of size ");
  Serial.print(length);
  Serial.println(" bytes");

  if (!InternalStorage.open(length)) {
    client.stop();
    Serial.println("There is not enough space to store the update. Can't continue with update.");
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
    Serial.print("Timeout downloading update file at ");
    Serial.print(length);
    Serial.println(" bytes. Can't continue with update.");
    downloading = false;
    return;
  }

  Serial.println("Sketch update apply and reset.");
  Serial.flush();
  InternalStorage.apply(); // this doesn't return
}

bool CheckWiFiPossibility() {
  int n = WiFi.scanNetworks(1);
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      if (WiFi.SSID(i) == MY_SSID) { //enter the ssid which you want to search
        Serial.println("The network you are looking for is available");
        return true;
      } else {
        return false;
      }
    }
  }
  Serial.println("");

}

bool ConnectToWiFi() {
  int i = 0;
  Serial.println("Initialize WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(MY_SSID, MY_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if (i == 5) {
      return false;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}
