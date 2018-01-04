#ifndef _COMMON_H_
#define _COMMON_H_


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
using namespace std;
#define BUF_SIZE 1024

void print_family(struct addrinfo*aip);
void print_type(struct addrinfo*aip);
void print_protocol(struct addrinfo*aip);
void print_flags(struct addrinfo*aip);
void print_addrinfo_list(struct addrinfo*head);
#endif
