#include <ESP8266WiFi.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>

const char* ssid     = "team2rulz";
//const char* password = "bluerox95";

IPAddress ip(192, 168, 4, 1);

void setup() {
  Serial.begin(38400);
  Serial.println("Serial Begun");
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.persistent(false);
  //WiFi.begin(ssid, "");
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to Server");
}

void loop() {
  WiFiClient client;
  const int httpPort = 8080;
  if (client.connect(ip, httpPort)) {
    Serial.println("connected to server");
    //for(int i = 0; i <10; i++){
    //  client.print("hello");
    //  client.print('\n');
    //}

    //for(int i = 0; i <10; i++){
    //  client.print("02");
      //client.print('\n');
    //}
    while (client.connected()) {
      int msg = client.read();
      if(msg != -1){
        Serial.print("Recieved: ");
        Serial.println(msg);
      }
    }
    client.stop();
    Serial.println("Reconnecting");
    
    }
  Serial.print(".");
  delay(500);

}
