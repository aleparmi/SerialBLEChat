#!/usr/bin/python

import serial
from MQTTSNclient import *
import os

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
		#create an mqttsn client
		mypid = os.getpid()
		client_uniq = "chat_pub_"+str(mypid)
		aclient = Client(client_uniq, broker, port=1884)

		#attach MQTT callbacks
		aclient.registerCallback(Callback())
		
		#connect to broker
		aclient.connect()
		
		#register topic
		topic_id = aclient.register("SerialMessageviaMQTTSN")

		print "IMPORTANT: You can send messages from this mqttsn client. They have to be surrounded by \" \""
		
		while True:
			line = ser.readline()
			print "Message received: ", line
			print "\n"
			#split line as it contains V,temp
			#list = line.split(",")
			#second list element is temp
			#temp = list[1].rstrip()
			aclient.publish(topic_id, line, qos=2)
			pass

	#handle app closure
	except (KeyboardInterrupt):
		print "Interrupt received"
		aclient.disconnect()
	except (RuntimeError):
		print "uh-oh! time to die"
		aclient.disconnect()