cmake_minimum_required(VERSION 3.14)

project(aqw-opcua-server CXX)

find_package(open62541 CONFIG REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)

add_subdirectory("aqw-opcua-server")

