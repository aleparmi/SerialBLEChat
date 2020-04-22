import serial

ser = serial.Serial('COM9', 9600, timeout=10)
ser.write(b'hello')
serial.tools.list_ports