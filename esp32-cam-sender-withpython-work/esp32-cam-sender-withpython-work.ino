#include <FS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <esp32cam.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
static const int RECEIVER_PORT = 80;
static const int DISCOVERY_PORT = 12345; // UDP broadcast port

WebServer server(80);
WiFiUDP udp;

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

volatile bool irSending = false;

void sendDiscoveryRequest() {
  udp.beginPacket(IPAddress(255, 255, 255, 255), DISCOVERY_PORT); // Broadcast address
  udp.print("DISCOVER");
  udp.endPacket();
}

void handleIRSend() {
  if (irSending) {
    server.send(400, "text/plain", "IR sending in progress.");
    return;
  }
  irSending = true;
  
  sendDiscoveryRequest();

  server.send(200, "text/plain", "Discovery request sent.");
}

void sendWirelessMessage(const String& message, IPAddress receiverIP) {
  WiFiClient client;
  if (client.connect(receiverIP, RECEIVER_PORT)) {
    Serial.println("Client connected");
    client.print(message);
    client.stop();
  } else {
    Serial.println("Connection to receiver failed.");
  }
  irSending = false;
}

void serveJpg() {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
  client.stop(); // Stop client connection after sending image
}

void handleJpgLo() {
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi() {
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleJpgMid() {
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}

void handleIRCode() {
  String irCode = server.arg("plain");
  Serial.print("Received IR code: ");
  Serial.println(irCode);  // Print the extracted IR code

  // Construct the UDP packet payload
  String udpPayload = irCode ;

  // Send the UDP packet
  udp.beginPacket(IPAddress(255, 255, 255, 255), DISCOVERY_PORT); // Broadcast address
  udp.write((const uint8_t*)udpPayload.c_str(), udpPayload.length()); // Convert String to const char* and specify the length
  udp.endPacket();

  Serial.println("UDP packet sent");

  server.send(200, "text/plain", "IR code received and forwarded via UDP.");
}



void setup() {
  Serial.begin(115200);
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
 
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFiManager wm;
  wm.autoConnect("ESP32-CAM");

   // Initialize mDNS with the hostname "esp32"
  if (!MDNS.begin("esp32cam")) {
    Serial.println("Error setting up mDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }

  server.on("/send-ir", HTTP_POST, handleIRCode);
  server.on("/send-ir", HTTP_GET, handleIRSend); // For testing
  server.on("/cam-lo.jpg", HTTP_GET, handleJpgLo);
  server.on("/cam-hi.jpg", HTTP_GET, handleJpgHi);
  server.on("/cam-mid.jpg", HTTP_GET, handleJpgMid);

  server.begin();

  udp.begin(DISCOVERY_PORT);
}

void loop() {
  server.handleClient();

  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    udp.read(packetBuffer, 255);
    packetBuffer[packetSize] = '\0'; // Null-terminate the string

    Serial.print("Discovery response: ");
    Serial.println(packetBuffer);
    
    // Assuming the response is the receiver's IP address
    IPAddress receiverIP;
    if (receiverIP.fromString(packetBuffer)) {
      Serial.print("Receiver IP address: ");
      Serial.println(receiverIP);
      
      
    } else {
      Serial.println("Invalid IP address received.");
    }
  }
}

