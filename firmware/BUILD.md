# Build Docker image to use for building
```
docker build --build-arg UID=$(id -u) -t rpisdk:latest docker/
```

# Build UF2 files
```
./build.sh
```

# Run Picotool
```
docker run --rm -v $(pwd):/tmp/app_dir -w /tmp/app_dir --name rpibuilder --privileged rpisdk:latest /bin/picotool info
```

# Flash uf2 (and execute) file to Pico using Picotool
```
docker run --rm -v $(pwd):/tmp/app_dir -w /tmp/app_dir --name rpibuilder --privileged rpisdk:latest /bin/picotool load -v -x turn-cygnicator-kalle/turn-cygnicator-kalle.uf2
```