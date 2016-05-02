import serial
import timeit
ser = serial.Serial('COM10', 9600)

def send0():
    ser.write("0")
    print ser.readline()

def send1():
    ser.write("1")
    print ser.readline()

def raveParty():
    count = 20
    while(count>0):
        send0()
        send1()
        count -= 1

def test():
    send0()
    send1()
    send0()
    send1()
    return 1
