#include <Servo.h>
#include <SoftwareSerial.h>

#define LEFTF 5
#define LEFTB 4
#define RIGHTF 0
#define RIGHTB 2

char returnChar = '\r';

// Servo variables
Servo servoLeft; Servo servoRight;
int leftPos = 15;
int rightPos = 180;

SoftwareSerial BT(3, 1);

void setup() {
  servoLeft.attach(14); //D5
  servoRight.attach(12); //D6
  servoLeft.write(leftPos);
  servoRight.write(rightPos);

  BT.begin(9600);
  Serial.begin(9600);
  BT.println("Hello from Arduino");

  Serial.begin(9600);

  // Set up output pins
  pinMode(LEFTF, OUTPUT);
  pinMode(LEFTB, OUTPUT);
  pinMode(RIGHTF, OUTPUT);
  pinMode(RIGHTB, OUTPUT);
}

// Set the motor speed for the specific motor.
void motorSpeed(int motorPin, int spd) {
  switch (motorPin) {
    case LEFTF:
      analogWrite(LEFTB, 0);
      break;
    case LEFTB:
      analogWrite(LEFTF, 0);
      break;
    case RIGHTF:
      analogWrite(RIGHTB, 0);
      break;
    case RIGHTB:
      analogWrite(RIGHTF, 0);
      break;
  }

  if (spd == 0) {
    analogWrite(motorPin, 0);
  } else {
    // For 1 return 800, rough minimum speed, for 2 return 1023
    analogWrite(motorPin, 223 * spd + 577);
  }
  Serial.print("MOTOR: ");
  Serial.println(motorPin);
}

// Move the servo.
void servoMove(Servo servo, int dir) {
  int diff = 20;
  int pos = servo.read();
  // Servo angle increasing, ie. opening
  switch (dir) {
    case 0:
      break;
    case 1:
      if ((pos + diff) > 180) {
        servo.write(180);
      } else {
        servo.write(pos + diff);
      }
      break;
    case 2:
      if ((pos - diff) < 0) {
        servo.write(0);
      } else {
        servo.write(pos - diff);
      }
      break;
  }
}

void open() {
  servoRight.write(55);
  servoLeft.write(140);
}

void close() {
  servoRight.write(180);
  servoLeft.write(5);
}

/* Interprets the incoming message and calls the correct function. */
void read_message(char bit1, char bit2) {
  //motorFunction motorFunc;
  //int input1;
  int input2 = bit2 - '0';

  switch (bit1) {
    case '0':
      motorSpeed(LEFTF, input2);
      break;
    case '1':
      motorSpeed(LEFTB, input2);
      break;
    case '2':
      motorSpeed(RIGHTF, input2);
      break;
    case '3':
      motorSpeed(RIGHTB, input2);
      break;
    case '4':
      servoMove(servoLeft, input2);
      break;
    case '5':
      servoMove(servoRight, input2);
      break;
  }

}



char readByte; //stores incoming character from other device
bool justReceived = false; //stores whether a character has been recieved.
char i, j, a;
int k = 0;
String readStr;
//char readStr[3];

void loop() {

  if (BT.available()) {
    while (BT.available()) {
      a = (char)BT.read();
      if (a == returnChar || k > 2) {
        justReceived = true;
      } else {
        readStr += a;
      }
    }
    //    BT.print(a);
    //    Serial.print(a);
    if (justReceived) {
      i = readStr[0];
      j = readStr[1];
      Serial.print("i = ");
      Serial.println(i);
      Serial.print("j = ");
      Serial.println(j);
      read_message(i, j);
      i = 0;
      j = 0;
      readStr = "";
    }
    //if (BT.available()) {
    //  a = BT.read();
    //  if (a == 'o') {
    //    open();
    //  } else if (a == 'c') {
    //    close();
    //  }
    //}





    //  if (BT.available()) {
    //    BT.println("blah blah");
    //    readByte = (BT.read());
    //    justReceived = true;
    //
    //    if (i < 19) {
    //      readStr[i] = readByte;
    //      i++;
    //      readStr[i] = '\0';
    //    }
    //
    //    }
    //    if (justReceived) {
    //      Serial.println("Hello");
    //      Serial.println(readStr);
    //      String test = String(readStr);
    //      String side = test.substring(0,1);
    //      String spd = test.substring(1);
    //      justReceived = false;
    //      i = 0;
    //      readStr[0] = '\0';
    //    }

  }
}
