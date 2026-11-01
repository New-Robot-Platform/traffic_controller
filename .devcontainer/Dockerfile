FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN apt-get update \
    && apt-get install -y \
        libpaho-mqtt-dev \
        libpaho-mqttpp-dev \
        mosquitto \
        mosquitto-clients \
    && rm -rf /var/lib/apt/lists/*
