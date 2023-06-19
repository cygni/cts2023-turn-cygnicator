FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
build-essential cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib git ca-certificates make python3
ENTRYPOINT /bin/bash

ARG UID=1000
RUN useradd -m -u ${UID} -s /bin/bash builder
USER builder
RUN git clone https://github.com/raspberrypi/pico-sdk /tmp/pico-sdk && \
    cd /tmp/pico-sdk && \
    git submodule update --init
RUN git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git /tmp/FreeRTOS-Kernel && \
    cd /tmp/FreeRTOS-Kernel && \
    git submodule update --init
WORKDIR /tmp

ENV PICO_SDK_PATH="/tmp/pico-sdk"
ENV FREERTOS_KERNEL_PATH="/tmp/FreeRTOS-Kernel"