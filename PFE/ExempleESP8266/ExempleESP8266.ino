#include <ESP8266WiFi.h>



void setup() {
  Serial.begin(115200);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect(); // delete old config
  delay(100);
  Serial.println("Setup done");
}

void loop() {
  Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
  }
  Serial.println();
  
  WiFi.disconnect(true); // delete old config
 
   Serial.println("Entrer le nom SSID :"); while (Serial.available()==0) delay(10);
    String str0 =Serial.readString();  
    char ssid0[str0.length() + 1];
    str0.toCharArray(ssid0, str0.length()+1);
   
      
   Serial.println("Entrer le mot de passe :"); while (Serial.available()==0) delay(10);;
    String str1 =Serial.readString();  
    char password0[str1.length() + 1];
    str1.toCharArray(password0, str1.length()+1);

   delay(1000);
   WiFi.begin(ssid0, password0);
 
  Serial.print( "Try to connect to SSID : ");Serial.print(ssid0); Serial.print( " :with: ");Serial.print(password0); 
 

   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
    Serial.println(WiFi.status());
    Serial.println(WiFi.localIP());
  }


     Serial.println("WiFi connected");
     Serial.println("IP address: ");
     Serial.println(WiFi.localIP());
   
    
while(1)
  delay(1000);
}

