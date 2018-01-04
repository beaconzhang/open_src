#!/bin/sh -x
port=9090

nohup ./getaddrinfo_server.out $port 2>&1 >>server.log &

./getaddrinfo_client.out 127.0.0.1  $port zhang xiang hello 2>&1 >>client.log

ps -aux |grep getaddrinfo_server |grep -v grep |awk '{print $2}'|xargs kill -9
