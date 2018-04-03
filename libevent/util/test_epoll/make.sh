#!/usr/bin/env bash
ulimit -c unlimited
g++ -g -O1 -std=c++11 server.cpp -o server.out ../socket/socket.cpp -lpthread
g++ -g -O1 -std=c++11 client.cpp -o client.out ../socket/socket.cpp -lpthread
