


//server setup (Only change if you know what you are doing)
//#define SERVER_PATH   "/api/file/%s"        // Set the URI to the .bin firmware
#define SERVER_PATH   "/api/file/"        // Set the URI to the .bin firmware
#define SERVER_PORT   3000                  // Commonly 80 (HTTP) | 443 (HTTPS)
#define UDP_BROADCAST_PORT  41234           // broadcast port number
#define UDP_LISNEN_PORT     41222           // command receiving port

/**
   This program uses 256 addresses of Flash memory.
   Indicate from what point it can start.
*/
#define FLASH_START 0
// The default implementation uses 256 addresses.
#define EEPROM_SIZE 256


// WIFI settings
String WIFI_SSID = "";
String WIFI_PASS = "";

/**
   These are the Default global values pleas don't change them.
*/
// Name max lenth is 64 chars
String NODE_NAME = "Caspar";
// Group ID. (max 255)
byte  NODE_GROUP = 1;
// Version name Max lenth is 191 chars
String NODE_VERSION = "V1.6";
TaskHandle_t OTPcontrol;
String UPDATE_FILE;
String serverIP;
bool ChekWiFiEnable = false;
bool downloading = false;
long wifiCheckTimeout = 4000;

WiFiClient wifiClient;  // HTTP
AsyncUDP udp;

// Helper function for reading the EEPROM
byte getEE(int pos, int def) {
  byte val = EEPROM.read(pos);
  if (val == 255) return def;
  return val;
}

void otaSetup(String ssid, String password, long wifiCheckDelay) {
 WIFI_SSID = ssid;
 WIFI_PASS = password;
  wifiCheckTimeout = wifiCheckDelay;

  
  // Start the flash manager
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to init EEPROM");
    delay(1000000);
  }

  xTaskCreatePinnedToCore(OTPcontrolcode, "OTPcontrol", 10000, NULL, 1, &OTPcontrol, 0);
  /* Task function,  name of task, Stack size of task, parameter of the task, priority of the task, Task handle to keep track of created task, pin task to core 0*/

  read_id();
}

/**
 * Read the Name and group from the ROM
 */
void read_id() {
  // Read name, version name and group from Flash
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
  // Read the version byte-by-byte from EEPROM
  String newVersion;
  for (int i = FLASH_START + 65; i < EEPROM_SIZE; i++) {
    byte readValue = getEE(i, 0);
    newVersion += String(char(readValue));
    if (readValue == 0) {
      break;
    }
  }
  NODE_VERSION = newVersion.c_str();
}

/**
 * Sets up the messaging listner.
 */
void OTPcontrolcode( void * pvParameters ) {
  for (;;) {
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
            // Max file name size is 191 chars due to ROM limitation.
            UPDATE_FILE = data_string.substring(3,packet.length());
            if (UPDATE_FILE.length() > 191) {
              Serial.println("File name was to large!");
              UPDATE_FILE = UPDATE_FILE.substring(0,191);
            }
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
    delay(wifiCheckTimeout);
  }
}

// Sends broadcast Message that will hopfully reach the server.
void broadcastToServer() {
  byte mac[6];
  WiFi.macAddress(mac);
  //udp broadcast on port
  String builtString =
    String(mac[5]) + ":" + String(mac[4]) + ":" + String(mac[3]) + ":" + String(mac[2]) + ":" + String(mac[1]) +
    ":" + String(mac[0]) + "|" + String(NODE_NAME) + "|" + String(NODE_GROUP) + "|" + String(NODE_VERSION);
  udp.broadcastTo(builtString.c_str(), UDP_BROADCAST_PORT);
}

// This function does the doawloading and running the new version of the sketch.
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

  client.setTimeout(30000);

//  char buff[64];
//  snprintf(buff, sizeof(buff), PATH, UPDATE_FILE.c_str());

  String url = String(PATH) + UPDATE_FILE;

  Serial.print("Check for update file ");
  Serial.println(url);

  // Make the GET request
  client.get(url.c_str());

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
  // writing byte-by-byte to EEPROM
  for (int i = FLASH_START + 65; i < FLASH_START + 65 + UPDATE_FILE.length(); i++) {
    EEPROM.write(i, byte(UPDATE_FILE.charAt(i - (FLASH_START + 65))));
  }

  // Add termination bit to the end.
  if (UPDATE_FILE.length() < 191) {
    EEPROM.write(FLASH_START + 65 + UPDATE_FILE.length(), byte(255));
  }

  EEPROM.commit();

  InternalStorage.apply(); // this doesn't return
}

// Helperfunction to check if the wifi is availible
bool CheckWiFiPossibility() {
  int n = WiFi.scanNetworks(1);
  Serial.print(n);
  Serial.println(" Networks found");
  if (n == 0) {
  } else {
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      if (WiFi.SSID(i) == WIFI_SSID.c_str()) { //enter the ssid which you want to search
        return true;
      } else {
        return false;
      }
    }
  }
}

// Helper function that will connect to a wifi network
bool ConnectToWiFi() {
  int i = 0;
  Serial.println("Initialize WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if (i == 10) {
      return false;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}
