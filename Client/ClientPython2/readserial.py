#!/usr/bin/python
#
#simple app to read string from serial port
#and publish via MQTT
#
#Andy Piper http://andypiper.co.uk
#2011/09/15

import serial
from MQTTSNclient import *
import os
import threading

def input_func():
    while True:
        t = input() + '\n'
        ser.write(t.encode())

if __name__ == '__main__':

	serialdev = 'COM8'
	broker = "192.168.178.84"
	port = 1884

	try:
		print "Connecting to serial port... ", serialdev
		#connect to serial port
		ser = serial.Serial(serialdev, 9600, timeout=20)
		print "Connected to Serial Port"
	except:
		print "Failed to connect serial"
		#unable to continue with no serial input
		raise SystemExit


	try:
		ser.reset_input_buffer()
		#create an mqtt client
		mypid = os.getpid()
		client_uniq = "chat_pub_"+str(mypid)
		aclient = Client(client_uniq, broker, port=1884)

		#attach MQTT callbacks
		aclient.registerCallback(Callback())
		
		#connect to broker
		aclient.connect()
		
		#remain connected to broker
		#read data from serial and publish
		x = threading.Thread(target=input_func)
		x.start()

		print "IMPORTANT: You can send messages from this mqttsn client. They have to be surrounded by \" \""
		
		while True:
			line = ser.readline()
			#split line as it contains V,temp
			#list = line.split(",")
			#second list element is temp
			#temp = list[1].rstrip()
			print line
			aclient.publish("nodex", line, qos=2)
			pass
		

	#handle list index error (i.e. assume no data received)
	#except (IndexError):
		#print "No data received within serial timeout period"
		#cleanup()
	#handle app closure
	except (KeyboardInterrupt):
		print "Interrupt received"
		cleanup()
	except (RuntimeError):
		print "uh-oh! time to die"
		cleanup()