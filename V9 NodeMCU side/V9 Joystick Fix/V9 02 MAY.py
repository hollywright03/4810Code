"""test.py - integrating xinput.XInputJoystick with pygame for Windows + Xbox 360 controller

Windows Xbox 360 cannot use pygame events for the left and right trigger. The axis doesn't come through distinctly.
This alternative corrects that issue, and adds functions unique to the Xbox controller.

General approach:

1. Detect joysticks.
2. Detect Windows.
3. Detect Xbox 360 controller.
4. Set up the joystick device platform+controller option.

For non-Windows controllers use pygame's joystick.Joystick as usual.

For Xbox 360 controller on Windows use xinput.XInputJoystick:

1. Do "joystick = xinput.XInputJoystick()".
2. Do "if WINDOWS_XBOX_360: joystick.dispatch_events()" each game tick to poll the device.
3. Handle pygame events as usual.

References:
https://github.com/r4dian/Xbox-360-Controller-for-Python
http://support.xbox.com/en-US/xbox-360/accessories/controllers
"""

"""
    MESSAGES DECODED:
    Messages sent to serial are 2 byte serial messages.
    The first byte ranges from 0-6 where the value is set as below
    +----------+--------------------------+
    | 1st Byte |       Pin affected       |
    +----------+--------------------------+
    |        0 | Forward                  |
    |        1 | Backward                 |
    |        2 | Right                    |
    |        3 | Left                     |
    |        4 | Left Servo               |
    |        5 | Right Servo              |
    |        6 | LED TOGGLE               |
    +----------+--------------------------+
    
    The second byte ranges from 0-2, with the value set as below.
    +----------+-----------------+-----------------+
    | 2nd Byte | 1st byte is 0-3 | 1st byte is 4-5 |
    +----------+-----------------+-----------------+
    |        0 | Stop motor      | No change       |
    |        1 | Speed = slow    | Increase angle  |
    |        2 | Speed = fast    | Decrease angle  |
    +----------+-----------------+-----------------+
"""

import math 

from operator import attrgetter
import platform

import pygame
from pygame.locals import *

import xinput

#ADDED CODE BY CAMERON WEBER
from time import sleep
import serial
ser = serial.Serial('COM31', 9600)
#</ADDED CODE>

# INITIALISATION CODE, NO CHANGE REQUIRED
__version__ = '1.0.0'
__vernum__ = tuple([int(s) for s in __version__.split('.')])
lastCommand = "none"
updateCount = 0

class Struct(dict):

    def __init__(self, **kwargs):
        dict.__init__(self, **kwargs)
        self.__dict__.update(**kwargs)


def draw_button(button):
    rect = button.rect
    value = 0 if button.value else 1
    pygame.draw.rect(screen, white, rect, value)


def draw_stick(stick):
    ox, oy = origin = stick.rect.center
    radius = stick.rect.h
    x, y = int(round(ox + stick.x * radius)), int(round(oy + stick.y * radius))
    pygame.draw.circle(screen, white, origin, radius, 1)
    pygame.draw.circle(screen, red, (x, y), 5, 0)


def draw_trigger(trigger):
    rect = trigger.rect
    pygame.draw.rect(screen, white, rect, 1)
    if trigger.value > 0.0:
        r = rect.copy()
        r.h = rect.h * trigger.value
        r.bottom = rect.bottom
        screen.fill(white, r)


def draw_hats(hats):
    pygame.draw.circle(screen, white, hats[0, 0].rect.center, 40, 1)
    for hat in hats.values():
        if hat.value:
            pygame.draw.rect(screen, white, hat.rect, 0)
    pygame.draw.rect(screen, white, hats[0, 0].rect, 1)


def stick_center_snap(value, snap=0.2):
    # Feeble attempt to compensate for calibration and loose stick.
    if value >= snap or value <= -snap:
        return value
    else:
        return 0.0

# ADDED CODE BY CAMERON WEBER
# CODE TO SEND A COMMAND BASED ON A VALUE WE'VE DETERMINED
def sendCommand(value):
    #print value
    if value == 1:
        #print "RED"
        ser.write("60")
        print ser.readline()
    elif (value == "SERRDY" or value == "SERRDN" or value == "SERRUY" or value == "SERRUN"):
        ser.write(value)
        print ser.readline()
    elif (value == "SERLDY" or value == "SERLDN" or value == "SERLUY" or value == "SERLUN"):
        ser.write(value)
        print ser.readline()
    elif ("STP" in value):
        ser.write(value)
        print ser.readline()
    elif (value.isdigit() and len(value) == 2):
        ser.write(value)
        #print "Value Changed" + value
        print ser.readline();

# NEED CHANGING, ONLY CARE ABOUT S-SLOW AND F-FAST
def calcTrackSpeed(x,y,lastMag,lastMotorDir):
    #print "{}:{}".format(round(x,2),round(y,2))
    mag = math.sqrt(x*x+y*y)
    angle = (math.atan2(y,x) * (180 /3.14159)) + 180 + 45 #offset of 225 degrees
    #print "{}:{}".format(round(mag,2),round(angle,2))
    motorDir = 0
    spd = 0
    direction = 0
    global lastCommand
    global updateCount
    if(angle >= 0 and angle < 90):
        ''' Quadrant 4 - LEFT TURN '''
        #print "quadrant4"
        motorDir = 3
        
    elif(angle >= 90 and angle < 180):
        '''Quadrant 3 - BACKWARDS'''
        #print "quadrant3"
        motorDir = 1
        
    elif(angle >=180  and angle < 270):
        '''Quadrant 2 - RIGHT TURN'''
        #print "quadrant 2"
        motorDir = 2
        
    elif(angle >= 270 and angle < 360):
        '''Quadrant 1 - FORWARDS'''
        #print "quadrant 1"
        #print "{}".format(mag)
        motorDir = 0

    updateCount = updateCount + 1

    #if(abs(mag - lastMag) > sendThresh)
    lastMag = mag
    lastMotorDir = motorDir

    if(abs(mag) > 0.8):
        '''Fast'''
        spd = 2
    elif (abs(mag) > 0.2):
        '''Slow'''
        spd = 1
    else:
        '''Off'''
        spd = 0

    currentCommand = "{}{}".format(motorDir,spd)

    if (lastCommand == currentCommand and updateCount > 50):
        sendCommand("00")
        lastCommand = "00"
        updateCount = 0
    elif (lastCommand != currentCommand):
        sendCommand(currentCommand)
        lastCommand = currentCommand
        updateCount = 0

    return [lastMag, lastMotorDir]
    

#</ADDED CODE>

pygame.init()
pygame.joystick.init()

# Initialize a joystick object: grabs the first joystick
PLATFORM = platform.uname()[0].upper()
WINDOWS_PLATFORM = PLATFORM == 'WINDOWS'
WINDOWS_XBOX_360 = False
JOYSTICK_NAME = ''
joysticks = xinput.XInputJoystick.enumerate_devices()
device_numbers = list(map(attrgetter('device_number'), joysticks))
#device_numbers = [0]
joystick = None
if device_numbers:
    joystick = pygame.joystick.Joystick(device_numbers[0])
    JOYSTICK_NAME = joystick.get_name().upper()
    print('Joystick: {} using "{}" device'.format(PLATFORM, JOYSTICK_NAME))
    if 'XBOX 360' in JOYSTICK_NAME and WINDOWS_PLATFORM:
        WINDOWS_XBOX_360 = True
        joystick = xinput.XInputJoystick(device_numbers[0])
        print('Using xinput.XInputJoystick')
    else:
        # put other logic here for handling platform + device type in the event loop
        print('Using pygame joystick')
        joystick.init()

screen = pygame.display.set_mode((640, 480))
screen_rect = screen.get_rect()
clock = pygame.time.Clock()
black = Color('black')
white = Color('white')
red = Color('red')

# button display
button_a = Struct(rect=Rect(560, 200, 20, 20), value=0)
button_b = Struct(rect=Rect(600, 160, 20, 20), value=0)
button_x = Struct(rect=Rect(520, 160, 20, 20), value=0)
button_y = Struct(rect=Rect(560, 120, 20, 20), value=0)
button_left_bumper = Struct(rect=Rect(40, 80, 40, 20), value=0)
button_right_bumper = Struct(rect=Rect(560, 80, 40, 20), value=0)
button_back = Struct(rect=Rect(240, 160, 20, 20), value=0)
button_start = Struct(rect=Rect(400, 160, 20, 20), value=0)
button_left_stick = Struct(rect=Rect(60, 160, 20, 20), value=0)
button_right_stick = Struct(rect=Rect(400, 240, 20, 20), value=0)
buttons = (
    button_a, button_b, button_x, button_y,
    button_left_bumper, button_right_bumper,
    button_back, button_start,
    button_left_stick, button_right_stick)

# stick display
left_stick = Struct(rect=Rect(0, 0, 80, 40), x=0.0, y=0.0)
right_stick = Struct(rect=Rect(0, 0, 40, 40), x=0.0, y=0.0)
left_stick.rect.center = button_left_stick.rect.center
right_stick.rect.center = button_right_stick.rect.center

# trigger display
left_trigger = Struct(rect=Rect(40, 40, 40, 40), value=0.0)
right_trigger = Struct(rect=Rect(560, 40, 40, 40), value=0.0)

# hat display
# arrangement:
# (-1,  1)    (0,  1)    (1,  1)
# (-1,  0     (0,  0)    (1,  0)
# (-1, -1)    (0, -1)    (1, -1)
hats = {}
hat_posx = {-1: 0, 0: 20, 1: 40}
hat_posy = {1: 0, 0: 20, -1: 40}
for y in 1, 0, -1:
    for x in -1, 0, 1:
        hats[x, y] = Struct(rect=Rect(220 + hat_posx[x], 220 + hat_posy[y], 20, 20), value=0)
which_hat = None  # save state

max_fps = 60

sleep(1)
bytesToRead = ser.inWaiting()
servo = False
yaxis = 0
xaxis = 0
print ser.read(bytesToRead)
#lastYValue = 0
#lastLValue = 0
#lastRValue = 0
lastValues = [0,0]
sendThresh = 0.2

while True:
    clock.tick(max_fps)
    if WINDOWS_XBOX_360:
        joystick.dispatch_events()

    for e in pygame.event.get():
        #print('event: {}'.format(pygame.event.event_name(e.type)))
        if e.type == JOYAXISMOTION:
            #print('JOYAXISMOTION: axis {}, value {}'.format(e.axis, e.value))
            if e.axis == 2:
                left_trigger.value = e.value
                #print e.value
                if(e.value > 0.5 and servo == False):
                    print "ON"
                    sendCommand("41") #ServoLeftDownYes
                    servo = True;
                elif(e.value < 0.5 and servo == True):
                    print "OFF"
                    #sendCommand("SERLDN") #ServoLeftDownNo
                    servo = False
            elif e.axis == 5:
                right_trigger.value = e.value
                print e.value
                if(e.value > 0.5 and servo == False):
                    print "ON"
                    sendCommand("51") #ServoRightDownYes
                    servo = True;
                elif(e.value < 0.5 and servo == True):
                    print "OFF"
                    #sendCommand("SERRDN") #ServoRightDownNo
                    servo = False

            elif e.axis == 0:
                left_stick.y = stick_center_snap(e.value * -1)
            elif e.axis == 1:
                left_stick.x = stick_center_snap(e.value)
            elif e.axis == 3:
                right_stick.y = stick_center_snap(e.value * -1)
                yaxis = e.value
                lastValues = calcTrackSpeed(xaxis,yaxis,lastValues[0],lastValues[1])
                """if(abs(e.value - lastYValue) > sendThresh):
                    lastYValue = e.value
                    print round(e.value,1)
                    sendCommand("STP{}".format(round(e.value,1)))"""
            elif e.axis == 4:
                right_stick.x = stick_center_snap(e.value)
                xaxis = e.value
                lastValues = calcTrackSpeed(xaxis,yaxis,lastValues[0],lastValues[1])
        elif e.type == JOYBUTTONDOWN:
            print('JOYBUTTONDOWN: button {}'.format(e.button))
            buttons[e.button].value = 1
#ADDED CODE BY CAMERON WEBER
            print "Joybutton down: " + str(e.button)
            if (e.button == 5):
                sendCommand("52") #ServoRightUpYes
            elif (e.button == 4):
                sendCommand("42") #ServoLeftUpYes
            elif(e.button == 1):
                sendCommand("60") #LED Toggle
            elif(e.button == 3):
                sendCommand("80") #Full servo close
            elif(e.button == 2):
                sendCommand("81") #Full servo open
#</ADDED CODE>  
        elif e.type == JOYBUTTONUP:
            #if(e.button == 5):
                #sendCommand("SERRUN") #ServoRightUpNo
            #elif(e.button == 4):
                #sendCommand("SERLUN") #ServoLeftUpNo
            #print('JOYBUTTONUP: button {}'.format(e.button))
            buttons[e.button].value = 0
        elif e.type == JOYHATMOTION:
            print "Joyhatmotion: " + str(e.value)
            if (e.value == (0,1)):
                sendCommand("71") #precise forward
            elif (e.value == (0,-1)):
                sendCommand("72") #precise back
            elif (e.value == (1,0)):
                sendCommand("74") #precise right
            elif (e.value == (-1,0)):
                sendCommand("73") #precise left
            elif (e.value == (0,0)):
                sendCommand("70") #precise stop
            # pygame sends this; xinput sends a button instead--the handler converts the button to a hat event
            #print('JOYHATMOTION: joy {} hat {} value {}'.format(e.joy, e.hat, e.value))
            if which_hat:
                hats[which_hat].value = 0
            if e.value != (0, 0):
                which_hat = e.value
                hats[which_hat].value = 1
        elif e.type == KEYDOWN:
            if e.key == K_ESCAPE:
                quit()
        elif e.type == QUIT:
            quit()

    # draw the controls
    screen.fill(black)
    for button in buttons:
        draw_button(button)
    draw_stick(left_stick)
    draw_stick(right_stick)
    draw_trigger(left_trigger)
    draw_trigger(right_trigger)
    draw_hats(hats)
    pygame.display.flip()
