#pragma once

/*
 *reference https://github.com/dermesser/libsocket.git
 *lister fd and connection fd
 *protocal tlv https://github.com/beaconzhang/TLV.git
 *read :type,status,count,tlv
 *write :type,count,tlv;
 *keepalive
 *check fd error
 * */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <fcntl.h>

namespace xzhang_socket{
    class read_data{
        public:
        bool type;//true lister or false connector;
        int fd;
        int pos;//point read data;
        char head[17];//head
        tlv data;
        read_data():type(false),pos(0){
            memset(head,sizeof(head),0);
            fd=-1;
        }
        read_data(bool type,int fd,int pos=-1):type(type),fd(fd),pos(pos){
                meset(head,sizeof(head),0);
        }
        int read();
        int write();
		int get_pos(){
			return pos;
		}
		void set_pos(int inpos){
			pos=inpos;
		}
		tlv* get_data(){
			return &data;
		}
		char* get_head(){
			return head;
		}
        bool is_listen(){
            return type;
        }
		int get_sockfd(){
			return fd;
		}
        private:
        int _read();
    };
    class tlv{
        public:
        int type;
        int length;
        char*buf;
        tlv():type(-1),length(-1),buff(NULL){}
        tlv(int type,int length):type(type),length(length){
            buf=new char[length+1];
            buf[length]='\0';
        }
        void init(int tp,int len){
            type=tp;
            length=len;
            buf=new char[length+1];
            buf[length]='\0';
        }
        ~tlv(){
            if(buf){
                delete[] buf;
                buf=NULL;
                type=length=-1;
            }
        }
        private:
            tlv(const tlv&){}
            tlv& operator =(const tlv&){}
    };
    class socket{
        int fd;
        string host;
        string service;
        public:
            socket(int fd=-1):fd(fd){}
            socket(string hostinf);//:分割
            void get_hostinfo_str(string&value);
            int create_server();
            int create_client();
            int set_noblock();
            int set_keepalive();
            int is_alive();
            int close_rd();
            int close_wr();
            int close();
            ~socket(){
                if(fd>0){
                    close(fd);
                }
            }
    };
}
