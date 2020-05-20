#!/usr/bin/python

from MQTTSNclient import *
import os

if __name__ == '__main__':

	broker = "127.0.0.1"
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
		
		#register topic
		topic_id = aclient.register("raspberry")
		
		#subscribe to topic
		rc, topic = aclient.subscribe("pcWindows")
		
		while True:
			print "\n"
			line = raw_input("Message: ")
			aclient.publish(topic_id, line, qos=2)
			pass
	
	#handle app closure
	except RuntimeError:
		print "uh-oh! time to die"
		aclient.unsubscribe("pcWindows")
		aclient.disconnect() 	
	except KeyboardInterrupt:
		aclient.unsubscribe("pcWindows")
		aclient.disconnect()
		print "Interrupt received"