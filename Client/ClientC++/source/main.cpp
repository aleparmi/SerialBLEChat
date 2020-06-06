/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include <string>
#include <iostream>
#include <cstdio>
#include <thread>
#include <signal.h>

// OS Specific sleep
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial/serial.h"
#include "MQTTSNPacket.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;

void my_sleep(unsigned long milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds); // 100 ms
#else
    usleep(milliseconds * 1000); // 100 ms
#endif
}

serial::Serial *serial_port;

volatile int bleConnected = 0;
volatile int mqttConnecting = 0;
volatile int mqttConnected = 0;

int rc = 0;
unsigned char buf[100];
unsigned char buf_pub[100];
int buflen = sizeof(buf);
int buflen_pub = sizeof(buf_pub);
MQTTSN_topicid topic_pub;
MQTTSNString topicstr_reg;
MQTTSN_topicid topic_sub;
int len = 0;
int dup = 0;
int qos = 0;
int retained = 0;
short packetid = 0;
unsigned short topicid, topicid_sub;
MQTTSNPacket_connectData options = MQTTSNPacket_connectData_initializer;

struct {
    string port = "COM6";
    unsigned long baud = 9600;
    char dev_id[20] = "PC_Ale";
    char sub[20] = "Rasp_Ale";
    int timeout = 250;
}settings;

#define BLE_CHUNK_SIZE 18

int mqttsn_connect();
void publish(char* payload);
void close();

void open_serial()
{
    try {
        int c = 0;
        serial_port->flush();
        if (serial_port->isOpen() == false) serial_port->open();
        cout << "Serial port open!" << endl;
        bleConnected = 1;
        do {
            if (c == -1) cout << "Trying to reconnect" << endl;
            c = mqttsn_connect();
        }while (c == -1);
    }
    catch (exception& e) {
        cerr << "Impossible to esatblish the MQTTSN connection: " << e.what() << endl;
    }
}

void write() {
    while (true) {
        char mypayload[50];

        std::cout << "Message: ";
        std::cin >> (char*) mypayload;

        publish(mypayload);
    }
}

int read(unsigned char* buf, int count) {

    rc = serial_port->read(buf, count);

    return rc;
}

// for some reason uart->send does not seem to automatically write to the UART in chunks?
// doing it manually instead...
int send_in_chunks(unsigned char *buffer, size_t length, size_t chunk_size)
{
    SSIZE_T n = 0;
    const unsigned char *p = buffer;
    while (length > 0)
    {
        n = serial_port->write(p, min(length, chunk_size));
        std::cout << "Message sent!" << endl;
        if (n <= 0) break;
        p += n;
        length -= n;
    }
    return (n <= 0) ? -1 : 0;
}

/** MQTT-SN transport helpers **/
int transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
    int rc = send_in_chunks(buf, buflen, BLE_CHUNK_SIZE);
    return rc;
}

int mqttsn_connect()
{
    if (mqttConnecting) {
        return -1;
    }

    /*Connection phase*/
    mqttConnecting = 1;
    options.clientID.cstring = settings.dev_id;
    len = MQTTSNSerialize_connect(buf, buflen, &options);
    rc = transport_sendPacketBuffer(buf, len);

    /* wait for connack */
    my_sleep(2000);
    rc = MQTTSNPacket_read(buf, buflen, read);
    if (rc == MQTTSN_CONNACK)
    {
        int connack_rc = -1;

        if (MQTTSNDeserialize_connack(&connack_rc, buf, buflen) != 1 || connack_rc != 0)
        {
            mqttConnecting = 0;
            cout << "MQTTSN Connection failed!" << endl;
            return -1;
        }

        cout << "MQTTSN Connection succeded!" << endl;
    }
    else {
        mqttConnecting = 0;
        cout << "CONNACK not received!" << endl;
        return -1;
    }

    /* register topic name */
    int packetid = 1;
    topicstr_reg.cstring = settings.dev_id;
    cout << "Topic registration name: " << topicstr_reg.cstring << endl;
    topicstr_reg.lenstring.len = int(strlen(settings.dev_id));
    len = MQTTSNSerialize_register(buf, buflen, 0, packetid, &topicstr_reg);
    rc = transport_sendPacketBuffer(buf, len);

    my_sleep(2000);
    rc = MQTTSNPacket_read(buf, buflen, read);
    if (rc == MQTTSN_REGACK)     /* wait for regack */
    {
        unsigned short submsgid;
        unsigned char returncode;

        rc = MQTTSNDeserialize_regack(&topicid, &submsgid, &returncode, buf, buflen);
        if (returncode != 0)
        {
            mqttConnecting = 0;
            cout << "Topic registration failed!" << endl;
            return -1;
        }
        else {
            mqttConnecting = 0;
            cout << "Topic registration succeded!" << endl;
        }
    }
    else {
        mqttConnecting = 0;
        cout << "REGACK not received!" << endl;
        return -1;
    }

    /*Subscribe to topic*/
    packetid = 2;
    topic_sub.data.long_.name = settings.sub;
    cout << "Topic subscription name: " << topic_sub.data.long_.name << endl;
    topic_sub.data.long_.len = int(strlen(topic_sub.data.long_.name));
    len = MQTTSNSerialize_subscribe(buf, buflen, dup, qos, packetid, &topic_sub);
    rc = transport_sendPacketBuffer(buf, len);

    my_sleep(2000);
    rc = MQTTSNPacket_read(buf, buflen, read);
    if (rc == MQTTSN_SUBACK)     /* wait for suback */
    {
        unsigned short submsgid;
        unsigned char returncode;

        rc = MQTTSNDeserialize_suback(&qos, &topicid_sub, &submsgid, &returncode, buf, buflen);
        if (returncode != 0)
        {
            mqttConnecting = 0;
            cout << "Subscription failed!" << endl;
            return -1;
        }
        else {
            mqttConnecting = 0;
            mqttConnected = 1;
            cout << "Subscription succeded!" << endl;
            return rc;
        }
    }
    else {
        cout << "SUBACK not received" << endl;
        mqttConnecting = 0;
        return -1;
    }
}

void my_handler(sig_atomic_t  s) {
    cout << "Caught signal %d" << s << endl;
    
    close();

    exit(1);
}

void close() {

    bleConnected = 0;
    mqttConnected = 0;
}

// publishes an MQTT-SN message. 
// keep in mind that BLE payloads over >19bytes might not work that well so try to 
// stay concise!
void publish(char * payload) {
    if (mqttConnected == 0) {
        // try to reconnect:
        int rc = mqttsn_connect();
        if (rc != 0) 
            return;
    } else {
        /* publish with short name */
        topic_pub.type = MQTTSN_TOPIC_TYPE_NORMAL;
        topic_pub.data.id = topicid;

        len = MQTTSNSerialize_publish(buf, buflen, dup, qos, retained, packetid,
                topic_pub, (unsigned char *) payload, int(strlen(payload)));
        rc = transport_sendPacketBuffer(buf, len);
    }
}

void receive(){
    unsigned char dup_pub, retained_pub, * payload_pub;
    unsigned short packetid_pub;
    MQTTSN_topicid topicid_pub;
    int payloadlen_pub = 0;

    while (true) {
        if (mqttConnected == 0) {
            // try to reconnect:
            int rc = mqttsn_connect();
            if (rc != 0) {
                cout << "Reconnection failed" << endl;
                return;
            }
        }
        else {
            /* receive incoming data */
            rc = MQTTSNPacket_read(buf_pub, buflen_pub, read);
            if (rc == MQTTSN_PUBLISH) {
                rc = MQTTSNDeserialize_publish(&dup_pub, &qos, &retained_pub, &packetid_pub, &topicid_pub,
                    &payload_pub, &payloadlen_pub, buf_pub, buflen_pub);
                if (rc == 1) {
                    cout << "Message: " << payload_pub << endl;
                }
                else {
                    cout << "Error in the deserialization of the received message" << endl;
                }
            }
        }
    }
}

int main()
{
    serial_port = new serial::Serial(settings.port, settings.baud, serial::Timeout::simpleTimeout(settings.timeout));

    open_serial();

    signal(SIGINT, my_handler);
    
    std::thread r(receive);
    std::thread w(write);

    w.join();
    r.join();

    close();

    return 0;
}
