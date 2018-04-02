#!/usr/bin/env bash

g++ -g -std=c++11 server.cpp ../socket/socket.cpp -lpthread
