cmake_minimum_required(VERSION 3.13.4)
project(my_project)

include_directories(include /home/pi/Projects/SerialBLEChat/Client/ClientC++/source)

set(serial_SRCS src/serial.cc include/serial/serial.h include/serial/v8stdint.h)
set(mqttsn_SRCS /home/pi/Projects/SerialBLEChat/Client/ClientC++/source/MQTTSNPacket.c
/home/pi/Projects/SerialBLEChat/Client/ClientC++/source/MQTTSNDeserializePublish.c
/home/pi/Projects/SerialBLEChat/Client/ClientC++/source/MQTTSNSerializePublish.c
/home/pi/Projects/SerialBLEChat/Client/ClientC++/source/MQTTSNConnectClient.c
/home/pi/Projects/SerialBLEChat/Client/ClientC++/source/MQTTSNSubscribeClient.c)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

list(APPEND serial_SRCS src/impl/unix.cc)
list(APPEND serial_SRCS src/impl/list_ports/list_ports_linux.cc)

## Add serial library
add_library(serial ${serial_SRCS})
add_library(mqttsn ${mqttsn_SRCS})

target_link_libraries(serial rt)
target_link_libraries(mqttsn rt)

## Build your executable
add_executable(main /home/pi/Projects/SerialBLEChat/Client/ClientC++/source/main.cpp)
add_executable(serial_example examples/serial_example.cc)
## Link your executable against the library
target_link_libraries(main serial mqttsn)
target_link_libraries(serial_example serial)
