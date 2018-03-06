#!/bin/sh -x

rm -rf *log
rm -rf core*
g++ test.cpp -std=c++11 -lpthread -g

ulimit -c unlimited


