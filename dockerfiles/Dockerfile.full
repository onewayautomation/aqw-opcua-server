FROM ubuntu:bionic AS build-tools

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    git \
    tar \
    unzip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

RUN git clone https://github.com/Microsoft/vcpkg.git

WORKDIR vcpkg

RUN ./bootstrap-vcpkg.sh && ./vcpkg integrate install



FROM build-tools AS build-restsdk

WORKDIR /src/vcpkg

RUN ./vcpkg install cpprestsdk \
    && rm -rf buildtrees \
    && rm -rf downloads \
    && rm -rf packages



FROM build-restsdk AS build-wserv

WORKDIR /src

RUN git clone https://github.com/ehnat0n/aqw-opcua-server.git

WORKDIR ./aqw-opcua-server/build

RUN cmake -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release .. \
    && cmake --build . --config Release



FROM ubuntu:bionic

RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/weather-server

COPY --from=build-wserv /src/aqw-opcua-server/build/bin/aqw-opcua-server ./

ENTRYPOINT ["./aqw-opcua-server"]
