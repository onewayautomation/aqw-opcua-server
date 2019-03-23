# aqw-opcua-server (Prototype)

## Description

A demo OPC UA server application, **currently in development**, that fetches weather information, such as temperature, pressure, humidity, wind speed etc, from web services online through rest calls. The request's responses are returned in [JSON](http://json.org/) format and manipulated in the application. The demo server put all the information requested available in OPC UA communication protocol.

## Libraries/API required
- [open62541](https://open62541.org/)
- [C++ REST SDK](https://github.com/Microsoft/cpprestsdk)
- [Open AQ API - Air Quality Data](https://openaq.org)
- [DarkSky API - Weather](https://darksky.net/dev)

### Building instructions
- Make sure you have the two libraries [open62541](https://open62541.org/) and [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) included in your project.
- To fetch weather data, you will use the API from [DarkSky API - Weather](https://darksky.net/dev). 
	* If you do not already have an account, you will need to [create one](https://darksky.net/dev/register) in order to request an API Key.
	* In `WebService.cpp` class implementation, replace the value of `WebService::KEY_API_DARKSKY` constant with your own API KEY that you received from your registration above.
- There is only one `TODO` in the the project, in the `Application.cpp` source code, the `static bool wasitCalled` variable inside the method `requestLocations` is set to true temporarily. With this variable we limit the request of locations only for the first Country. As soon as a callback option is implemented, to detecte when the Client selects a specific Country-Location, this variable will be removed.
- Build & run!

## Project Overview

A more detail explanation about the project is going to be here soon...
