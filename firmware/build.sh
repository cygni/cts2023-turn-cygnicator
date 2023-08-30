#!/bin/bash
GREEN="\e[32m"
YELLOW="\e[33m"
ENDCOLOR="\e[0m"

if [[ "$(docker images -q rpisdk:latest 2> /dev/null)" == "" ]]; then
  echo "Builder image not present, building..."
  docker build --build-arg UID="$(id -u)" -t rpisdk:latest docker/
fi

docker run --rm --user "$(id -u)" -v "$(pwd)":"$(pwd)" -w "$(pwd)" --name rpibuilder rpisdk:latest /bin/cmake .
docker run --rm --user "$(id -u)" -v "$(pwd)":"$(pwd)" -w "$(pwd)" --name rpibuilder rpisdk:latest /bin/make

echo -e "\n${YELLOW}=== Output Files ===${GREEN}"
find . -name "*.uf2"
echo -e "${ENDCOLOR}"