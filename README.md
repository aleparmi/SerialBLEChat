# SerialBLEChat

Repository for NGN Project

## Establish BLE Serial Chat over MQTT-SN

1) Raspberry: Start the Broker in /SerialBLEChat/BrokerGW --> ./broker_mqtts broker.txt
2) Raspberry: Start the Python2 Client in /SerialBLEChat/Client/ClientPython2 --> python Client_local.py
3) PC: Open Serial Port on PC --> sudo rfcomm watch rfcomm0
4) Raspberry: Start the Bluetooth<-->Broker Forwarder in /SerialBLEChat/BLEMQTTSN --> node index.js
5) PC: Start the C++ Client in /SerialBLEChat/Client/ClientC++/serial/build --> sudo ./main


### Configure Serial Port

In order to open a serial terminal for the bluetooth communication between two devices (Windows - Android), follow the following steps:

1) Establish a bluetooth connection between the two devices, like you would normally do
2) If one of the two devices is a smartphone please install this app: https://play.google.com/store/apps/details?id=ptah.apps.bluetoothterminal&hl=it
3) On the Windows device try to open an ingress COM port; to do so:
  a) Go on the bluetooth options
  b) Open more bluetooth options
  c) Select the COM ports tab
  d) Add an ingress COM port and make sure that the name of the device appears in the port's description
4) Modify the script by inserting you COM port
5) Lauch the app and connect to the device from there
6) Launch the script making sure you install this library: https://pypi.org/project/pyserial/ (doc: https://pyserial.readthedocs.io/en/latest/)
7) If this setup does not work you probably have to install a software that comes with your bluetooth hardware and setup the COM ports from there

## Broker Git Repo

Check this: https://github.com/eclipse/mosquitto.rsmb

## Gateway (+ not completed Client Git repo)

Check this: https://github.com/eclipse/paho.mqtt-sn.embedded-c

## Pyserial Doc

https://pyserial.readthedocs.io/en/latest/pyserial_api.html

## Configure RaspberryPi for serial ports

https://www.youtube.com/watch?v=sY06F_sPef4&feature=youtu.be

To bind multiple serial ports, simply insert sudo rfcomm watch /dev/rfcommX where X is (0, 1, 2....)

Enhance Automation!
