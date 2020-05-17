import serial
import threading


ser = serial.Serial('/dev/rfcomm0')

def input_func():
    while True:
        t = input() + '\n'
        ser.write(t.encode())


def main():

    if ser.isOpen() is False:
        print('Serial port is not open. Quitting...')
        quit()
    
    print('Connected to Serial port.')
    print "IMPORTANT: You can send messages from this mqttsn client. They have to be surrounded by \" \""

    x = threading.Thread(target=input_func)
    x.start()

    try:
        while True:
            print(str(ser.readline()))

    except KeyboardInterrupt:
        print('Ending Program.') 

if __name__ == '__main__':
    main()
