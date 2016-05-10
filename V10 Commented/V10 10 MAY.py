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

# Used for math operations in calculating joystick angle
import math 

# Determines the platform that this is running on
from operator import attrgetter
import platform

# Used to interact with the controller through the GUI
import pygame
from pygame.locals import *

# Used to map controller keys
import xinput

# Used for delays
from time import sleep

# Used to send serial messages via BT dongle
import serial

# This is the COM port that the BT dongle has been assigned for Outgoing messages
ser = serial.Serial('COM31', 9600)

# INITIALISATION CODE
__version__ = '1.0.0'
__vernum__ = tuple([int(s) for s in __version__.split('.')])
lastCommand = "none"						# Command last sent to the craft
updateCount = 0								# Used as a threshold for sending repeat messages

# GUI Initialisation function
class Struct(dict):
    def __init__(self, **kwargs):
        dict.__init__(self, **kwargs)
        self.__dict__.update(**kwargs)

# Draws the button within the GUI
def draw_button(button):
    rect = button.rect
    value = 0 if button.value else 1		# Include an indication of whether the button is being pressed
    pygame.draw.rect(screen, white, rect, value)

# Draws the joystick within the GUI
def draw_stick(stick):
	# Work out and display location of the joystick in the GUI
    ox, oy = origin = stick.rect.center
    radius = stick.rect.h
    x, y = int(round(ox + stick.x * radius)), int(round(oy + stick.y * radius))	
    pygame.draw.circle(screen, white, origin, radius, 1)	# Draw origin
    pygame.draw.circle(screen, red, (x, y), 5, 0)			# Draw current position

# Draws the front trigger buttons within the GUI
def draw_trigger(trigger):
    rect = trigger.rect
    pygame.draw.rect(screen, white, rect, 1)
    if trigger.value > 0.0: 				# If the trigger is pressed
        r = rect.copy()
        r.h = rect.h * trigger.value
        r.bottom = rect.bottom
        screen.fill(white, r)

# Draws the D pad within the GUI
def draw_hats(hats):
    pygame.draw.circle(screen, white, hats[0, 0].rect.center, 40, 1)
    for hat in hats.values():				# Iterates through each possible pushed part of the D pad
        if hat.value:						# Checks whether that quadrant is pressed
            pygame.draw.rect(screen, white, hat.rect, 0)
    pygame.draw.rect(screen, white, hats[0, 0].rect, 1)

# Feeble attempt to compensate for calibration and loose stick.
def stick_center_snap(value, snap=0.2):
	# Checks whether the new value is within the 'snap threshold'
    if value >= snap or value <= -snap:
        return value
    else:
    	# Probably loose joystick reading
        return 0.0

# This function sends the desired message through the serial BT link and waits for a response
def sendCommand(value):
    if (value.isdigit() and len(value) == 2):
        ser.write(value)
        print ser.readline();

# This function is used to read the joystick value and determine the positioning commands
# to send to the craft. It takes in an x and y joystick position, calculates the position 
# and then uses that position to determine which signal to send to the craft.
def calcTrackSpeed(x,y):

    mag = math.sqrt(x*x+y*y)						# Pythagoras to determine joystick magnitude
    angle = (math.atan2(y,x) * (180 /3.14159)) + 180 + 45 # Offset of 225 degrees to align Q4 with 0 degrees
    
    motorDir = 0
    spd = 0
    
    # Access the global variables
    global lastCommand		 
    global updateCount


    if (angle >= 0 and angle < 90):
        ''' Quadrant 4 - LEFT TURN '''
        motorDir = 3
    
    elif (angle >= 90 and angle < 180):
        '''Quadrant 3 - BACKWARDS'''
        motorDir = 1
        
    elif (angle >=180  and angle < 270):
        '''Quadrant 2 - RIGHT TURN'''
        motorDir = 2
        
    elif (angle >= 270 and angle < 360):
        '''Quadrant 1 - FORWARDS'''
        motorDir = 0

    # Used to keep track of when the message should be resent (allowing for dropping of packets)
    updateCount = updateCount + 1

    if (abs(mag) > 0.8):
        '''Motor Fast'''
        spd = 2
    elif (abs(mag) > 0.2):
        '''Motor Slow'''
        spd = 1
    else:
        '''Motor Off'''
        spd = 0

    # Format the current command as a 2 byte string to be sent
    currentCommand = "{}{}".format(motorDir,spd)

    if (lastCommand == currentCommand and updateCount > 50):
    	# Packet was probably dropped - Stop the motors
        sendCommand("00")
        lastCommand = "00"
        updateCount = 0
    elif (lastCommand != currentCommand):
    	# New command, sent immediately
        sendCommand(currentCommand)
        lastCommand = currentCommand
        updateCount = 0
    


# BEGINNING OF CODE EXECUTION FLOW

pygame.init()								# Initialise the pygame interface and GUI
pygame.joystick.init()						# Initialise the joystick

PLATFORM = platform.uname()[0].upper()		# Return the name of the computer platform
WINDOWS_PLATFORM = PLATFORM == 'WINDOWS'	# Will return true if operating on Windows
WINDOWS_XBOX_360 = False					# Not Xbox 360
JOYSTICK_NAME = ''							# Do not search for specific joystick
joysticks = xinput.XInputJoystick.enumerate_devices()		# Returns a list of joysticks
device_numbers = list(map(attrgetter('device_number'), joysticks))	# Extracts the deviCe numbers for each of the joysticks

joystick = None 							# Initialise the joystick object as empty
if device_numbers:							# If there is a joystick available
    joystick = pygame.joystick.Joystick(device_numbers[0])		# Use the first available joystick
    JOYSTICK_NAME = joystick.get_name().upper()					# Save the name of the joystick
    print('Joystick: {} using "{}" device'.format(PLATFORM, JOYSTICK_NAME))		# Provide feedback to the user
    if 'XBOX 360' in JOYSTICK_NAME and WINDOWS_PLATFORM:		# Xbox 360 Controller on Windows
        WINDOWS_XBOX_360 = True
        joystick = xinput.XInputJoystick(device_numbers[0])
        print('Using xinput.XInputJoystick')
    else:									# Any other platform or controller
        # put other logic here for handling platform + device type in the event loop
        print('Using pygame joystick')
        joystick.init()

screen = pygame.display.set_mode((640, 480))# Screen width 640 x 480
screen_rect = screen.get_rect()				# Save the GUI screen to be used		
clock = pygame.time.Clock()					# Clock reference
black = Color('black')						# Define colours to be used
white = Color('white')
red = Color('red')

# Add buttons to GUI display
button_a = Struct(rect=Rect(560, 200, 20, 20), value=0)
button_b = Struct(rect=Rect(600, 160, 20, 20), value=0)
button_x = Struct(rect=Rect(520, 160, 20, 20), value=0)
button_y = Struct(rect=Rect(560, 120, 20, 20), value=0)

# Add the bumpers to the GUI
button_left_bumper = Struct(rect=Rect(40, 80, 40, 20), value=0)
button_right_bumper = Struct(rect=Rect(560, 80, 40, 20), value=0)

# Add extra buttons to the GUI
button_back = Struct(rect=Rect(240, 160, 20, 20), value=0)
button_start = Struct(rect=Rect(400, 160, 20, 20), value=0)

# Add stick buttons to the GUI
button_left_stick = Struct(rect=Rect(60, 160, 20, 20), value=0)
button_right_stick = Struct(rect=Rect(400, 240, 20, 20), value=0)

# List of all the buttons
buttons = (
    button_a, button_b, button_x, button_y,
    button_left_bumper, button_right_bumper,
    button_back, button_start,
    button_left_stick, button_right_stick)

# Add the joystick display to the GUI
left_stick = Struct(rect=Rect(0, 0, 80, 40), x=0.0, y=0.0)
right_stick = Struct(rect=Rect(0, 0, 40, 40), x=0.0, y=0.0)
left_stick.rect.center = button_left_stick.rect.center
right_stick.rect.center = button_right_stick.rect.center

# Add the trigger to the display
left_trigger = Struct(rect=Rect(40, 40, 40, 40), value=0.0)
right_trigger = Struct(rect=Rect(560, 40, 40, 40), value=0.0)

# Adds the D pad to the GUI display
# D pad arrangement:
# (-1,  1)    (0,  1)    (1,  1)
# (-1,  0     (0,  0)    (1,  0)
# (-1, -1)    (0, -1)    (1, -1)
hats = {}							# Initialise the list of hats
hat_posx = {-1: 0, 0: 20, 1: 40}	# Define hat mapping to position
hat_posy = {1: 0, 0: 20, -1: 40}	
for y in 1, 0, -1:					# Find the quadrant of the hat selected
    for x in -1, 0, 1:
        hats[x, y] = Struct(rect=Rect(220 + hat_posx[x], 220 + hat_posy[y], 20, 20), value=0) # Update the GUI with the selected hat
which_hat = None  					# Save state

max_fps = 60						# Select sampling rate

sleep(1)							# Delay 
bytesToRead = ser.inWaiting()		# Save from serial buffer
servo = False						# Initialise the servo flag
yaxis = 0							# Initialise the x and y axis values
xaxis = 0
print ser.read(bytesToRead)			# Read previously saved bytes

while True:							# Inifite loop
    clock.tick(max_fps)				# Set the clock tick increment
    if WINDOWS_XBOX_360:
        joystick.dispatch_events()	# Get all joystick events

    for e in pygame.event.get():	# For each event found act on it based on its type
        if e.type == JOYAXISMOTION:	
            if e.axis == 2:
                left_trigger.value = e.value 				# Save the value of the event trigger
                if(e.value > 0.5 and servo == False):
                    print "ON"								# Move the Left Servo down
                    sendCommand("41")
                    servo = True;							# Update servo flag
                elif(e.value < 0.5 and servo == True):
                    print "OFF"								# Update the servo flag
                    servo = False
            elif e.axis == 5:
                right_trigger.value = e.value 				# Save the value of the event trigger
                print e.value
                if(e.value > 0.5 and servo == False):	
                    print "ON"								# Move the Right Servo down
                    sendCommand("51") 
                    servo = True;							# Update servo flag		
                elif(e.value < 0.5 and servo == True):
                    print "OFF"								# Update servo flag
                    servo = False

            elif e.axis == 0:								# Do nothing
                left_stick.y = stick_center_snap(e.value * -1)
            elif e.axis == 1:								# Do nothing
                left_stick.x = stick_center_snap(e.value)
            elif e.axis == 3:								# Using joystick - move motors
                right_stick.y = stick_center_snap(e.value * -1)
                yaxis = e.value
                calcTrackSpeed(xaxis,yaxis)
            elif e.axis == 4:								# Using joystick - move motors
                right_stick.x = stick_center_snap(e.value)
                xaxis = e.value
                calcTrackSpeed(xaxis,yaxis)
        elif e.type == JOYBUTTONDOWN:						# Pushing buttons
            buttons[e.button].value = 1
            print "Joybutton down: " + str(e.button)
            if (e.button == 5):
                sendCommand("52") 							# Move right servo up
            elif (e.button == 4):
                sendCommand("42") 							# Move left servo up
            elif(e.button == 1):
                sendCommand("60") 							# Toggle LEDS
            elif(e.button == 3):
                sendCommand("80") 							# Fully close servos
            elif(e.button == 2):
                sendCommand("81") 							# Fully open servos
        elif e.type == JOYBUTTONUP:
            buttons[e.button].value = 0						# Do nothing
        elif e.type == JOYHATMOTION:						# Precision mode - small motions
            print "Joyhatmotion: " + str(e.value)
            if (e.value == (0,1)):							# Precise forward
                sendCommand("71") 
            elif (e.value == (0,-1)):						# Precise backwards
                sendCommand("72")
            elif (e.value == (1,0)):						# Precise right turn
                sendCommand("74") 
            elif (e.value == (-1,0)):						# Precise left turn
                sendCommand("73") 
            elif (e.value == (0,0)):						# Stop
                sendCommand("70")
            # Pygame sends this to the xinput function which sends a button instead - the handler converts the button to a hat event
            if which_hat:
                hats[which_hat].value = 0
            if e.value != (0, 0):
                which_hat = e.value
                hats[which_hat].value = 1
        elif e.type == KEYDOWN:								# Exit
            if e.key == K_ESCAPE:
                quit()
        elif e.type == QUIT:								# Exit
            quit()

    # Draw/update the GUI
    screen.fill(black)
    for button in buttons:									# Draw each button
        draw_button(button)
    draw_stick(left_stick)									# Draw joysticks
    draw_stick(right_stick)
    draw_trigger(left_trigger)								# Draw triggers
    draw_trigger(right_trigger)
    draw_hats(hats)											# Draw D pad
    pygame.display.flip()									# Display
