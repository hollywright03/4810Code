/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>

//const char* ssid     = "Hoolly doolly";
const char* ssid     = "zsun-sd7F017A21";
//const char* password = "bluerox95";

//const char* host = "localhost";
IPAddress ip(10,168,168,102);

// Software serial
SoftwareSerial cameraconnection = SoftwareSerial(13, 15);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

void setup() {
  Serial.begin(38400);
  delay(10);

  Serial.println("Camera test");
  
  // Try to locate the camera
  if (!cam.begin()) {
    Serial.println("Camera not found.");
    while(1);
  } else {
    Serial.println("Camera found: ");
  }
  // Set picture size
  cam.setImageSize(VC0706_160x120);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.persistent(false);
  WiFi.begin(ssid, "");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int value = 0;

void loop() {
  delay(5000);
  ++value;

    //for (int i = 0; i < 10; i++) {
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  // Prepare request
  String start_request = "";
  String end_request = "";
  start_request = start_request + "\n" + "--AaB03x" + "\n" + "Content-Disposition: form-data; name=\"picture\"; filename=\"cam.jpg\"" + "\n" + "Content-Type: image/jpeg" + "\n" + "\n";  
  end_request = end_request + "\n" + "--AaB03x--" + "\n";
  
  uint16_t extra_length;
  extra_length = start_request.length() + end_request.length();
  Serial.println("Extra length:");
  Serial.println(extra_length);
  
  uint16_t len = jpglen + extra_length;
 
  Serial.println("Starting connection to server...");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  while (!client.connect(ip, httpPort)) {
    Serial.println("connection failed");
  }
  
  Serial.println(F("Connected !"));
  String message = "POST /dashboard/camera/camera.php HTTP/1.1\r\nHost: 192.168.133.1:80\r\nContent-Type: multipart/form-data; boundary=AaB03x\r\nContent-Length: ";
  Serial.print(message);    
  Serial.print(len);
  Serial.print(start_request); 
  
  client.println(F("POST /dashboard/camera/camera.php HTTP/1.1"));
  client.println(F("Host: 192.168.133.1:80"));
  client.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
  client.print(F("Content-Length: "));
  client.println(len); 
  client.print(start_request); 
//  client.print(message);    
//  client.print(len);
//  client.print(start_request);    

  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
        
    uint8_t *buffer;
    uint8_t bytesToRead = 64;
    if (64 > jpglen) {
      bytesToRead = jpglen;
    }
    buffer = cam.readPicture(bytesToRead);

//    for (int i = 0; i < sizeof(buffer); i++) {
//      client.write(&buffer[i], 1);
//      //Serial.print(buffer[i]);
//    }
    client.write((const char *)buffer, jpglen);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }
    jpglen -= bytesToRead; 
  }
   
  client.print(end_request);
  Serial.print(end_request);
  client.println();
  

//  Serial.print("connecting to ");
//  Serial.println(ip);
//  
//  
//  
//  // We now create a URI for the request
//  String url = "http://192.168.133.1/dashboard/camera/camera.php";
//  
//  Serial.print("Requesting URL: ");
//  Serial.println(url);
//  
//  // This will send the request to the server
//  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
//               "Host:192.168.133.1:80\r\n" + 
//               "Connection: close\r\n\r\n");
//  int timeout = millis() + 5000;
//  while (client.available() == 0) {
//    if (timeout - millis() < 0) {
//      Serial.println(">>> Client Timeout !");
//      client.stop();
//      return;
//    }
//  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
//}
}

