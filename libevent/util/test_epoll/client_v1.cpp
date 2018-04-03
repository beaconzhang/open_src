#include <iostream>
#include <string>
#include "../roll_buf/roll_buf.h"
#include "../epoll/epoll.h"
#include "../socket/socket.h"
#include <stdint.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(){
	while(1){
		xzhang_socket::socket listfd("192.168.1.18:8888");
		listfd.create_client();
		xzhang_socket::read_data* rd=new xzhang_socket::read_data(false,listfd.get_fd());
		xzhang_socket::read_data* rrd=new xzhang_socket::read_data(false,listfd.get_fd());
		rd->produce_data(10);
		rd->print();
		rd->write();
		while(rrd->read()<=0){
			cout<<"continue read\n";
			rrd->print();
		}
		if(*rd==*rrd){
			cout<<"true\n";
		}else{
			cout<<"origin:";
			rd->print();
			cout<<"recv:";
			rrd->print();
		}
		//rrd->print();
		delete rrd;
		delete rd;
		sleep(1);
	}
}
