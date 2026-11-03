FROM ubuntu:24.04 AS build
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      g++ \
      libpaho-mqttpp-dev \
      libpaho-mqtt-dev && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY src/publisher.cpp .
RUN g++ -std=c++20 -O2 publisher.cpp -o client -lpaho-mqttpp3 -lpaho-mqtt3as -pthread

FROM ubuntu:24.04
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libpaho-mqttpp3-1 \
      libpaho-mqtt1.3 && \
    rm -rf /var/lib/apt/lists/*
COPY --from=build /build/client /usr/local/bin/client
CMD ["client"]
