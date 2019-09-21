# aqw-opcua-server (prototype).

## Description.

A demo OPC UA server application, **currently in development**, that fetches weather information, such as temperature, pressure, humidity, wind speed etc, from web services online through REST calls. Responses are returned in [JSON](http://json.org/) format and are processed by the application. The demo server outputs all requested information in accordance with OPC UA communication protocol.

## Libraries/APIs required:

- [open62541](https://open62541.org/);
- [C++ REST SDK](https://github.com/Microsoft/cpprestsdk);
- [Open AQ API](https://openaq.org) - air quality data;
- [DarkSky API](https://darksky.net/dev) - weather information.

### Building instructions (as of September 2019).

1) [Open62541](https://open62541.org/) library. Currently we include amalgamated version 0.3.1 files directly in this repository. No action required after cloning it. You may find v1.0rc5+ files on project website, but it has slightly different API and therefore won't work by default. Also you may find that v0.3.0 files are available through `vcpkg` and `conan` library managers, but there are various issues with them:
	- v0.3.0 is from December 2018, while v0.3.1 has various fixes and was released in July 2019;
	- vcpkg version is not build properly and requires manual adjustments like moving of .dll files etc;
	- conan default option works on MSVC 15 2017, but has compatibility issues with MSVC 16 2019;
	- conan default option doesn't work on Ubuntu and fails due to linker errors.

2) [C++ REST SDK](https://github.com/Microsoft/cpprestsdk). Simply follow the instructions. Just make sure you include `:x64-windows` triplet option or set `VCPKG_DEFAULT_TRIPLET` as `x64-windows` on windows machines because default setting is `:x86-windows` (32-bit).

3) [Open AQ API](https://openaq.org). Simply make sure website is available from your machine.

4) [DarkSky API](https://darksky.net/dev) is used to fetch weather data:
	- if you do not have an account, you will need to [create one](https://darksky.net/dev/register) in order to request an API Key;
	- after building the server with cmake open the `settings.json` file located inside `build/bin[/CONFIG]` forlder and under the `darksky_api` object you may:
		* **(REQUIRED)** replace `api_key` parameter value with `Your API key` that you received from Dark Sky;
		* change "interval_download" parameter value (in minutes) to control how often weather data is downloaded.
			
5) You can build this application with [CMake](https://cmake.org) (verified on Windows 10 and Ubuntu 18.04):

	- (**optional**) make sure [CMake v3.10+ is installed](https://cmake.org/download/) or `sudo apt install cmake`;
	- make sure [vcpkg is installed](https://github.com/microsoft/vcpkg#quick-start) (it includes a CMake by default);
	- install cpprestsdk in vcpkg: `vcpkg install cpprestsdk` (**use `:x64-windows` triplet in windows**);
	- create directory `build` in the root repository directory and switch to it: `mkdir build && cd build`
	- run cmake configure with toolchain from vcpkg: `cmake -DCMAKE_TOOLCHAIN_FILE="**your/path/to**/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release ..`;
	- build with cmake: `cmake --build . --config Release`;
	- run the server from executable directory where `settings.json` file is located.

## Project Overview.

A more detailed explanation of the project is going to be there soon...
