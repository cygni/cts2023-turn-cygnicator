#!/usr/bin/env bash

DEVICE=$1
if [ -z "$DEVICE" ]; then
    DEVICE=/dev/ttyACM0
fi
docker run --rm -v $(pwd):$(pwd) -w $(pwd)/build --device=$DEVICE -it rpisdk:latest  picocom $DEVICE
