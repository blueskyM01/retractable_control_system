cmake_minimum_required(VERSION 3.5)
PROJECT(RETRACTABLE_SENSOR_LIB)

MESSAGE(STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})

# include_directories(/home/psa/aQC/retractable_control_system)

ADD_EXECUTABLE(main main.cpp rsp.cpp rsp.h)
ADD_EXECUTABLE(laser_simulator laser_simulator.cpp)
ADD_EXECUTABLE(websocket_client_test websocket_client_test.cpp)

TARGET_LINK_LIBRARIES(main wiringPi)
TARGET_LINK_LIBRARIES(laser_simulator wiringPi)
TARGET_LINK_LIBRARIES(websocket_client_test wiringPi)