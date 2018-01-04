/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char* ssid = "[SSID_HERE]";
const char* password = "[SECRET_HERE]";

IPAddress ip ( 192,  168,   1,  201);
IPAddress gateway_ip ( 192,  168,   1,   1);
IPAddress subnet_mask(255, 255, 255,   0);

uint8_t relayState = HIGH;  // HIGH: closed switch
uint8_t buttonState = HIGH;
uint8_t currentButtonState = buttonState;

long buttonStartPressed = 0;
long buttonDurationPressed = 0;

// Sonoff properties
const uint8_t BUTTON_PIN = 0;
const uint8_t RELAY_PIN  = 12;
const uint8_t LED_PIN    = 13;

enum CMD {
  CMD_NOT_DEFINED,
  CMD_PIR_STATE_CHANGED,
  CMD_BUTTON_STATE_CHANGED,
};
volatile uint8_t cmd = CMD_NOT_DEFINED;

ESP8266WebServer server(80);

void setup_wifi() {
  delay(500);

  // connect static
  WiFi.config(ip, gateway_ip, subnet_mask, gateway_ip);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

///////////////////////////////////////////////////////////////////////////
//   ISR
///////////////////////////////////////////////////////////////////////////
/*
  Function called when the button is pressed/released
*/
void buttonStateChangedISR() {
  cmd = CMD_BUTTON_STATE_CHANGED;
}

///////////////////////////////////////////////////////////////////////////
//   Sonoff switch
///////////////////////////////////////////////////////////////////////////
/*
  Function called to set the state of the relay
*/
void setRelayState() {
  digitalWrite(RELAY_PIN, relayState);
  digitalWrite(LED_PIN, (relayState + 1) % 2);
}

void changeState() {
  relayState = relayState == HIGH ? LOW : HIGH;
  setRelayState();

  String message = "";
  message += relayState;
  server.send(200, "text/html", message);
}

void OffThenOnSwitch() {
  relayState = LOW;
  setRelayState();

  delay(3000);

  relayState = HIGH;
  setRelayState();

  String message = "";
  message += relayState;
  server.send(200, "text/html", message);
}

void temporarySwitch() {
  relayState = relayState == HIGH ? LOW : HIGH;
  setRelayState();

  delay(3000);

  relayState = relayState == HIGH ? LOW : HIGH;
  setRelayState();

  String message = "";
  message += relayState;
  server.send(200, "text/html", message);
}

void getStatus() {
  String message = "";
  message += relayState;
  server.send(200, "text/html", message);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  // init the I/O
  pinMode(LED_PIN,    OUTPUT);
  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(BUTTON_PIN, buttonStateChangedISR, CHANGE);

  OffThenOnSwitch();

  setup_wifi();

  IPAddress myIP = WiFi.localIP();
  Serial.println("AP IP address: ");
  Serial.println(myIP);
  server.on("/switch", changeState);
  server.on("/temporarySwitch", temporarySwitch);
  server.on("/offThenOnSwitch", OffThenOnSwitch);
  server.on("/state", getStatus);
  server.begin();
  Serial.println("HTTP server started");
  delay(2000);
}

void loop() {
  switch (cmd) {
    case CMD_NOT_DEFINED:
      // do nothing
      server.handleClient();
      break;
    case CMD_BUTTON_STATE_CHANGED:
      currentButtonState = digitalRead(BUTTON_PIN);
      if (buttonState != currentButtonState) {
        // tests if the button is released or pressed
        if (buttonState == LOW && currentButtonState == HIGH) {
          buttonDurationPressed = millis() - buttonStartPressed;
          if (buttonDurationPressed < 500) {
            relayState = relayState == HIGH ? LOW : HIGH;
            setRelayState();
          }
        } else if (buttonState == HIGH && currentButtonState == LOW) {
          buttonStartPressed = millis();
        }
        buttonState = currentButtonState;
      }
      cmd = CMD_NOT_DEFINED;
      server.handleClient();
      break;
  }

  yield();

  if (WiFi.status() != WL_CONNECTED)
  {
    setup_wifi();
  }
  server.handleClient();

  yield();
}
