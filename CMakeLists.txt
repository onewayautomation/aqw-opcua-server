cmake_minimum_required(VERSION 3.10)

project(aqw-opcua-server)

find_package(cpprestsdk CONFIG REQUIRED)

file(COPY "settings_example.json" DESTINATION ".")
file(RENAME ${CMAKE_BINARY_DIR}/settings_example.json ${CMAKE_BINARY_DIR}/settings.json)

add_subdirectory("aqw-opcua-server")