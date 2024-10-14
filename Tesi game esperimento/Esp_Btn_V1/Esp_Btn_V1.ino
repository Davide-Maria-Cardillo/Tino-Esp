#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// ------------------- WIFI SETTINGS

// Physical Metaverse 2.4GHz
IPAddress default_gateway(192, 168, 0, 1);   // IP Address of your WiFi Router (Gateway)
IPAddress default_subnet(255, 255, 255, 0);  // Subnet mask
IPAddress default_primaryDNS(8, 8, 8, 8);    // DNS 1
const IPAddress baseIP(192, 168, 0, 103);    //CAMBIO CON IP PC

// WIFI Name & pwd
#define WIFI_SSID "Rete Tino 2.4Ghz"  // name of your wifi
#define WIFI_PSW "Earthbound"                  // password of your wifi

// UDP
#define MY_UDP_PORT 40616
#define IN_SIZE 255
#define OUT_SIZE 255

// COMMUNICATION
#define EMPTY_STRING ' '
#define DELIMITER '_'

// DEFAULT LED PINS
const uint8_t default_ledPinOn = 27;        // digital GPIO27
const uint8_t default_ledPinWiFi = 14;      // digital GPIO14
const uint8_t default_ledPinFunction = 12;  // digital GPIO12
WiFiUDP UDP;
char in_packet[IN_SIZE];
char out_packet[OUT_SIZE];

// ESP STATIC IP -> uno diverso per ogni esp
const IPAddress localIP(192, 168, 0, 5);  //Physical Metaverse 2.4GHz


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
      // blink fast to signal failed STATIC IP setup

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

    // blink the WIFI led while awaiting for connection once (per loop)
    // NB the "blink_led" method contains the delay

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

  // turn WIFI led ON: WIFI connection successful

  Serial.println("[CONNECT TO WIFI] - complete\n");
}

char read_UDP() {
  int packetSize = UDP.parsePacket();

  char msg;

  if (packetSize) {
    Serial.print("Received packet! Size: ");
    Serial.println(packetSize);
    int len = UDP.read(in_packet, 10);  // the value is written in the BUFFER specificed as the first argument ("in_packet" in our case)

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
int buttonPin = 33;      //22 N_7, N_3 | 27 - N_2 | 33-N_5 N_1 N_4, N_6
int ledbtnPin = 5;       //pin 5- N_2
int led_blue_Pin = 26;   //26 - N_5 N_1 N_4, N_6 | 14 - N_7, N_3
int led_green_Pin = 27;  //27 - N_5 N_1 N_4, N_6 | 12 - N_7, N_3
int led_red_Pin = 12;    //12 - N_5 N_1 N_4, N_6 | 13 - N_7, N_3

bool btn_high = false;
int counter;

// ------------------- MAIN

void setup() {

  // SERIAL
  Serial.begin(115200);
  delay(200);


  pinMode(buttonPin, INPUT);
  pinMode(led_blue_Pin, OUTPUT);
  pinMode(led_green_Pin, OUTPUT);
  pinMode(led_red_Pin, OUTPUT);
  pinMode(ledbtnPin, OUTPUT);
  digitalWrite(ledbtnPin, LOW);

  Serial.println("SETUP --- --- --- BEGIN");

  setup_wifi();
  delay(200);

  Serial.println("SETUP --- --- --- COMPLETE");
}

void loop() {

  // read the state of the pushbutton value
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH
  if (buttonState == HIGH) {

    // turn LED on

    sendMsg("0", baseIP, MY_UDP_PORT);
    if (!btn_high) {
      btn_high = true;

      //------------RIGHT-------------------------------

      //RIGHT GREEN LED
      /*digitalWrite(ledbtnPin, HIGH);
      delay(2000);
      digitalWrite(ledbtnPin, LOW);
      */

      //RIGHT RGB LED
      digitalWrite(led_blue_Pin, LOW);
      digitalWrite(led_red_Pin, LOW);
      digitalWrite(led_green_Pin, HIGH);
      delay(2000);
      digitalWrite(led_green_Pin, LOW);

      //------------WRONG-------------------------------

      /*counter = 5;
      digitalWrite(led_blue_Pin, LOW);
      digitalWrite(led_green_Pin, LOW);
      while(counter > 0){
      digitalWrite(led_red_Pin,HIGH);
      delay(300);
      digitalWrite(led_red_Pin, LOW);
      delay(300);
      counter--;
      }*/

      btn_high = false;
    }
  }
}