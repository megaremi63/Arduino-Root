//CODE POUR ESP

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID "Rogers56600"
#define WIFI_PASSWORD "88CDFRogers"
#define DHTTYPE DHT11  
#define LEDPIN 14
#define DHTPIN 10

const char* mqtt_server = "192.168.0.16";
long lastMsg = 0;
char msg[50];
int value = 0;

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor.
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
  pinMode(DHTPIN, INPUT);
  setup_wifi();
  dht.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LEDPIN, HIGH);
  } else {
    digitalWrite(LEDPIN, LOW); 
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "MQTT Connected");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 200000) {
    lastMsg = now;
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    client.loop();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    client.loop();
    int humidity = (int) h;
    int temperature = (int) t;
  
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    
    ++value;
    char tChar[10];
    char hChar[10];
    dtostrf(t, 4, 2, tChar);
    dtostrf(h, 4, 2, hChar);
    
    snprintf (msg, 75, "'Office',%i,%i",temperature, humidity);
    Serial.print("Published message: ");
    Serial.println(msg);
    client.publish("Sensor/Office/Temperature", tChar);
    client.publish("Sensor/Office/Humidity", hChar);
    client.publish("Sensor/Office/SQL", msg);

    // Convert data to JSON

    String json =
      "{\"data\":{"
      "\"humidity\": \"" + String(h) + "\","
      "\"room\": \"" + "Office" + "\","
      "\"temperature\": \"" + String(t)+ "\"}"
      "}";

 // Convert JSON string to character array
    char jsonChar[100];
    json.toCharArray(jsonChar, json.length()+1);
 
 // Publish JSON character array to MQTT topic
     client.publish("Sensor/Office/json", jsonChar); 
  }
  
}

