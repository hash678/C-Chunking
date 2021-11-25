#!/bin/bash
# kill $(lsof -t -i:9004)
gcc -lpthread ./src/server.c -o ./bin/server  
./bin/server
