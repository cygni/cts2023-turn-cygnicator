#!/usr/bin/env bash

BIN=$1
if [ -z "$BIN" ]; then
    BIN=turn-cygnicator-reference/turn-cygnicator-reference
fi
docker run --rm -v $(pwd):$(pwd) -w $(pwd)/build -it rpisdk:latest $BIN
