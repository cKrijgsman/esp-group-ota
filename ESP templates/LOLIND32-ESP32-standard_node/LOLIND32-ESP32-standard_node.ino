
#include <WiFi.h> //WIFI setup
#include <ArduinoHttpClient.h> //http download
#include <AsyncUDP.h> //sending/receiving UDP packets (commands)
#include <ArduinoOTA.h> // only for InternalStorage

//network setup
#define SECRET_SSID   "Ziggo9794608"
#define SECRET_PASS   "r76Jfaxnhtje"
//server setup
#define SERVER_ID     "www.my-hostname.it"   // Set your correct hostname
//#define SERVER_PATH   "/update-v%d.bin"     // Set the URI to the .bin firmware
#define SERVER_PORT   80                    // Commonly 80 (HTTP) | 443 (HTTPS)


const short VERSION = 1;

const char MY_SSID[] = SECRET_SSID;
const char MY_PASS[] = SECRET_PASS;

WiFiClient    wifiClient;  // HTTP
int status = WL_IDLE_STATUS;

void handleSketchDownload() {
    const char* SERVER = SERVER_ID;  // hostname
    const unsigned short PORT = SERVER_PORT; // Port number
    const char* PATH = "/update-v%d.bin";       // URI to the .bin firmware
    const unsigned long CHECK_INTERVAL = 6000;  // Time interval between update checks (ms)

    // Time interval check
    static unsigned long previousMillis;
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis < CHECK_INTERVAL)
        return;
    previousMillis = currentMillis;

    HttpClient client(wifiClient, SERVER, PORT);  // HTTP

    char buff[32];
    snprintf(buff, sizeof(buff), PATH, VERSION + 1);

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
        return;
    }

    Serial.println("Sketch update apply and reset.");
    Serial.flush();
    InternalStorage.apply(); // this doesn't return
}

void setup() {

    Serial.begin(115200);
    while (!Serial);

    Serial.print("Sketch version ");
    Serial.println(VERSION);

    Serial.println("Initialize WiFi");
    WiFi.begin(MY_SSID, MY_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    //------------------------------------------ non OTA setup code --------------------------

    String builtString = String("victor|Birds|1");

    udp.broadcastTo(builtString.c_str(), 41234);
}

void loop() {
    // check for updates
    //handleSketchDownload();

    // add your normal loop code below ...
}