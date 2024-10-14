#include "DFRobotDFPlayerMini.h"
#include <ESP32Servo.h>

DFRobotDFPlayerMini player;

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
#define WIFI_PSW "Earthbound"         // password of your wifi

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

// -------------- SERVO
#define PIN_SERVO_A 23
Servo servo_A;

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

int ledbtnPin = 5;       //pin 5- N_2
int led_blue_Pin = 26;   //26 - N_5 N_1 N_4, N_6 | 14 - N_7, N_3
int led_green_Pin = 27;  //27 - N_5 N_1 N_4, N_6 | 12 - N_7, N_3
int led_red_Pin = 12;    //12 - N_5 N_1 N_4, N_6 | 13 - N_7, N_3

bool btn_high = false;
int counter;

// ------------------- MAIN

void setup() {

  // SERIAL
  Serial.begin(9600);
  delay(200);

  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Inizializza la comunicazione seriale per il modulo DFPlayer Mini

  if (!player.begin(Serial2)) {  // Inizia la comunicazione con il modulo DFPlayer Mini
    Serial.println(F("Errore nel comunicare con il modulo DFPlayer"));
    while (true)
      ;
  }
  player.volume(5);  // Imposta il volume a 5 (0 - 30)
  player.play(1);    // Riproduce la prima traccia presente nella scheda SD
  //ASSICURATI DI NOMINARE CORRETTAMENTE I FILE MP3 (SENZA SPAZI O CARATTERI SPECIALI)
  //PUOI SUDDIVIDERLI IN CARTELLE MA QUESTO POTREBBE DARTI DEGLI ERRORI.
  player.play(2);  

  pinMode(led_blue_Pin, OUTPUT);
  pinMode(led_green_Pin, OUTPUT);
  pinMode(led_red_Pin, OUTPUT);
  pinMode(ledbtnPin, OUTPUT);
  digitalWrite(ledbtnPin, LOW);

  ESP32PWM::allocateTimer(0);               // Assegna un timer per il PWM (necessario per l'ESP32)
  servo_A.setPeriodHertz(50);               // Imposta la frequenza del servo a 50Hz (standard per i servo)
  servo_A.attach(PIN_SERVO_A, 1000, 2000);  // Attacca il servo al pin 18 (puoi cambiarlo) e imposta il range di movimento

  Serial.println("SETUP --- --- --- BEGIN");

  setup_wifi();
  delay(200);

  Serial.println("SETUP --- --- --- COMPLETE");
}

void loop() {

  Serial.print("accendo il Rosso");
  // turn LED on
  digitalWrite(led_red_Pin, HIGH);
  digitalWrite(led_blue_Pin, LOW);
  digitalWrite(led_green_Pin, LOW);

  Serial.print("giro il motore");

  servo_A.write(10);

  delay(2000);

  Serial.print("accendo il Verde");
  // turn LED on
  digitalWrite(led_green_Pin, HIGH);
  digitalWrite(led_blue_Pin, LOW);
  digitalWrite(led_red_Pin, LOW);

  Serial.print("giro il motore");
  servo_A.write(50);

  delay(2000);
}