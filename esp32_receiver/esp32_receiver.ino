#include <FS.h>
#include <WiFiManager.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
bool responseReceived = false;
const uint16_t kRecvPin = 15; // GPIO 15 (G15) for IR Receiver
IRrecv irrecv(kRecvPin);
decode_results results;
const char* serverAddress = "djangobackend-seven.vercel.app"; // Replace with the IP address of your Django server
const int serverPort = 443; // Replace with the port of your Django server
const char* apiEndpoint = "/store-ir-codes/";
const int port = 80;
const int discoveryPort = 12345; // UDP broadcast port
const uint16_t kIrLedPin = 2; // Pin connected to the IR LED
IRsend irsend(kIrLedPin);

WiFiServer server(port);
WiFiUDP udp;

// Variables to store IR codes
unsigned long powerButtonCode = 0;
unsigned long muteButtonCode = 0;
unsigned long volumeUpButtonCode = 0;
unsigned long volumeDownButtonCode = 0;

// Global variables to store user ID and device ID
String globalDeviceID = ""; // Changed to String type
int globalUserID = 0;


void sendConfirmationResponse(WiFiClient& client, std::function<void()> callback) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println();
  client.println("{\"status\": \"ok\", \"message\": \"Device added confirmation received\"}");

  delay(5000) ;
  // Call the callback function after sending the confirmation response
  callback();
  delay(5000);

  // Send additional response message
  client.println("{\"additional_message\": \"Response after callback execution\"}");

  // Close the client connection after sending the response
  client.stop();
}

void handleAddDeviceRequest(WiFiClient& client) {
  // Read the HTTP request
  String request = client.readStringUntil('\r');
  Serial.println(request);

  // Parse JSON request
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, request);

  // Extract user ID and device ID
  globalUserID = doc["userId"].as<int>();
  globalDeviceID = doc["deviceId"].as<String>();

  // Send confirmation response
  sendConfirmationResponse(client, startIRCodeCollection);
}



void sendIRCodesToServer(String device_id, int user_id) {
  HTTPClient http;
  http.begin("https://" + String(serverAddress) + apiEndpoint);

  // Convert IR codes to hexadecimal strings
  char powerHex[9];
  char muteHex[9];
  char volumeUpHex[9];
  char volumeDownHex[9];
  sprintf(powerHex, "%08X", powerButtonCode);
  sprintf(muteHex, "%08X", muteButtonCode);
  sprintf(volumeUpHex, "%08X", volumeUpButtonCode);
  sprintf(volumeDownHex, "%08X", volumeDownButtonCode);

  // Construct the JSON payload
  String jsonData = "{\"power\":\"" + String(powerHex) + "\",\"mute\":\"" + String(muteHex) + "\",\"volume_up\":\"" + String(volumeUpHex) + "\",\"volume_down\":\"" + String(volumeDownHex) + "\",\"device_id\":\"" + device_id + "\",\"user_id\":" + String(user_id) + "}";

  // Set the Content-Type header to application/json
  http.addHeader("Content-Type", "application/json");

  // Make an HTTP POST request with JSON data
  int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error in HTTP POST request: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}









void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(kIrLedPin, OUTPUT);
  irrecv.enableIRIn();  // Start the receiver
  irsend.begin();

  Serial.println();
  WiFiManager wifiManager;

  if (!wifiManager.autoConnect("ESP32")) {
    Serial.println("Failed to connect to WiFi");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  Serial.println("Connected to WiFi");
  Serial.println("Local IP:");
  Serial.println(WiFi.localIP());

  // Initialize mDNS with the hostname "esp32"
  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up mDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }

  server.begin();
  udp.begin(discoveryPort);


  
}

void startIRCodeCollection() {
    int currentButton = 1; // 1 for Power, 2 for Mute, 3 for Volume Up, 4 for Volume Down
    Serial.println("Starting IR code collection. Please save the following IR codes before proceeding:");
    Serial.println("- Power code");
    Serial.println("- Mute code");
    Serial.println("- Volume Up code");
    Serial.println("- Volume Down code");
    Serial.println("Once you've saved all the codes, press the corresponding buttons on your remote.");
    // IR Receiver
    while (WiFi.status() == WL_CONNECTED) {
        if (irrecv.decode(&results)) {
            Serial.println("IR code received:");
            char hexCode[9]; // To store the hexadecimal representation (8 characters + null terminator)
            sprintf(hexCode, "%08X", results.value); // Convert to hexadecimal string
            Serial.println(hexCode);
            delay(1000);
            irrecv.resume();  // Receive the next value

            // Store IR codes based on their functionality
            if (currentButton == 1) {
                Serial.println("Power button code received.");
                powerButtonCode = results.value;
                currentButton = 2; // Move to the next button (Mute)
            } else if (currentButton == 2) {
                Serial.println("Mute button code received.");
                muteButtonCode = results.value;
                currentButton = 3; // Move to the next button (Volume Up)
            } else if (currentButton == 3) {
                Serial.println("Volume Up button code received.");
                volumeUpButtonCode = results.value;
                currentButton = 4; // Move to the next button (Volume Down)
            } else if (currentButton == 4) {
                Serial.println("Volume Down button code received.");
                volumeDownButtonCode = results.value;
                // All buttons captured, exit the loop
                break;
            }
        }
    }

    // Inform user about storing codes
    Serial.println("Storing IR codes...");

    // Send IR codes to the server
    sendIRCodesToServer(globalDeviceID, globalUserID);

    // Reset variables for the next round of IR code collection
    powerButtonCode = 0;
    muteButtonCode = 0;
    volumeUpButtonCode = 0;
    volumeDownButtonCode = 0;
    globalDeviceID = ""; // Reset to empty string
    globalUserID = 0;

    Serial.println("IR codes stored successfully.");
}









void loop() {
  // Handle HTTP requests from ESP32CAM device
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Empty line indicates end of HTTP request headers
            handleAddDeviceRequest(client);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }

  

  
// Handle UDP discovery requests
int packetSize = udp.parsePacket();
if (packetSize) {
  char packetBuffer[255];
  udp.read(packetBuffer, 255);
  Serial.print("Discovery request received from: ");
  Serial.println(udp.remoteIP());
  Serial.print("IR code received: ");
  Serial.println(packetBuffer);

  // Extract the IR code from the packetBuffer
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, packetBuffer);
  if (error) {
    Serial.print("Deserialization error: ");
    Serial.println(error.c_str());
    return;
  }

// Extract the IR code
String irCode = doc["code"].as<String>(); // Changed this line

Serial.print("IR code extracted: ");
Serial.println(irCode);

// Convert the IR code to numerical representation
unsigned long irCodeNum = strtoul(irCode.c_str(), nullptr, 16);

Serial.print("IR code in numerical format: 0x");
Serial.println(irCodeNum, HEX);

Serial.print("Now Sending code ");

// Handle the IR code here (send to TV, etc.)
// For example:
irsend.sendNEC(irCodeNum, 32);

Serial.println("Sent");
}



}



