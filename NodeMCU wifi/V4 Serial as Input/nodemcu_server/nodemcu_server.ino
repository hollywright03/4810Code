#include <ESP8266WiFi.h>
#include <Wire.h>

const char WiFiPass[] = "password";
const char WiFiName[] = "team2rulz";

WiFiServer server(8080);

int Byte1; // the data given from Computer
int Byte2;
bool byteCount;
bool messageRecieved;

void setup() {
  Serial.begin(38400);
  Serial.println("Serial Begun");
  delay(10);

  Serial.println("Starting Access point");
  WiFi.mode(WIFI_AP);
  //uint8_t mac[WL_MAC_ADDR_LENGTH];
  //WiFi.softAPmacAddress(mac);
  //String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
  //               String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  //macID.toUpperCase();
  
  WiFi.softAP(WiFiName);
  Serial.println("Wifi AP Ready");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
  Serial.println("Waiting for client");

  byteCount = false;

}

//WiFiClient client;
//char msg[4];
void loop() {

  //if (Serial.available()) {
  //  Serial.readBytes(msg, 2);
  //  messageRecieved = true;
  //}
  //  if(messageRecieved){

  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client Connected");
    while(client.connected()){
      //String msg = client.readStringUntil('\n');
      int msg = client.read();
      if(msg != -1){
        Serial.print("Recieved: ");
        Serial.println(char(msg));
      }
    }
  }
  //delay(500);
  //Serial.print(".");
    //Serial.println("Connecting to Client");
    //client.stop();
    //client = server.available();
    //if (client.available()) {
    //  String msg = client.readStringUntil('\r');
    //  Serial.print("Recieved: ");
    //  Serial.println(msg);
    //      client.flush();
    //      client.println(msg);
    //      Serial.print("Message: ");
    //      Serial.println(msg);
    //      messageRecieved = false;
    //      //client.flush();
    //      Serial.println("Flushed Client");
  //}
  //  }
  //Serial.print("Recieved:");
  //Serial.println(msg);
  //client.println("2>>1");
  //client.flush();
  //Serial.println("Flushed Client");
  //Serial.print(".");
  //delay(20);
  //delay(500);
}
