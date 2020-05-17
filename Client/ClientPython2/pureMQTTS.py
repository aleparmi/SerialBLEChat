#!/usr/bin/python

from MQTTSNclient import *
import os
from time import sleep


if __name__ == '__main__':

	broker = "192.168.178.84"
	port = 1884

	try:
		#create an mqtt client
		mypid = os.getpid()
		client_uniq = "chat_pub_"+str(mypid)
		aclient = Client(client_uniq, broker, port=1884)

		#attach MQTT callbacks
		aclient.registerCallback(Callback())
		
		#connect to broker
		aclient.connect()
		
		#subscribe to topic
		#rc, topic = aclient.subscribe("nodex")
		
		while True:
			print "\n"
			sleep(0.2)
			line = raw_input("Message: ")
			aclient.publish("nodex", line, qos=2)
			pass
	
	#handle app closure
	except (KeyboardInterrupt):
		print "Interrupt received"
		aclient.unsubscribe("nodex")
		aclient.disconnect()
		cleanup()
	except (RuntimeError):
		print "uh-oh! time to die"
		aclient.unsubscribe("nodex")
		aclient.disconnect()
		cleanup()