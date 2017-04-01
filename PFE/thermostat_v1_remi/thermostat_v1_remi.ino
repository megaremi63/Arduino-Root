#include <FS.h>                   //this needs to be first, or it all crashes and burns...
//#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h> 
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
//#include <Wire.h>
//#include <SI7021.h>
// SENSORS
#include <DHT.h>
#define DHTTYPE DHT11  
#define LEDPIN 14
#define DHTPIN 10

DHT dht(DHTPIN, DHTTYPE);

int RESETPIN = 0;
#define si7021Addr 0x40
  
//#define MOTION_PIN          16    //d0, motion sensor
//#define RELAY_CH3_PIN       5     //d1, relay ch3
//#define RELAY_CH4_PIN       4     //d2, relay ch4
//#define COURANT_PIN         A0    //A0, capteur de courant
      
//#define RESETPIN          12

//defines for MQTT
#define MQTT_PORT       1883
#define WILL_TOPIC      "status"
#define WILL_QOS        0
#define RETAINED        true
#define WILL_MESSAGE    "offline"

long lastMsg = 0;
char msgt[50];
char msgh[50];
int value = 0;

//----- config pins ------
 // SoftwareSerial LCD(2, 16); // D4 et D2 comme RX et TX
//  SoftwareSerial LCD(13, 15); // D7 et D8 comme RX et TX
 
//--------------------------------
 int PWMpin = 14; 
 bool active ; 

 
//create clientID for mqtt
char mqttClientID[16];
int chipID = ESP.getChipId();
String str_chipID = String(chipID);
String str_mqttClientID = "esp" + str_chipID;
//create topic for new device
char newDevTopic[26];
String str_newDevTopic = "newDev/" + str_mqttClientID;
String newDeviceJson= "{ \"request\":{\"request_type\":\"newDevice\", \"SN\":\"esp"+str_chipID+"\", \"device_type\":\"thermostat\" }}";

//flag for saving data in file system
bool shouldSaveConfig = false;

//flag to indicate if the config file got retrived in the file system
bool failedToGetConfig = false;

//variables used to store data entered in config portal
char mqtt_user[40];
char mqtt_pass[40];
char mqtt_server[40];

//variables used for getting local id from node red
char localID[6];
bool nodeRedResp = false; //flag to wait until node-red sends localID

//variables for temp and hum
float humidity;
float celsTemp ;
                                      
WiFiClient espClient;
PubSubClient client(espClient);             //create a mqtt client

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// callback is used when the client is subscribed to topics in order to receive messages
void callback(char* topic, byte* payload, unsigned int length) 
{

  //print topic and payload to serial monitor
  //also save payload into char array, this way we can use strcmp
  char message[length];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    message[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  message[length] = '\0';             //add the null character
  Serial.println();
  

  //handling of the messages
  /*strcmp is used to verify on wich topic the message was sent. 
    Since strcmp can’t be used with a switch statement, an if statement will be used for each topic.
    When the to strings are equal, strcomp will output 0 */





      String subscribeTopic = "home/nodered/set/" + String(localID)+"/desired_temp";
      char subscribeTopicChar[subscribeTopic.length()+1];
      subscribeTopic.toCharArray(subscribeTopicChar,subscribeTopic.length()+1);
      
      
  if (strcmp(topic,subscribeTopicChar)==0) 
  {
   Serial.print("Message reçu : "); Serial.println(message);
  
  }


    

/************
    if (strcmp(topic,"home/relayCh4")==0) 
    {
            // Switch on the LED on or off
            if (strcmp(message,"on")==0) {
              digitalWrite(RELAY_CH4_PIN, HIGH);
            } else if (strcmp(message,"off")==0) {
              digitalWrite(RELAY_CH4_PIN, LOW); 
            }  
    }
 ****/   


  if (strcmp(topic,newDevTopic)==0) {
    nodeRedResp = true;
    strcpy(localID,message);
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, using username and password and Last will and testament
    if (client.connect(mqttClientID, mqtt_user, mqtt_pass, WILL_TOPIC, WILL_QOS, RETAINED, WILL_MESSAGE)) { 
      //Once connected, publish a message containning the status of the device
      //message is retained for all new subscribers to receive the status immediately when subscribing
      
     // String publishTopic = "home/status/" + String(localID);
     // client.publish(publishTopic, "online",RETAINED);


           String subscribeTopic = "home/nodered/set/" + String(localID)+"/desired_temp";
           char subscribeTopicChar[subscribeTopic.length()+1];
           subscribeTopic.toCharArray(subscribeTopicChar,subscribeTopic.length()+1);
      client.subscribe(subscribeTopicChar);
          subscribeTopic = "home/nodered/set/" + String(localID)+"/heat";
          char subscribeTopicChar2 [subscribeTopic.length()+1];
          subscribeTopic.toCharArray(subscribeTopicChar2,subscribeTopic.length()+1);
      client.subscribe(subscribeTopicChar2);
      
      Serial.println("connected");
    } else {
      //if we can't connect we publish the state                                         
      Serial.print("failed, rc=");                    
      Serial.print(client.state());

      //Wait 5 seconds before retrying
      Serial.println(" try again in 5 seconds");
      delay(5000);                                    
    }
    
   
  }
}
////-----------------------read temp humity---------------------------
//void readTempHum()
//{
//        unsigned int data[2];
//     
//      Wire.beginTransmission(si7021Addr);
//      //Send humidity measurement command
//      Wire.write(0xF5);
//      Wire.endTransmission();
//      delay(500);
//     
//      // Request 2 bytes of data
//      Wire.requestFrom(si7021Addr, 2);
//      // Read 2 bytes of data to get humidity
//      if(Wire.available() == 2)
//      {
//        data[0] = Wire.read();
//        data[1] = Wire.read();
//      }
//     
//      // Convert the data
//       humidity  = (((float)data[0] * 256.0) + data[1]);
//      humidity = ((125 * humidity) / 65536.0) - 6;
//     
//      Wire.beginTransmission(si7021Addr);
//      // Send temperature measurement command
//      Wire.write(0xF3);
//      Wire.endTransmission();
//      delay(500);
//     
//      // Request 2 bytes of data
//      Wire.requestFrom(si7021Addr, 2);
//     
//      // Read 2 bytes of data for temperature
//      if(Wire.available() == 2)
//      {
//        data[0] = Wire.read();
//        data[1] = Wire.read();
//      }
//     
//      // Convert the data
//      float temp  = (((float)data[0] * 256.0) + data[1]);
//       celsTemp = ((175.72 * temp) / 65536.0) - 46.85;
//      float fahrTemp = celsTemp * 1.8 + 32;
//
//  
//}

//---------------------------------------------------------------------
void setup() {  
pinMode( RESETPIN , INPUT);
    if (digitalRead(RESETPIN) == LOW) 
    {
//          digitalWrite(LEDPIN, HIGH);
//          delay(5000);
//          digitalWrite(LEDPIN, LOW);
          if (SPIFFS.format()){
            Serial.println("formated file system");
          }
          delay(2000);
          ESP.restart();
      }  






  
  Serial.begin(115200);                     //begin serial communication, wifimanager communicates at 115200
  //LCD.begin(9600);

pinMode(PWMpin, OUTPUT); 

  
  /*------------i2c config pour si7021-----------------------
  Wire.begin();
  Wire.beginTransmission(si7021Addr);
  Wire.endTransmission();
  delay(300);
 */
  //clear file system for testing
  /*
  if (SPIFFS.format()){
    Serial.println("formated file system");
  }
  */

  str_mqttClientID.toCharArray(mqttClientID,16);   //convert to char in order to use as argument for MQTT and WM
  Serial.println(" ");
  Serial.print("mqtt client ID: ");
  Serial.println(mqttClientID);
  str_newDevTopic.toCharArray(newDevTopic,26);    //convert to char in order to use as argument for MQTT

  
  /************************WiFiManager***********************************/
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // The extra parameters to be configured
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user name", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt password", mqtt_pass, 40);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(mqttClientID, "password")) {
    //will only enter here if there is a timeout set
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  
  //if you get here you have connected to the WiFi
  Serial.println("wifi connected");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  /************************WiFiManager end***********************************/

  //if wifi connected without portal, we check the config file
  if (!shouldSaveConfig){
  //try to open and read config from FS json
    Serial.println("mounting FS...");
    if (SPIFFS.begin()) {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial.println("config file exists");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);
  
          configFile.readBytes(buf.get(), size);
          DynamicJsonBuffer jsonBuffer;
          JsonObject& json = jsonBuffer.parseObject(buf.get());
          json.printTo(Serial);
          if (json.success()) {
            Serial.println("\nparsed json");

            strcpy(mqtt_server, json["mqtt_server"]);
            strcpy(mqtt_user, json["mqtt_user"]);
            strcpy(mqtt_pass, json["mqtt_pass"]);
            strcpy(localID, json["localID"]);
            
            configFile.close();

            //connect to mqtt, we must connect here in order to get local ID from node-red
            client.setServer(mqtt_server, 1883);  //sets the server details
            client.setCallback(callback);         //sets the message callback function
          } else {
            Serial.println("failed to load json config");
            failedToGetConfig = true;
          }
        } else {
          Serial.println("failed to open config.json");
          failedToGetConfig = true;
        }
      } else {
        Serial.println("config.json does not exists");
        failedToGetConfig = true;
      }
    } else {
      Serial.println("failed to mount FS");
      failedToGetConfig = true;
    }
  }
  

  if (shouldSaveConfig || failedToGetConfig){
    //start the wifi portal
    if (failedToGetConfig){
      wifiManager.startConfigPortal(mqttClientID, "password");
    }
    
    //read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  
    //connect to mqtt, we must connect here in order to get local ID from node-red
    client.setServer(mqtt_server, 1883);  //sets the server details
    client.setCallback(callback);         //sets the message callback function

    //wait for node-red response. send request every 5 sec for local ID
    Serial.println("Waiting for NodeRed local ID");
    while (!nodeRedResp){
      if (!client.connected()) {
        reconnect();
        client.subscribe(newDevTopic);  //set the subcription on the topic that will publish local id                                
      }
      //***a modifier***
     
      char newDeviceJsonChar[newDeviceJson.length()+1];
      newDeviceJson.toCharArray(newDeviceJsonChar,newDeviceJson.length()+1);
      
      client.publish("newDev",newDeviceJsonChar );
      Serial.print(".");
      delay(5000);
      client.loop(); //must be called to receive message
    } 
    
    //when your reach here this mean you got a local ID
    Serial.println("received local ID from node-red");
    
    //we now have all the info and can save it to FS
    Serial.println("saving config");
    //formatting config
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pass"] = mqtt_pass;
    json["localID"] = localID;

    SPIFFS.begin();
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    } else {
      json.printTo(Serial);
      Serial.println(" ");
      json.printTo(configFile);
      configFile.close();
    }
  }

}


void loop() {
  
  //Checks if the client is connected to the server
  //if not connected, call the reconnect fonction
  if (!client.connected()) {
    reconnect();                                  
  }
  
  //allows the client to process incoming messages
  //and maintain its connection to the server.
  client.loop();
long now = millis();
  if (now - lastMsg > 10000) {
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
    
   // snprintf (msgt, 75, "000,temperature,%i",temperature);
   // snprintf (msgh, 75, "000,humidity,%i",humidity);

    String publishTopic = "home/sensor/" + String(localID)+"/temperature";
    char publishTopicChar[publishTopic.length()+1];
    publishTopic.toCharArray(publishTopicChar,publishTopic.length()+1);

    String publishTopicH = "home/sensor/" + String(localID)+"/humidity";
    char publishTopicHChar[publishTopicH.length()+1];
    publishTopicH.toCharArray(publishTopicHChar,publishTopicH.length()+1);

      
    client.publish(publishTopicChar, tChar);
    client.publish(publishTopicHChar, hChar);
    Serial.print("Published message: ");
    Serial.println(msgt);
    Serial.println(msgh);




//    //=====tem
//     String publishTopic = "home/sensor/" + String(localID)+"/temperature";
//      char publishTopicChar[publishTopic.length()+1];
//      publishTopic.toCharArray(publishTopicChar,publishTopic.length()+1);
//
//
//      String str_Temp = String(celsTemp); char char_Temp[10]; 
//      str_Temp.toCharArray(char_Temp,10);
//      client.publish(publishTopicChar, char_Temp);
//
////======hum
//      String publishTopicH = "home/sensor/" + String(localID)+"/humidity";
//      char publishTopicHChar[publishTopicH.length()+1];
//      publishTopicH.toCharArray(publishTopicHChar,publishTopicH.length()+1);
//
//
//  String str_Hum = String(humidity); char char_Hum[10]; 
//  str_Hum.toCharArray(char_Hum,10);
//  client.publish(publishTopicHChar, char_Hum);

}
}


