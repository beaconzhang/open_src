#!/bin/sh -xe

#compile common.cpp
g++ -c common.cpp -o common.o

#compile server
g++ getaddrinfo_server.cpp common.o -o getaddrinfo_server.out

#compile client
g++ getaddrinfo_client.cpp common.o -o getaddrinfo_client.out

rm -rf common.o
