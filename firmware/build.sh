#!/bin/bash
GREEN="\e[32m"
YELLOW="\e[33m"
ENDCOLOR="\e[0m"
if [[ "$(docker images -q rpisdk:latest 2> /dev/null)" == "" ]]; then
  echo "Builder image not present, building..."
  docker build --build-arg UID="$(id -u)" -t rpisdk:latest docker/
fi
firmware_dir="$(pwd)"
mkdir build
docker run --rm --user "$(id -u)" -v "${firmware_dir}":"${firmware_dir}" -w "$(pwd)/build" --name rpibuilder rpisdk:latest /usr/bin/cmake ..
docker run --rm --user "$(id -u)" -v "${firmware_dir}":"${firmware_dir}" -w "$(pwd)/build" --name rpibuilder rpisdk:latest /usr/bin/make
echo -e "\n${YELLOW}=== Output Files ===${GREEN}"
find . -name "*.uf2"
echo -e "${ENDCOLOR}"
