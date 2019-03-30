# aqw-opcua-server (Prototype)

## Description

A demo OPC UA server application, **currently in development**, that fetches weather information, such as temperature, pressure, humidity, wind speed etc, from web services online through rest calls. The request's responses are returned in [JSON](http://json.org/) format and manipulated in the application. The demo server put all the information requested available in OPC UA communication protocol.

## Libraries/API required
- [open62541](https://open62541.org/)
- [C++ REST SDK](https://github.com/Microsoft/cpprestsdk)
- [Open AQ API - Air Quality Data](https://openaq.org)
- [DarkSky API - Weather](https://darksky.net/dev)

### Building instructions
- Make sure you have the two libraries [open62541](https://open62541.org/) and [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) included in your project.It is expected that amalgamated open62541 header and source files are located at folder ..\..\open62541-Single-File and C++ REST SDK - under vcpkg folder ..\..\vcpkg\installed\x64-windows, relatively to the VC project folder.
- To fetch weather data, you will use the API from [DarkSky API - Weather](https://darksky.net/dev). 
	* If you do not already have an account, you will need to [create one](https://darksky.net/dev/register) in order to request an API Key.
	* In `WebService.cpp` class implementation, replace the value of `WebService::KEY_API_DARKSKY` constant with your own API KEY that you received from your registration above. You can go directly to this constant location searching for TODO in the project.
- Check the `TODO` in the `WebService.cpp` source code,  the constant `INTERVAL_DOWNLOAD_WEATHER_DATA` controls the interval in minutes of the download of weather data.
- Build & run!

## Project Overview

A more detail explanation about the project is going to be here soon...
