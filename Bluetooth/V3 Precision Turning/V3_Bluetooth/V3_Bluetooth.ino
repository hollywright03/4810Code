#include <SoftwareSerial.h>// import the serial library
#include <Servo.h>

#define LEFTMOTORFORWARD 112 //0
#define LEFTMOTORBACKWARD 113 //1
#define RIGHTMOTORFORWARD 114 //2
#define RIGHTMOTORBACKWARD 115 //3
#define LEFTSERVO 116 //4
#define RIGHTSERVO 117 //5
#define LEDTOGGLE 118 //6
#define PRECISEMOTOR 119 //7
#define GRABBERTOGGLE 120 //8

#define SPEEDOFF 112 //0
#define SPEEDSLOW 114 //2
#define SPEEDFAST 115 //3
#define SERVOCLOSE 113 //1
#define SERVOOPEN 114 //2

#define PRECISESTOP 112 //0
#define PRECISEFORWARD 113 //1
#define PRECISEBACKWARD 114 //2
#define PRECISELEFT 115 //3
#define PRECISERIGHT 116 //4

#define GRABBERCLOSE 112 //0
#define GRABBEROPEN 113 //1

#define ANGLECHANGE 20
#define PRECISEPWM 400 //~40% duty
//#define FASTPWM 1024 //full duty
//#define SLOWPWM 512 //half duty

SoftwareSerial Bluetooth(13, 15); // RX, TX
//SoftwareSerial Bluetooth(3, 1); // RX, TX
//int ledpin=16; // led on D13 will show blink on / off
int Byte1; // the data given from Computer
int Byte2;
bool byteCount;
bool messageRecieved;

Servo leftServo;
Servo rightServo;
int leftServoPin = 12;
int rightServoPin = 14;
int leftServoPos;
int rightServoPos;
int leftServoClose = 5;
int leftServoOpen = 125;
int rightServoClose = 175;
int rightServoOpen =55;

bool motorDirection;
int leftMotorForwardPin = 4;
int leftMotorBackwardPin = 5;
int rightMotorForwardPin = 0;
int rightMotorBackwardPin = 2;

int ledPin = 16;
bool ledOn;

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  //Serial.println("Serial Setup");
  Bluetooth.begin(9600);
  Bluetooth.println("Bluetooth On please press 1 or 0 blink LED ..");
  //pinMode(ledpin,OUTPUT);
  //digitalWrite(ledpin,1);
  byteCount = false;
  messageRecieved = false;

  leftServo.attach(leftServoPin);
  leftServo.write(90);
  leftServoPos = 90;
  rightServo.attach(rightServoPin);
  rightServo.write(90);
  rightServoPos = 90;

  pinMode(leftMotorForwardPin, OUTPUT);
  pinMode(leftMotorBackwardPin, OUTPUT);
  pinMode(rightMotorForwardPin, OUTPUT);
  pinMode(rightMotorBackwardPin, OUTPUT);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,HIGH);
  ledOn = true;
}

void loop() {
  if (messageRecieved){
    if(Byte1 == LEFTMOTORFORWARD){
      Bluetooth.println("LMF");
      //Serial.println("LMF");
      messageRecieved = false;
      motorDirection = true;
      moveLeftMotor(Byte2);
    }else if(Byte1 == LEFTMOTORBACKWARD){
      Bluetooth.println("LMB");
      //Serial.println("LMB");
      messageRecieved = false;
      motorDirection = false;
      moveLeftMotor(Byte2);
    }else if(Byte1 == RIGHTMOTORFORWARD){
      Bluetooth.println("RMF");
      //Serial.println("RMF");
      messageRecieved = false;
      motorDirection = true;
      moveRightMotor(Byte2);
    }else if(Byte1 == RIGHTMOTORBACKWARD){
      Bluetooth.println("RMB");
      //Serial.println("RMB");
      messageRecieved = false;
      motorDirection = false;
      moveRightMotor(Byte2);
    }else if(Byte1 == LEFTSERVO){
      Bluetooth.println("LS");
      //Serial.println("LS");
      messageRecieved = false;
      moveLeftServo(Byte2);
    }else if(Byte1 == RIGHTSERVO){
      Bluetooth.println("RS");
      //Serial.println("RS");
      messageRecieved = false;
      moveRightServo(Byte2);
    }else if(Byte1 == LEDTOGGLE){
      Bluetooth.println("LED");
      //Serial.println("RS");
      messageRecieved = false;
      ledToggle(Byte2);
    }else if(Byte1 == PRECISEMOTOR){
      Bluetooth.println("PM");
      messageRecieved = false;
      preciseMotor(Byte2);
    }else if(Byte1 == GRABBERTOGGLE){
      Bluetooth.println("GT");
      messageRecieved = false;
      toggleGrabber(Byte2);
    }else{
      Bluetooth.print("DNC: ");
      Bluetooth.print(Byte1);
      Bluetooth.print(" : ");
      Bluetooth.println(Byte2);
      //Serial.print("DNC: ");
      //Serial.print(Byte1);
      //Serial.print(" : ");
      //Serial.println(Byte2);
      messageRecieved = false;
      }
  }
  if (Bluetooth.available()){
    if(!byteCount){
      Byte1 = Bluetooth.read();
      byteCount = true;
    }else{
      Byte2 = Bluetooth.read();
      byteCount = false;
      messageRecieved = true;
      //Serial.print("Message Recieved: ");
      //Serial.print(Byte1);
      //Serial.print(" : ");
      //Serial.println(Byte2);
    }
  }
    
    
    //Serial.println("availiable");
    //Byte1=Bluetooth.read();
    //Serial.println(BluetoothData);
    //if(Byte1=='1'){   // if number 1 pressed ....
    //digitalWrite(ledpin,1);
    //Bluetooth.println("LED  On D13 ON ! ");
    //Genotronex.println("1");
    //}else if (Byte1=='0'){// if number 0 pressed ....
    //digitalWrite(ledpin,0);
    //Bluetooth.println("LED  On D13 Off ! ");
    //Genotronex.println("0");
    //}else {
   //   Bluetooth.print("DNC: ");
   //   Bluetooth.println(BluetoothData);
   //   }
delay(10);
}

void moveLeftServo(int command){
  if(command == SERVOCLOSE){
    leftServoPos -= ANGLECHANGE;
    if(leftServoPos < leftServoClose){
      leftServoPos = leftServoClose;
    }
    leftServo.write(leftServoPos);
  }else if(command == SERVOOPEN){
    leftServoPos += ANGLECHANGE;
    if(leftServoPos > leftServoOpen){
      leftServoPos = leftServoOpen;
    }
    leftServo.write(leftServoPos);
  }
}

void moveRightServo(int command){
  if(command == SERVOCLOSE){
    rightServoPos += ANGLECHANGE;
    if(rightServoPos > rightServoClose){
      rightServoPos = rightServoClose;
    }
    rightServo.write(rightServoPos);
  }else if(command == SERVOOPEN){
    rightServoPos -= ANGLECHANGE;
    if(rightServoPos < rightServoOpen){
      rightServoPos = rightServoOpen;
    }
    rightServo.write(rightServoPos);
  }
}

void toggleGrabber(int command){
  if(command == GRABBERCLOSE){
    rightServoPos = rightServoClose;
    rightServo.write(rightServoPos);
    leftServoPos = leftServoClose;
    leftServo.write(leftServoPos);
  }else if(command == GRABBEROPEN){
    rightServoPos = rightServoOpen;
    rightServo.write(rightServoPos);
    leftServoPos = leftServoOpen;
    leftServo.write(leftServoPos);
  }
}

void moveLeftMotor(int command){
  analogWrite(leftMotorForwardPin, 0);
  analogWrite(leftMotorBackwardPin, 0);
  int spd = 0;
  if(command == SPEEDSLOW){
    spd = 512;
  }else if(command == SPEEDFAST){
    spd = 1024;
  }
  if(motorDirection){
    analogWrite(leftMotorForwardPin, spd);
  }else{
    analogWrite(leftMotorBackwardPin, spd);
  }
}

void moveRightMotor(int command){
  analogWrite(rightMotorForwardPin, 0);
  analogWrite(rightMotorBackwardPin, 0);
  int spd = 0;
  if(command == SPEEDSLOW){
    spd = 512;
  }else if(command == SPEEDFAST){
    spd = 1024;
  }
  if(motorDirection){
    analogWrite(rightMotorForwardPin, spd);
  }else{
    analogWrite(rightMotorBackwardPin, spd);
  }
}

void preciseMotor(int command){
  analogWrite(rightMotorForwardPin, 0);
  analogWrite(rightMotorBackwardPin, 0);
  analogWrite(leftMotorForwardPin, 0);
  analogWrite(leftMotorBackwardPin, 0);

  if(command == PRECISEFORWARD){
    analogWrite(rightMotorForwardPin, PRECISEPWM);
    analogWrite(leftMotorForwardPin, PRECISEPWM);
  }else if(command == PRECISEBACKWARD){
    analogWrite(rightMotorBackwardPin, PRECISEPWM);
    analogWrite(leftMotorBackwardPin, PRECISEPWM);
  }else if(command == PRECISELEFT){
    analogWrite(rightMotorForwardPin, PRECISEPWM);
    analogWrite(leftMotorBackwardPin, PRECISEPWM);
  }else if(command == PRECISERIGHT){
    analogWrite(rightMotorBackwardPin, PRECISEPWM);
    analogWrite(leftMotorForwardPin, PRECISEPWM);
  }
}

void ledToggle(int command){
  if(ledOn == true){
    digitalWrite(ledPin,LOW);
    ledOn = false;
  }else{
    digitalWrite(ledPin,HIGH);
    ledOn = true;
  }
}
