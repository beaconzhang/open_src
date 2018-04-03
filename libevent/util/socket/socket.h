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
#include <sys/uio.h>
#include <string>
#include <netinet/tcp.h>
#include <iostream>
#include <unistd.h>
using std::cout;
using std::string;

namespace xzhang_socket{
    class tlv{
        public:
        int type;
        int length;
        char*buf;
        tlv():type(0),length(0),buf(NULL){}
        tlv(int type,int length):type(type),length(length){
            buf=new char[length+1];
            buf[length]='\0';
        }
		void produce_data(int _length,int _type=0){
			type=_type;
			length=_length;
			buf=new char[length+1];
			buf[length]='\0';
			for(int i=0;i<length;i++){
				switch(rand()%3){
					case 0:
						buf[i]='0'+rand()%10;
						break;
					case 1:
						buf[i]='a'+rand()%26;
						break;
					case 2:
						buf[i]='A'+rand()%26;
						break;
					default:
						perror("produce_data error\n");
						break;
				}
			}
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
    class read_data{
        public:
        bool type;//true lister or false connector;
        int fd;
        int pos;//point read data;
        char head[17];//head
        tlv data;
        read_data():type(false),pos(0){
            memset(head,0,sizeof(head));
            fd=-1;
        }
        read_data(bool type,int fd,int pos=0):type(type),fd(fd),pos(pos){
                memset(head,0,sizeof(head));
        }
		bool operator ==(const read_data&rd){
			return data.type==rd.data.type&&data.length==rd.data.length&&!memcpy(data.buf,rd.data.buf,data.length);
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
		void produce_data(int _length,int _type=0){
			data.produce_data(_length,_type);
			snprintf(head,9,"%08x",data.type);
			snprintf(head+8,9,"%08x",data.length);
		}
		void print(){
			cout<<"is_listen:"<<type<<" fd:"<<fd<<" pos:"<<pos<<" type:"<<data.type<<" length:"<<\
				data.length<<" head:"<<head<<" data:"<<data.buf<<"\n";
		}
        private:
        int _read();
    };
    class socket{
        int fd;
        string host;
        string service;
        public:
            socket(int fd=-1):fd(fd){}
            socket(string hostinf);//:分割
            int get_hostinfo_str();
			void reset(int in_fd=-1){
				fd=in_fd;
			}
            int create_server();
            int create_client();
            int set_noblock();
            int set_keepalive();
            int is_alive();
            int close_rd();
            int close_wr();
            int close();
			int get_socket(){
				return fd;
			}
			int get_fd(){
				return fd;
			}
            ~socket(){
                if(fd>0){
                    ::close(fd);
                }
            }
    };
}
