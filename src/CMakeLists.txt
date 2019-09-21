#using 3.10 because of Ubuntu installation
cmake_minimum_required(VERSION 3.10)

set(headers
    "CountryData.h"
    "LocationData.h"
    "open62541.h"
    "Settings.h"
    "WeatherData.h"
    "WebService.h"
)

set(sources
    "Application.cpp"
    "CountryData.cpp"
    "LocationData.cpp"
    "open62541.c"
    "Settings.cpp"
    "WeatherData.cpp"
    "WebService.cpp"
)

source_group("Headers" FILES ${headers})
source_group("Sources" FILES ${sources})

add_executable(aqw-opcua-server ${headers} ${sources})

target_link_libraries(aqw-opcua-server PRIVATE cpprestsdk::cpprest)

if (WIN32)
    target_link_libraries(aqw-opcua-server PRIVATE ws2_32)
endif ()