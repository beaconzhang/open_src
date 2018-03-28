#include <iostream>
#include <string>
#include "../roll_buf/roll_buf.h"
#include "../epoll/epoll.h"
#include "../socket.h"
using namespace std;

roll_buf<xzhang_socket::read_data*> read_rool_buf=NULL;
roll_buf<xzhang_socket::read_data*> write_rool_buf=NULL;
int read_num_thread=3,write_num_thread=3;
struct read_work_iterm{
	roll_buf<xzhang_socket::read_data*>* buf;
	uint64_t
};

int main(){


}

