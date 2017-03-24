#include <ESP8266WiFi.h>

#include "WemoSwitch.h"
#include "WemoManager.h"
#include "CallbackFunction.h"

// prototypes
boolean connectWifi();

//on/off callbacks
void lightOn();
void lightOff();

//------- Replace the following! ------
char ssid[] = "Rogers56600";       // your network SSID (name)
char password[] = "88CDFRogers";  // your network key

WemoManager wemoManager;
WemoSwitch *light = NULL;

const int ledPin = 16;

void setup()
{
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  light = new WemoSwitch("Office Lights", 80, lightOn, lightOff);
  wemoManager.addDevice(*light);

  pinMode(ledPin, OUTPUT); // initialize digital ledPin as an output.
  delay(10);
  digitalWrite(ledPin, HIGH); // Wemos BUILTIN_LED is active Low, so high is off
}

void loop()
{
  wemoManager.serverLoop();
}

void lightOn() {
    Serial.print("Switch 1 turn on ...");
    digitalWrite(ledPin, HIGH);
}

void lightOff() {
    Serial.print("Switch 1 turn off ...");
    digitalWrite(ledPin, LOW);
}
