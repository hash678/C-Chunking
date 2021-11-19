#!/bin/bash
kill $(lsof -t -i:9003)

gcc ./src/server.c -o ./bin/server  

./bin/server
