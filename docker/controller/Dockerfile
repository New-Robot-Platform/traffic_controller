FROM ubuntu:24.04 AS build
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      g++ \
      libpaho-mqttpp-dev \
      libpaho-mqtt-dev && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY src/subscriber.cpp .
RUN g++ -std=c++20 -O2 subscriber.cpp -o controller -lpaho-mqttpp3 -lpaho-mqtt3as -pthread

FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
      libpaho-mqttpp3-1 \
      libpaho-mqtt1.3 && \
    rm -rf /var/lib/apt/lists/*
COPY --from=build /build/controller /usr/local/bin/controller
CMD ["controller"]
