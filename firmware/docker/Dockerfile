# Build image
# docker build --build-arg UID=$(id -u) -t rpisdk:latest .

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib git ca-certificates make python3 \
    libusb-1.0-0-dev pkg-config clangd clang-format picocom libncurses5-dev \
    libncursesw5-dev

# for simulator
RUN apt-get install -y --no-install-recommends libncurses5-dev

# for simulator
RUN apt-get install -y --no-install-recommends libncurses5-dev

ARG UID=1000
RUN useradd -m -u ${UID} -s /bin/bash builder

ENV PICO_SDK_PATH="/tmp/pico-sdk"
ENV FREERTOS_KERNEL_PATH="/tmp/FreeRTOS-Kernel"

USER builder
RUN git clone https://github.com/raspberrypi/pico-sdk /tmp/pico-sdk && \
    cd /tmp/pico-sdk && \
    git submodule update --init
RUN git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git /tmp/FreeRTOS-Kernel && \
    cd /tmp/FreeRTOS-Kernel && \
    git submodule update --init
WORKDIR /tmp

USER root
RUN git clone --depth 1 https://github.com/raspberrypi/picotool.git /tmp/picotool && \
    cd /tmp/picotool && \
    mkdir build || true && \
    cd build && \
    cmake .. && \
    make

RUN cp /tmp/picotool/build/picotool /bin/picotool && \
    rm -rf /tmp/picotool

RUN chown builder:builder /bin/picotool

USER builder
