cmake_minimum_required(VERSION 3.10)

project(aqw-opcua-server)

find_package(cpprestsdk CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory("src")

file(GENERATE OUTPUT "$<TARGET_FILE_DIR:aqw-opcua-server>/settings.json" INPUT "settings_example.json")

#Doesn't work in VC 16 2019 correctly - ALL BUILD still selected as startup project. VC 15 2017 seems ok.
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VC_STARTUP_PROJECT aqw-opcua-server)
