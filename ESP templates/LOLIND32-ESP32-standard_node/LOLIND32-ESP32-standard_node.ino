
#include <WiFi.h> //WIFI setup
#include <ArduinoHttpClient.h> //http download
#include <AsyncUDP.h> //sending/receiving UDP packets (commands)
#include <ArduinoOTA.h> // only for InternalStorage

#define SCRIPT_VERSION "1"
//network setup
#define SECRET_SSID   "Ziggo9794608"
#define SECRET_PASS   "r76Jfaxnhtje"
//server setup
#define SERVER_ID     "www.my-hostname.it"  // Set your correct hostname
#define SERVER_PATH   "/file/%s"            // Set the URI to the .bin firmware
#define SERVER_PORT   80                    // Commonly 80 (HTTP) | 443 (HTTPS)
#define UDP_BROADCAST_PORT  41234           // broadcast port number
#define UDP_LISNEN_PORT     41222           // command receiving port

char *NODE_NAME = "berta";
char *NODE_GROUP = "koeien";
char *NODE_VERSION = SCRIPT_VERSION;

const char MY_SSID[] = SECRET_SSID;
const char MY_PASS[] = SECRET_PASS;

bool ChekWiFiEnable = false;

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

void setup() {

    Serial.begin(115200);
    while (!Serial);

    Serial.print("Sketch version ");
    Serial.println(NODE_VERSION);

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



    //------------------------------------------ non OTA setup code --------------------------

}

void loop() {

    if (ChekWiFiEnable == true) {
        if ((WiFi.status() != WL_CONNECTED) && (CheckWiFiPossibility() == true)) {
            ConnectToWiFi();
            //timerAlarmEnable(timer1);
            if(udp.listen(1234)) {
                Serial.print("UDP Listening on IP: ");
                Serial.println(WiFi.localIP());
                udp.onPacket([](AsyncUDPPacket packet) {
                    Serial.print("UDP Packet Type: ");
                    Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
                    Serial.print(", From: ");
                    Serial.print(packet.remoteIP());
                    Serial.print(":");
                    Serial.print(packet.remotePort());
                    Serial.print(", To: ");
                    Serial.print(packet.localIP());
                    Serial.print(":");
                    Serial.print(packet.localPort());
                    Serial.print(", Length: ");
                    Serial.print(packet.length());
                    Serial.print(", Data: ");
                    Serial.write(packet.data(), packet.length());
                    Serial.println();
                    //reply to the client
                    packet.printf("Got %u bytes of data", packet.length());
                });
            }

            broadcastToServer();
        }
    }
    Serial.print(" \n . ");
    delay(1000);
    //handleSketchDownload();
    //broadcastToServer();


    // add your normal loop code below ...
}

void onPacketCallBack(AsyncUDPPacket packet) {

    Serial.print("UDP Packet Type: ");
    Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
    Serial.print(", From: ");
    Serial.print(packet.remoteIP());
    Serial.print(":");
    Serial.print(packet.remotePort());
    Serial.print(", To: ");
    Serial.print(packet.localIP());
    Serial.print(":");
    Serial.print(packet.localPort());
    Serial.print(", Length: ");
    Serial.print(packet.length());
    Serial.print(", Data: ");
    Serial.write(packet.data(), packet.length());
    packet.flush();


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
    const char *SERVER = SERVER_ID;  // hostname
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
    snprintf(buff, sizeof(buff), PATH);

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

