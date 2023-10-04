#!/usr/bin/env bash

UF2=$1
if [ -z "$UF2" ]; then
    UF2=turn-cygnicator/turn-cygnicator.uf2
fi
docker run --rm -v $(pwd):$(pwd) -w $(pwd)/build --name rpibuilder --privileged --user=root rpisdk:latest /bin/picotool load -f -v -x $UF2
