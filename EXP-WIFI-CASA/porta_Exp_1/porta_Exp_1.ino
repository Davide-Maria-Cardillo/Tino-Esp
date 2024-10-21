#include "DFRobotDFPlayerMini.h"
#include <ESP32Servo.h>

DFRobotDFPlayerMini player;

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
const uint8_t default_ledPinOn = 27;        // digital GPIO27
WiFiUDP UDP;
char in_packet[IN_SIZE];
char out_packet[OUT_SIZE];

// ESP STATIC IP -> ESP3 DOOR TASK
const IPAddress localIP(192, 168, 0, 7);  // ESP3 DOOR TASK

// -------------- SERVO
#define PIN_SERVO_A 23
Servo servo_A;

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
    int len = UDP.read(in_packet, 10);  // the value is written in the BUFFER specificed as the first argument ("in_packet" in our case)

    msg = (char)in_packet[0];
    Serial.println(msg);
  }

  return msg;
}

// ------------------- VARIABLES

int led_red_Pin = 12;
int led_green_Pin = 27;

// ------------------- MAIN

void setup() {

  // SERIAL
  Serial.begin(9600);
  delay(200);

  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Inizializza la comunicazione seriale per il modulo DFPlayer Mini
  delay(1000);

  if (!player.begin(Serial2)) {  // Inizia la comunicazione con il modulo DFPlayer Mini
    Serial.println(F("Errore nel comunicare con il modulo DFPlayer"));
    while (true)
      ;
  }
  player.volume(30);  // Imposta il volume a 5 (0 - 30)

  pinMode(led_green_Pin, OUTPUT);
  pinMode(led_red_Pin, OUTPUT);
  digitalWrite(led_green_Pin, LOW);
  digitalWrite(led_red_Pin, HIGH);  // Start with LED red ON

  ESP32PWM::allocateTimer(0);               // Assegna un timer per il PWM (necessario per l'ESP32)
  servo_A.setPeriodHertz(50);               // Imposta la frequenza del servo a 50Hz (standard per i servo)
  servo_A.attach(PIN_SERVO_A, 1000, 2000);  // Attacca il servo al pin 18 (puoi cambiarlo) e imposta il range di movimento
  servo_A.write(0);

  Serial.println("SETUP --- --- --- BEGIN");

  setup_wifi();
  delay(200);

  Serial.println("SETUP --- --- --- COMPLETE");
}

void loop() {

  // Leggi il messaggio UDP
  char msg = read_UDP();

  // Gestione dei messaggi ricevuti
  if (msg == '1') {
    // Messaggio "1" ricevuto da ESP2 per "leggi QR code"
    qr_code_read = true;
    Serial.println("QR code read activated.");
  } else if (msg == '2') {
    // Messaggio "2" ricevuto da ESP1 per bottone premuto con "leggi QR code" attivo
    if (qr_code_read) {
      // Riproduci audio 1, accendi LED verde, muovi il motore
      player.play(2);
      digitalWrite(led_red_Pin, LOW);
      digitalWrite(led_green_Pin, HIGH);
      servo_A.write(50);
      Serial.println("Playing positive sound, turning LED green, moving servo to 50.");
    }
  } else if (msg == '3') {
    // Messaggio "3" ricevuto da ESP1 per bottone premuto senza "leggi QR code" attivo
    if (!qr_code_read) {
      // Riproduci audio 2, mantieni LED rosso
      player.play(1);
      digitalWrite(led_red_Pin, HIGH);
      digitalWrite(led_green_Pin, LOW);
      Serial.println("Playing negative sound, keeping LED red.");
    }
  }
}
