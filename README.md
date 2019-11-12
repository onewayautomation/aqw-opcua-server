# aqw-opcua-server

## Description

A demo OPC UA server application, **currently in development**, that fetches weather information, such as temperature, pressure, humidity, wind speed etc, from web services online through REST calls. Responses are returned in [JSON](http://json.org/) format and are processed by the application. The demo server outputs all requested information in accordance with OPC UA communication protocol.

## Libraries/APIs required

* [open62541](https://open62541.org/);
* [C++ REST SDK](https://github.com/Microsoft/cpprestsdk);
* [Open AQ API](https://openaq.org) - air quality data;
* [DarkSky API](https://darksky.net/dev) - weather information.

## Docker image

Docker image is available at [Docker Hub](https://hub.docker.com/r/ehnat0n/weather-server).

Server expects `settings.json` file with valid Dark Sky API key inside next to executable (see Dark Sky API section below). You can mount file to file directly from host machine. To use a different location you need to pass it as an argument at runtime.

Server runs on `48484` port by default.

Usage example (`settings.json` file in `/home/dev` dir on host machine, forwarding `48484` port):

* `docker pull ehnat0n/weather-server`
* `docker run -v ~/dev/settings.json:/opt/weather-server/settings.json -p 48484:48484 ehnat0n/weather-server`

If you are willing to build docker image by yourself checkout sample files/options in [dockerfiles folder](dockerfiles/README.md).

## Building instructions (as of September 2019)

1. [Open62541](https://open62541.org/) library. Currently we include amalgamated version 0.3.1 files directly in this repository. No action required after cloning it. You may find v1.0rc5+ files on project website,but it has slightly different API and therefore won't work by default. Also you may find that v0.3.0 files are available through `vcpkg` and `conan` library managers, but there are various issues with them:

    * v0.3.0 is from December 2018, while v0.3.1 has various fixes and was released in July 2019;
    * vcpkg version is not build properly and requires manual adjustments like moving of .dll files etc;
    * conan default option works on MSVC 15 2017, but has compatibility issues with MSVC 16 2019;
    * conan default option doesn't work on Ubuntu and fails due to linker errors.

2. [C++ REST SDK](https://github.com/Microsoft/cpprestsdk). Simply follow the instructions. Just make sure you include `:x64-windows` triplet option or set `VCPKG_DEFAULT_TRIPLET` as `x64-windows` on windows machines because default setting is `:x86-windows` (32-bit).

3. [Open AQ API](https://openaq.org). Simply make sure website is available from your machine.

4. [DarkSky API](https://darksky.net/dev) is used to fetch weather data:

    * if you do not have an account, you will need to [create one](https://darksky.net/dev/register) in order to request an API Key;
    * after building the server with cmake open the `settings.json` file located inside `build/bin[/CONFIG]` forlder and under the `darksky_api` object you may:
        * **(REQUIRED)** replace `api_key` parameter value with `Your API key` that you received from Dark Sky;
        * change "interval_download" parameter value (in minutes) to control how often weather data is downloaded.

5. You can build this application with [CMake](https://cmake.org) (verified on Windows 10 and Ubuntu 18.04):

    * (**optional**) make sure [CMake v3.10+ is installed](https://cmake.org/download/) or `sudo apt install cmake`;
    * make sure [vcpkg is installed](https://github.com/microsoft/vcpkg#quick-start) (it includes CMake by default);
    * install cpprestsdk in vcpkg: `vcpkg install cpprestsdk` (**use `:x64-windows` triplet in windows**);
    * create directory `build` in the root repository directory and switch to it: `mkdir build && cd build`
    * run cmake configure with toolchain from vcpkg:
        * Windows, MSVS 15 2017 : `cmake -G "Visual Studio 15 2017" Win64 -DCMAKE_TOOLCHAIN_FILE="**your/path/to**/vcpkg/scripts/buildsystems/vcpkg.cmake" ..`;
        * Windows, MSVS 16 2019 : `cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE="**your/path/to**/vcpkg/scripts/buildsystems/vcpkg.cmake" ..`;
        * Windows, other: `cmake -DVCPKG_TARGET_TRIPLET="x64-windows" -DCMAKE_TOOLCHAIN_FILE="**your/path/to**/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release ..`;
        * Ubuntu: `cmake -DCMAKE_TOOLCHAIN_FILE="**your/path/to**/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release ..`;
    * build with cmake: `cmake --build . --config Release`;

6. Starting the server:

    * you can start `aqw-opcua-server.exe` from anywhere, by default it will try to pickup settings from `settings.json` file located next to executable;
    * if default settings location is not acceptable, you can create your copy either from `settings.json` or `settings_example.json` and pass it as a parameter `relative to current directory`.

    Server runs on port `48484` by default.

    Examples:

    * `path/to/aqw-opcua-server.exe` - default scenario with default settings;
    * `path/to/aqw-opcua-server.exe my_custom_settings.json` - use your settings from current folder;
    * `path/to/aqw-opcua-server.exe relative/path/to/settings/collection/my_settings_variant42.json`.
