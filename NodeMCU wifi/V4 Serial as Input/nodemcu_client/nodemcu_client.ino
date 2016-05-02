#include <ESP8266WiFi.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>

const char* ssid     = "team2rulz";
//const char* password = "bluerox95";

IPAddress ip(192, 168, 4, 1);

char byte1;
char byte2;
bool byteCount;
bool messageRecieved;

void setup() {
  Serial.begin(38400);
  //Serial.println("Serial Begun");
  delay(10);

  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  WiFi.persistent(false);
  //WiFi.begin(ssid, "");
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  //Serial.println("Connecting to Server");
}

void loop() {
  WiFiClient client;
  const int httpPort = 8080;
  if (client.connect(ip, httpPort)) {
    //Serial.println("connected to server");

    while(client.connected()){
      if(messageRecieved){
        client.print(byte1);
        client.print(byte2);
        Serial.print("R:");
        Serial.print(byte1);
        Serial.println(byte2);
        messageRecieved = false;
      }
      if(Serial.available()){
        if(!byteCount){
          byte1 = Serial.read();
          byteCount = true;
        }else{
          byte2 = Serial.read();
          byteCount = false;
          messageRecieved = true;
        }
      }
    }
    
    client.stop();
    byteCount = false;
    messageRecieved = false;
    //Serial.println("Reconnecting");
    }
  Serial.print(".");
  delay(500);

  //Serial.println("not Connected!");

  //client.println(F("hello"));

}
