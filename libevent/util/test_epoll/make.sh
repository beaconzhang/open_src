#!/usr/bin/env bash
ulimit -c unlimited
rm -rf core.*
g++ -g -O0 -std=c++11 server.cpp -o server.out ../socket/socket.cpp -lpthread
g++ -g -O0 -std=c++11 client.cpp -o client.out ../socket/socket.cpp -lpthread
