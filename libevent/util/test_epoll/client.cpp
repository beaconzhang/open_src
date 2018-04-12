#include <iostream>
#include <string>
#include <sys/epoll.h>
#include "../socket/socket.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <pthread.h>
#include "../clock_time/clock_time.h"
using namespace std;
int us_sleep=1000;
int start_len=1024,end_len=8096;
int num_fd=10;
struct _work_iterm{
	xzhang_socket::read_data* arr;
	xzhang_libevent::clock_tm ct;
	_work_iterm(int fd){
		arr=new xzhang_socket::read_data(false,fd);
	}
	void produce_data(int start,int end,int type){
		arr->produce_data(rand()%(end-start)+start,type);
	}
	int write(){
		return arr->write();
	}
	int read(){
		return arr->read();
	}
	int get_fd(){
		return arr->fd;
	}
	int get_type(){
		return arr->get_type();
	}
	void set_pos(int pos=0){
		arr->set_pos(0);
	}
	long elaspe(){
		return ct.end();
	}
	~_work_iterm(){
		if(arr){
			delete arr;
		}
	}
};
//write thread write data,read thread read data,not need lock;
struct work_iterm{
	struct _work_iterm* _wi;
	uint32_t count;
	int fd;
	work_iterm(int fd):count(0),fd(fd){
		_wi=NULL;
	}
	work_iterm(){
		_wi=NULL;
		count=0;
		fd=-1;
	}
	void set_fd(int _fd){
		fd=_fd;
	}
	void reset(bool is_write=true,int type=0){
		_wi=new struct _work_iterm(fd);
		if(is_write){
			_wi->produce_data(start_len,end_len,type);
		}
	}
	~work_iterm(){
		if(_wi){
			delete _wi;
		}
		close(fd);
	}
};
//write thread data,read thread delete data;
unordered_map<uint64_t,_work_iterm*> um;
int id=0;
void* write_work(void*arg){
	work_iterm* wi=(work_iterm*)arg;
	int efd=epoll_create1(O_CLOEXEC);
	struct epoll_event* ret=new struct epoll_event[num_fd];
	struct epoll_event ee;
	ee.events=EPOLLOUT|EPOLLRDHUP|EPOLLERR;
	for(int i=0;i<num_fd;i++){
		ee.data.ptr=wi+i;
		if(epoll_ctl(efd,EPOLL_CTL_ADD,wi[i].fd,&ee)){
			cerr<<"epoll_ctl add error\n";
			abort();
		}
	}
	while(1){
		int n=epoll_wait(efd,ret,num_fd,-1);
		for(int i=0;i<1;i++){
			work_iterm* wi_tmp=(work_iterm*)ret[i].data.ptr;
			int fd=wi_tmp->fd;
			if((ret[i].events & EPOLLERR) || (ret[i].events & EPOLLHUP) ||\
					(!(ret[i].events & EPOLLOUT))){
				cerr<<"fd:"<<fd<<" error\n";
				continue;
			}else{
				if(wi_tmp->_wi->write()){
					um[wi_tmp->_wi->get_type()]=wi_tmp->_wi;
					wi_tmp->reset(true,++id);
					//cerr<<"write data:";
					//wi_tmp->_wi->arr->print();
					wi_tmp->count++;
				}
			}
		}
		usleep(us_sleep);
	}
	return NULL;
}
void* read_work(void*arg){
	work_iterm* wi=(work_iterm*)arg;
	int efd=epoll_create1(O_CLOEXEC);
	struct epoll_event* ret=new struct epoll_event[num_fd];
	struct epoll_event ee;
	ee.events=EPOLLIN|EPOLLRDHUP|EPOLLERR;
	for(int i=0;i<num_fd;i++){
		ee.data.ptr=wi+i;
		if(epoll_ctl(efd,EPOLL_CTL_ADD,wi[i].fd,&ee)){
			cerr<<"epoll_ctl add error\n";
			abort();
		}
	}
	while(1){
		int n=epoll_wait(efd,ret,num_fd,-1);
		for(int i=0;i<n;i++){
			work_iterm* wi_tmp=(work_iterm*)ret[i].data.ptr;
			int fd=wi_tmp->fd;
			if((ret[i].events & EPOLLERR) || (ret[i].events & EPOLLHUP) ||\
					(!(ret[i].events & EPOLLIN))){
				cerr<<"fd:"<<fd<<" error\n";
				continue;
			}else{
				if(wi_tmp->_wi->read()){
					wi_tmp->_wi->arr->print();
					if(*(um[wi_tmp->_wi->get_type()]->arr)==*(wi_tmp->_wi->arr)){
						wi_tmp->count++;
					}else{
						cerr<<"origin:";
						um[wi_tmp->_wi->get_type()]->arr->print();
						cerr<<"recv:";
						wi_tmp->_wi->arr->print();
					}
					auto iter=um.find(wi_tmp->_wi->get_type());
					delete um[wi_tmp->_wi->get_type()];
					if(iter!=um.end()){
						um.erase(um.find(wi_tmp->_wi->get_type()));
					}else{
						cerr<<"can't find key:"<<wi_tmp->_wi->get_type()<<"\n";
					}
					delete wi_tmp->_wi;
					wi_tmp->count++;
					wi_tmp->reset(false);
				}
			}
		}
		usleep(us_sleep);
	}
	return NULL;
}

int main(){
	work_iterm* read_wi=new work_iterm[num_fd];
	work_iterm* write_wi=new work_iterm[num_fd];
	for(int i=0;i<num_fd;i++){
		xzhang_socket::socket listfd("192.168.1.18:8888");
		listfd.create_client();
		listfd.set_noblock();
		listfd.set_keepalive();
		read_wi[i].set_fd(listfd.get_fd());
		write_wi[i].set_fd(listfd.get_fd());
		listfd.reset();
		read_wi[i].reset(false);
		write_wi[i].reset(true,id++);
	}
	pthread_t pid;
	pthread_create(&pid,NULL,read_work,read_wi);
	pthread_detach(pid);
	pthread_create(&pid,NULL,write_work,write_wi);
	pthread_detach(pid);
	while(1){
		uint64_t total_read=0,total_write=0;
		for(int i=0;i<num_fd;i++){
			total_read+=read_wi[i].count;
			total_write+=write_wi[i].count;
		}
		cout<<"statistic info:\ntotal_read:"<<total_read<<"\ttotal_write:"<<\
			total_write<<"\tum.size="<<um.size()<<"\n";
		sleep(20);
	}
}
