#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_wpa2.h" // WPA2 Enterprise

#define LED_PIN 2 // Onboard LED for ESP32 Wrover is usually GPIO2

const char* ssid = "eduroam";
const char* username = "s135591@student.squ.edu.om";
const char* password = "xxxxxxx"; // Replace with your real password

WebServer server(80);
bool ledState = false;

String htmlPage() {
  String state = ledState ? "ON" : "OFF";
  String buttonLabel = ledState ? "Turn OFF" : "Turn ON";
  String buttonColor = ledState ? "#e74c3c" : "#2ecc71";
  return "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>ESP32 LED Control</title>"
         "<style>body{font-family:sans-serif;background:#222;color:#fff;text-align:center;padding-top:40px;}"
         ".led{font-size:2em;margin:20px;}button{font-size:1.5em;padding:15px 40px;border:none;border-radius:8px;background:" + buttonColor + ";color:#fff;cursor:pointer;transition:background 0.2s;}button:hover{background:#16a085;}</style></head>"
         "<body><h1>ESP32 LED Control</h1>"
         "<div class='led'>LED is <b>" + state + "</b></div>"
         "<form action='/toggle' method='POST'><button type='submit'>" + buttonLabel + "</button></form>"
         "<p style='margin-top:40px;font-size:0.9em;color:#aaa;'>WiFi: " + String(ssid) + "</p>"
         "</body></html>";
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleToggle() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println();
  Serial.print("Connecting to WiFi (WPA2-Enterprise): ");
  Serial.println(ssid);
  WiFi.disconnect(true); // clean up old config
  delay(1000);
  WiFi.mode(WIFI_STA);

  // WPA2 Enterprise Config
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
  esp_wifi_sta_wpa2_ent_enable();

  WiFi.begin(ssid);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Open http://");
    Serial.print(WiFi.localIP());
    Serial.println("/ in your browser");
  } else {
    Serial.println("WiFi connection failed!");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}