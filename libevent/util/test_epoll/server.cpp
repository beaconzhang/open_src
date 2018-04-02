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
using namespace std;

int read_num_thread=3,write_num_thread=3;
struct process_work_iterm{
	int read_num_thread;
	int write_num_thread;
	uint64_t in_count;
	uint64_t out_count;
	int index;

	roll_buf<xzhang_socket::read_data*>* read_rool_buf;
	roll_buf<xzhang_socket::read_data*>* write_rool_buf;
	vector<xzhang_socket::read_data*>vec;
	process_work_iterm(int read_num_thread=0,int write_num_thread=0,int index=0,roll_buf<xzhang_socket::read_data*>* read_rool_buf=NULL,\
			roll_buf<xzhang_socket::read_data*>* write_rool_buf=NULL):read_num_thread(read_num_thread),write_num_thread(write_num_thread),\
			index(index),read_rool_buf(read_rool_buf),write_rool_buf(write_rool_buf){
				in_count=0;
				out_count=0;
			}
	void set_buf(int _read_num_thread,int _write_num_thread,roll_buf<xzhang_socket::read_data*>* _read_rool_buf,\
			roll_buf<xzhang_socket::read_data*>* _write_rool_buf){
		read_num_thread=_read_num_thread;
		write_num_thread=_write_num_thread;
		read_rool_buf=_read_rool_buf;
		write_rool_buf=_write_rool_buf;
	}
};
struct read_work_iterm{
	roll_buf<xzhang_socket::read_data*>* buf;
	xzhang_epoll::epoll<xzhang_socket::read_data,roll_buf<xzhang_socket::read_data*> > *ep;
	read_work_iterm(roll_buf<xzhang_socket::read_data*>* buf,xzhang_epoll::\
			epoll<xzhang_socket::read_data,roll_buf<xzhang_socket::read_data*> > *ep=NULL):\
		buf(buf),ep(ep){}
	read_work_iterm(){
		buf=NULL;
		ep=NULL;
	}
	void set_buf(roll_buf<xzhang_socket::read_data*>* in_buf){
		buf=in_buf;
	}
};
//暂时只启动一个process work
void* process_work(void*arg){
	process_work_iterm* pwi=(process_work_iterm*)arg;
	vector<xzhang_socket::read_data*>vec;
	vector<vector<xzhang_socket::read_data*> > vec_write;
	vec_write.resize(pwi->write_num_thread);
	bool flag=false;
	while(1){
		for(int i=0;i<vec_write.size();i++){
			vec_write[i].resize(0);
		}
		for(int i=0;i<pwi[i].read_num_thread;i++){
			pwi->read_rool_buf[i].pop_front_noblock(vec,pwi->read_rool_buf[i].get_element_size());
			for(uint32_t j=0;j<vec.size();j++){
				int fd=vec[j]->fd;
				vec_write[fd%pwi->write_num_thread].push_back(vec[j]);
			}
		}
		for(uint32_t i=0;i<vec_write.size();i++){
			pwi->write_rool_buf->push_back(vec_write[i]);
			if(vec_write[i].size()){
				flag=true;
			}
		}
		if(!flag){
			usleep(5000);
		}
	}
	return NULL;
}
void* recv_work(void*arg){
	read_work_iterm* rwi=(read_work_iterm*)arg;
	xzhang_socket::socket listfd("0.0.0.0:8888");
	listfd.create_server();
	if(!rwi->ep){
		rwi->ep=new xzhang_epoll::epoll<xzhang_socket::read_data,roll_buf<xzhang_socket::read_data*> >(\
				rwi->buf);
	}
	cout<<"lister fd:"<<listfd.get_fd()<<"\n";
	xzhang_socket::read_data* list_read_data=new xzhang_socket::read_data(true,\
			listfd.get_fd(),0);
	struct epoll_event list_epoll_event;
	list_epoll_event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
	list_epoll_event.data.ptr=list_read_data;
	rwi->ep->add(listfd.get_fd(),&list_epoll_event);
	while(1){
		rwi->ep->start();
	}
	return NULL;
}
void* snd_work(void*arg){
	read_work_iterm* rwi=(read_work_iterm*)arg;	
	if(!rwi->ep){
		rwi->ep=new xzhang_epoll::epoll<xzhang_socket::read_data,roll_buf<xzhang_socket::read_data*> >(NULL,
				rwi->buf);
	}
	while(1){
		rwi->ep->write();
	}
	return NULL;
}

int main(){
	int num_read=3;
	int num_write=3;
	int num_process=1;
	roll_buf<xzhang_socket::read_data*>* read_roll_buf=new roll_buf<xzhang_socket::read_data*>[num_read];
	roll_buf<xzhang_socket::read_data*>* write_roll_buf=new roll_buf<xzhang_socket::read_data*>[num_write];
	process_work_iterm* pwi=new process_work_iterm[1];
	read_work_iterm* rwi=new read_work_iterm[num_read];
	read_work_iterm* wwi=new read_work_iterm[num_write];
	for(int i=0;i<num_read;i++){
		pthread_t pid;
		rwi[i].set_buf(read_roll_buf+i);
		pthread_create(&pid,NULL,recv_work,rwi+i);
		pthread_detach(pid);
	}
	for(int i=0;i<num_write;i++){
		pthread_t pid;
		wwi[i].set_buf(write_roll_buf+i);
		pthread_create(&pid,NULL,snd_work,wwi+i);
		pthread_detach(pid);
	}
	for(int i=0;i<num_process;i++){
		pwi[i].set_buf(num_read,num_write,read_roll_buf,write_roll_buf);
		pthread_t pid;
		pthread_create(&pid,NULL,process_work,pwi+i);
		pthread_detach(pid);
	}
	while(1){
		sleep(60);
		cout<<"statistic info\n";
	}

}

