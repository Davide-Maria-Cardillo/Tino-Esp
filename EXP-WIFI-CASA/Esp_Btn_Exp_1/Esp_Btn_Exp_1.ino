#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// ------------------- WIFI SETTINGS

// Rete Tino 2.4Ghz
IPAddress default_gateway(192, 168, 0, 1);   // IP Address of your WiFi Router (Gateway)
IPAddress default_subnet(255, 255, 255, 0);  // Subnet mask
IPAddress default_primaryDNS(8, 8, 8, 8);    // DNS 1
const IPAddress baseIP(192, 168, 0, 100);    //CAMBIO CON IP PC

// WIFI Name & pwd
#define WIFI_SSID "Physical Metaverse 2.4GHz"  // name of your wifi
#define WIFI_PSW "earthbound"         // password of your wifi

// UDP
#define MY_UDP_PORT 40616
#define IN_SIZE 255
#define OUT_SIZE 255

// COMMUNICATION
#define EMPTY_STRING ' '
#define DELIMITER '_'

// DEFAULT LED PINS
const uint8_t default_ledPinOn = 27;  // digital GPIO27
WiFiUDP UDP;
char in_packet[IN_SIZE];
char out_packet[OUT_SIZE];

// ESP STATIC IP -> ESP1 LED+BUTTON TASK
const IPAddress localIP(192, 168, 0, 5);  //1 LED+BUTTON TASK 1

// IP Addresses for other devices
const IPAddress ledIP(192, 168, 0, 6);   // ESP2 LED ONLY TASK
const IPAddress doorIP(192, 168, 0, 7);  // ESP3 DOOR TASK
const IPAddress pcIP(192, 168, 0, 100);  // PC for Game Manager

// LED
bool qr_code_read = false;

// ------------------- WIFI
void setup_wifi() {

  // Prevent connecting to wifi based on previous configuration
  WiFi.disconnect();

  // setup with STATIC IP
  bool wifi_configured = false;
  while (!wifi_configured) {
    Serial.println("CONNECTING");
    if (!WiFi.config(localIP, default_gateway, default_subnet, default_primaryDNS)) {
      Serial.println("[CONNECT TO WIFI] - failed to configure STATIC IP");
      delay(1000);
    } else {
      wifi_configured = true;
      Serial.println("[CONNECT TO WIFI] - configured STATIC IP");
    }
  }

  // set the ESP32 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSW);

  // Attempting connection to WiFi
  Serial.println("Trying to connect ...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(" .. not yet connected - current wifi status/connected status: ");
    Serial.print(WiFi.status());
    Serial.println("/");
    Serial.println(WL_CONNECTED);
  }

  // notify being connected to WiFi;
  Serial.print("Connected to Local Network - ESP IP: ");
  Serial.println(WiFi.localIP());

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Begin listening to UDP port
  UDP.begin(MY_UDP_PORT);
  Serial.print("UDP on:");
  Serial.println(MY_UDP_PORT);

  Serial.println("[CONNECT TO WIFI] - complete\n");
}

char read_UDP() {
  int packetSize = UDP.parsePacket();
  char msg = ' ';

  if (packetSize) {
    Serial.print("Received packet! Size: ");
    Serial.println(packetSize);
    int len = UDP.read(in_packet, IN_SIZE);  // the value is written in the BUFFER specificed as the first argument ("in_packet" in our case)

    msg = (char)in_packet[0];
    Serial.println(msg);
  }

  return msg;
}

void sendMsg(String msg, IPAddress ip, int port) {
  Serial.println("***********************");
  Serial.println(ip);
  Serial.println(port);
  Serial.println("***********************");
  UDP.beginPacket(ip, port);
  UDP.print(msg);
  UDP.endPacket();
}

// ------------------- VARIABLES

int buttonState = 0;
int buttonPin = 33;
int led_red_Pin = 12;
int led_green_Pin = 27;

// ------------------- MAIN

void setup() {

  // SERIAL
  Serial.begin(115200);
  delay(200);

  pinMode(buttonPin, INPUT);
  pinMode(led_green_Pin, OUTPUT);
  pinMode(led_red_Pin, OUTPUT);
  digitalWrite(led_green_Pin, LOW);
  digitalWrite(led_red_Pin, HIGH);  // Start with LED red ON

  Serial.println("SETUP --- --- --- BEGIN");

  setup_wifi();
  delay(200);

  Serial.println("SETUP --- --- --- COMPLETE");
  Serial.print("ESP1 IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // Leggi il messaggio UDP
  char msg = read_UDP();

  // Aggiorna lo stato di "leggi QR code"
  if (msg == '1') {
    qr_code_read = true;
  }

  // Leggi lo stato del bottone
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {
    if (qr_code_read) {
      // Se "leggi QR code" attivo, invia messaggio "2" a doorIP (ESP3)
      digitalWrite(led_red_Pin, LOW);
      digitalWrite(led_green_Pin, HIGH);
      sendMsg("2", doorIP, MY_UDP_PORT);
      Serial.println("Sent '2' to doorIP for positive feedback.");
    } else {
      // Se "leggi QR code" non attivo, invia messaggio "3" a doorIP (ESP3)
      sendMsg("3", doorIP, MY_UDP_PORT);
      Serial.println("Sent '3' to doorIP for negative feedback.");
      // Invia messaggio al PC per incrementare contatore
      sendMsg("5", pcIP, MY_UDP_PORT);
      Serial.println("Sent '5' to PC to update counter.");
    }
  }
}
