#include <SoftwareSerial.h>// import the serial library

//SoftwareSerial Genotronex(10, 11); // RX, TX
SoftwareSerial Genotronex(13, 15); // RX, TX
int ledpin=5; // led on D13 will show blink on / off
int BluetoothData; // the data given from Computer

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  Serial.println("Setup");
  Genotronex.begin(9600);
  Genotronex.println("Bluetooth On please press 1 or 0 blink LED ..");
  pinMode(ledpin,OUTPUT);
  digitalWrite(ledpin,1);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Genotronex.available()){
    Serial.println("availiable");
    BluetoothData=Genotronex.read();
    Serial.println(BluetoothData);
    if(BluetoothData=='1'){   // if number 1 pressed ....
    digitalWrite(ledpin,1);
    Genotronex.println("LED  On D13 ON ! ");
    //Genotronex.println("1");
    }else if (BluetoothData=='0'){// if number 0 pressed ....
    digitalWrite(ledpin,0);
    Genotronex.println("LED  On D13 Off ! ");
    //Genotronex.println("0");
    }else {
      Genotronex.print("DNC: ");
      Genotronex.println(BluetoothData);
      }
}
delay(100);// prepare for next data ...
}
