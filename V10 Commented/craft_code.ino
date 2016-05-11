#include <Servo.h>
#include <SoftwareSerial.h>

/* DC Motor variables. */
#define LEFTF 4 //D2
#define LEFTB 5 //D1
#define RIGHTF 2 //D4
#define RIGHTB 0 //D3

/* Servo variables. */
Servo servoLeft; Servo servoRight;
#define LOPEN = 125;
#define ROPEN = 55;
#define LCLOSE = 5;
#define RCLOSE = 175;

/* LED array variables. */
#define LEDPIN 16; //D0
bool ledOn;

/* Set up the bluetooth RX and TX pins. */
SoftwareSerial BT(13, 15);

void setup() {
  /* Initialise the servos. */
  servoLeft.attach(12); // D6
  servoRight.attach(14); // D5
  servoLeft.write(LCLOSE);
  servoRight.write(RCLOSE);

  BT.begin(9600);
  BT.println("Hello from Computer");

  /* Set up motor output pins. */
  pinMode(LEFTF, OUTPUT);
  pinMode(LEFTB, OUTPUT);
  pinMode(RIGHTF, OUTPUT);
  pinMode(RIGHTB, OUTPUT);

  /* Set up LED output pin. */
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  ledOn = true;
}

/*
   Function:  motor_speed
   ----------------------
   Sets the speed for the given motor.

   motorPin:  The motor to set the speed for.
   spd:       The speed. 0 is off, 1 is slow, 2 is fast and 3 is precision.

   returns:   Nil.
*/
void motor_speed(int motorPin, int spd) {
  /* Turn off opposing direction before setting speed. */
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
  case default:
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
}

/*
   Function:  servo_move
   ---------------------
   Moves the specified servo by diff amount in the direction specified.

   servo:   The servo to move. 4 for the left, 5 for the right.
   dir:     The direction to move the servo. 0 is do nothing,
            1 is towards open, 2 is towards closed.

   returns: Nil.
*/
void servo_move(int servo, int dir) {
  int diff = 60;
  int pos;

  if (servo == 4) {
    /* Move the left servo. */
    pos = servoLeft.read();
    switch (dir) {
      case 0:
        break;
      case 1:
        if ((pos + diff) > LOPEN) {
          servoLeft.write(LOPEN);
        } else {
          servoLeft.write(pos + diff);
        }
        break;
      case 2:
        if ((pos - diff) < LCLOSE) {
          servoLeft.write(LCLOSE);
        } else {
          servoLeft.write(pos - diff);
        }
        break;
    case default:
        break;
    }
  } else if (servo == 5) {
    /* Move the right servo. */
    pos = servoRight.read();
    switch (dir) {
      case 0:
        break;
      case 1:
        if ((pos - diff) > ROPEN) {
          servoRight.write(ROPEN);
        } else {
          servoRight.write(pos + diff);
        }
        break;
      case 2:
        if ((pos - diff) < RCLOSE) {
          servoRight.write(RCLOSE);
        } else {
          servoRight.write(pos - diff);
        }
        break;
    }
  }
}

/*
 * Function:  open_grabber
 * -----------------------
 * Opens the grabber fully.
 * 
 * No inputs.
 * 
 * returns: Nil.
 */
void open_grabber() {
  servoRight.write(ROPEN);
  servoLeft.write(LOPEN);
}

/*
 * Function:  close_grabber
 * ------------------------
 * Closes the grabber fully.
 * 
 * No inputs.
 * 
 * returns: Nil.
 */
void close_grabber() {
  servoRight.write(RCLOSE);
  servoLeft.write(LCLOSE);
}

/*
 * Function:  led_toggle
 * ---------------------
 * Toggles the LED array between off and on.
 * 
 * No inputs.
 * 
 * returns: Nil.
 */
void led_toggle() {
  if (ledOn) {
    digitalWrite(LEDPIN, LOW);
    ledOn = false;
  } else {
    digitalWrite(LEDPIN, HIGH);
    ledOn = true;
  }
}

/*
 * Function:  forward
 * ------------------
 * Moves both motors forward at the specified speed using the 
 * motor_speed function.
 * 
 * spd:   The speed to pass to motor_speed.
 * 
 * returns: Nil.
 */
void forward(int spd) {
  motor_speed(LEFTF, spd);
  motor_speed(RIGHTF, spd);
}


/*
 * Function:  backward
 * -------------------
 * Moves both motors backward at the specified speed using the 
 * motor_speed function.
 * 
 * spd:   The speed to pass to motor_speed.
 * 
 * returns: Nil.
 */
void backward(int spd) {
  motor_speed(LEFTB, spd);
  motor_speed(RIGHTB, spd);
}


/*
 * Function:  left_turn
 * --------------------
 * Move the motors in opposite directions to turn left at the specified
 * speed using the motor_speed function.
 * 
 * spd:   The speed to pass to motor_speed.
 * 
 * returns: Nil.
 */
void left_turn(int spd) {
  motor_speed(LEFTB, spd);
  motor_speed(RIGHTF, spd);
}


/*
 * Function:  rightt_turn
 * ----------------------
 * Move the motors in opposite directions to turn right at the specified
 * speed using the motor_speed function.
 * 
 * spd:   The speed to pass to motor_speed.
 * 
 * returns: Nil.
 */
void right_turn(int spd) {
  motor_speed(LEFTF, spd);
  motor_speed(RIGHTB, spd);
}

/*
 * Function:  stop_motor
 * ---------------------
 * Stops both motors using the motor_speed function.
 * 
 * No inputs.
 * 
 * returns: Nil.
 */
void stop_motor() {
  motor_speed(LEFTF, 0);
  motor_speed(RIGHTF, 0);
}

/*
 * Function:  precision_turn
 * -------------------------
 * Move the motors in the direction specified at very slow speed using the
 * direction functions.
 * 
 * dir:   The direction to turn.
 * 
 * returns: Nil.
 */
void precision_turn(int dir) {
  switch (dir) {
    case 0:
      stop_motor();
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

/*
 * Function:  grabber
 * ------------------
 * Toggles the grabber between open and closed using the grabber functions.
 * 
 * No inputs.
 * 
 * returns: Nil.
 */
void grabber(int cmd) {
  if (cmd == 0) {
    close_grabber();
  } else if (cmd == 1) {
    open_grabber();
  }
}

/*
 * Function:  read_message
 * -----------------------
 * Interprets the incoming message and calls the correct function.
 * 
 * input1:  The first byte of the incoming bluetooth message.
 *          Specifies the component.
 * input2:  The second byte of the incoming bluetooth message.
 *          Specifies speed, position or direction.
 * 
 * returns: Nil.
 */
void read_message(int input1, int input2) {
  switch (input1) {
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
      servo_move(input1, input2);
      break;
    case 5:
      servo_move(input1, input2);
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

bool justReceived = false; /* Stores whether a full message has been recevied. */
bool byteCount = false; /* Stores whether a character has already been received. */
int byte1, byte2; /* Stores the received characters. */

/*
 * Function:  loop
 * ---------------------
 * Continuously read the bluetooth message and send it to read_message for 
 * interpretation.
 * 
 * No inputs.
 * 
 * returns: Nil.
 */
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

