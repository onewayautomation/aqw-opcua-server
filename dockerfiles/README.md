# Dockerfiles

## Dockerfile.full

Runs multi-stage build in docker containers and creates final image with the server (~75 MB). Takes ~1h on my machine.

Stage names (for targeting and image reuse): `build-tools`, `build-restsdk`, `build-wserv`.

Server file path inside the container: `/opt/weather-server/aqw-opcua-server`.

Entry point: `./aqw-opcua-server`.

Server expects `settings.json` file with valid Dark Sky API key inside next to executable (see project description). You can mount file to file directly from host machine. To use a different location you need to pass it as an argument at runtime.

Server runs on `48484` port by default.

Usage example (`settings.json` file in `/home/dev` dir on host machine, forwarding `48484` port):

* `docker build . -f Dockerfile.full -t weather-server`
* `docker run -v ~/dev/settings.json:/opt/weather-server/settings.json -p 48484:48484 weather-server`

## Dockerfile.simple

Manually build the server on Ubuntu following instructions from the main project page on GitHub.

Run this build command from root repository folder:

* `docker build . -f dockerfiles/Dockerfile.simple -t weather-server`
