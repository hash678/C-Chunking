#!/bin/bash
kill $(lsof -t -i:9003)

gcc -pthread ./src/server.c -o ./bin/server  

./bin/server
