
# Generate Makefile
```
docker run --rm -v $(pwd):/tmp/app_dir -w /tmp/app_dir --name rpibuilder rpisdk:latest /bin/cmake .
```

# Build Makefile
```
docker run --rm -v $(pwd):/tmp/app_dir -w /tmp/app_dir --name rpibuilder rpisdk:latest /bin/make
```

# Run Picotool
```
docker run --rm -v $(pwd):/tmp/app_dir -w /tmp/app_dir --name rpibuilder --device /dev/null rpisdk:latest /bin/picotool
```