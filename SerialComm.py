import serial

ser = serial.Serial('COM9', 9600, timeout=1)
ser.write(b'hello')