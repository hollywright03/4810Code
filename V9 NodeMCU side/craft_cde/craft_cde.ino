#include <Servo.h>
#include <SoftwareSerial.h>

#define LEFTF 4
#define LEFTB 5
#define RIGHTF 2
#define RIGHTB 0

char returnChar = '\r';

// Servo variables
Servo servoLeft; Servo servoRight;
int leftPos = 125;
int rightPos = 55;

int ledPin = 16;
bool ledOn;

SoftwareSerial BT(13, 15);

void setup() {
  servoLeft.attach(12); //D5
  servoRight.attach(14); //D6
  servoLeft.write(leftPos);
  servoRight.write(rightPos);

  BT.begin(9600);
  Serial.begin(9600);
  BT.println("Hello from Computer");

  //Serial.begin(9600);

  // Set up output pins
  pinMode(LEFTF, OUTPUT);
  pinMode(LEFTB, OUTPUT);
  pinMode(RIGHTF, OUTPUT);
  pinMode(RIGHTB, OUTPUT);

  // LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  ledOn = true;
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
  } else if (spd == 1) {
    analogWrite(motorPin, 800);
  } else if (spd == 2) {
    // For 1 return 800, rough minimum speed, for 2 return 1023
    //analogWrite(motorPin, 223 * spd + 577);
    analogWrite(motorPin, 1023);
  } else if (spd == 3) {
    analogWrite(motorPin, 400);
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

void open_grabber() {
  servoRight.write(55);
  servoLeft.write(125);
}

void close_grabber() {
  servoRight.write(175);
  servoLeft.write(5);
}

void led_toggle() {
  if (ledOn) {
    digitalWrite(ledPin, LOW);
    ledOn = false;
  } else {
    digitalWrite(ledPin, HIGH);
    ledOn = true;
  }
}

void forward(int spd) {
  motorSpeed(LEFTF, spd);
  motorSpeed(RIGHTF, spd);
}

void backward(int spd) {
  motorSpeed(LEFTB, spd);
  motorSpeed(RIGHTB, spd);
}

void left_turn(int spd) {
  motorSpeed(LEFTB, spd);
  motorSpeed(RIGHTF, spd);
}

void right_turn(int spd) {
  motorSpeed(LEFTF, spd);
  motorSpeed(RIGHTB, spd);
}

void stopMotor() {
  motorSpeed(LEFTF, 0);
  motorSpeed(RIGHTF, 0);
}

void precision_turn(int dir) {
  switch (dir) {
    case 0:
      stopMotor();
      break;
    case 1:
      forward(3);
      break;
    case 2:
      backward(3);
      break;
    case 3:
      left_turn(3);
      break;
    case 4:
      right_turn(3);
      break;
    default:
      break;
  }
}

void grabber(int cmd) {
  if (cmd == 0) {
    close_grabber();
  } else if (cmd == 1) {
    open_grabber();
  }
}

/* Interprets the incoming message and calls the correct function. */
void read_message(int bit1, int input2) {
  //motorFunction motorFunc;
  //int input1;
  //int input2 = bit2 - '0';

  //  switch (bit1) {
  //    case 0:
  //      motorSpeed(LEFTF, input2);
  //      break;
  //    case 1:
  //      motorSpeed(LEFTB, input2);
  //      break;
  //    case 2:
  //      motorSpeed(RIGHTF, input2);
  //      break;
  //    case 3:
  //      motorSpeed(RIGHTB, input2);
  //      break;
  //    case 4:
  //      servoMove(servoLeft, input2);
  //      break;
  //    case 5:
  //      servoMove(servoRight, input2);
  //      break;
  //    case 6:
  //      led_toggle();
  //      break;
  //    case 7:
  //      //BT.println("No code for this yet lol");
  //      break;
  //  }
  switch (bit1) {
    case 0:
      forward(input2);
      break;
    case 1:
      backward(input2);
      break;
    case 2:
      right_turn(input2);
      break;
    case 3:
      left_turn(input2);
      break;
    case 4:
      servoMove(servoLeft, input2);
      break;
    case 5:
      servoMove(servoRight, input2);
      break;
    case 6:
      led_toggle();
      break;
    case 7:
      precision_turn(input2);
      break;
    case 8:
      grabber(input2);
      break;
    default:
      forward(0);
  }

}



//char readByte; //stores incoming character from other device
bool justReceived = false; //stores whether a character has been recieved.
bool byteCount = false;
char i, j, a;
int k = 0;
int byte1, byte2;
String readStr;
//char readStr[3];

void loop() {


  if (justReceived) {
    BT.println(byte1);
    justReceived = false;
    read_message(byte1, byte2);
  }

  if (BT.available()) {
    if (!byteCount) {
      byte1 = BT.read() - '0';
      byteCount = true;
    } else {
      byte2 = BT.read() - '0';
      byteCount = false;
      justReceived = true;
    }
    delay(10);


  }
}

