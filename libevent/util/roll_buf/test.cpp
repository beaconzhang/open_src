#include "roll_buf.h"
#include <string>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
using namespace std;
roll_buf<string*>* rbuf;

struct work_iterm{
    uint32_t num;
    uint32_t length;
    uint32_t cnt;
    bool stop;
    work_iterm(uint32_t num=1024,uint32_t length=1024,uint32_t cnt=0,bool stop=false):num(num),length(length),cnt(cnt),stop(stop){}
};
string* produce_string(int32_t pthread,uint32_t seq,uint32_t num){
	char buf[20];
	snprintf(buf,sizeof(buf),"%08x %08x ",pthread,seq);
	string* ret=new string(buf);
	ret->resize(num);
	for(uint32_t i=18;i<num;i++){
		int mod=0;
		//cerr<<i<<"\n";
		if((mod=rand()%62)<10){
			(*ret)[i]='0'+mod;
		}else if(mod<36){
			(*ret)[i]='A'+mod-10;
		}else{
			(*ret)[i]='a'+mod-36;
		}
	}
	return ret;
}
void* produce_work(void*arg){
	work_iterm* wi=(work_iterm*)arg;
	char buf [2048];
	snprintf(buf,sizeof(buf),"%08x.log",pthread_self());
	fstream fout(buf,ios::out);
	vector<string*>vec;
	string* result=NULL;
	while(!wi->stop&&wi->cnt<wi->num){
		vec.clear();
		int mod=1;
		if((mod=(rand()&7)+1)==1){
			result=produce_string(pthread_self(),wi->cnt++,wi->length);
			fout<<*result<<"\n";
			rbuf->push_back(result);
		}else{
			for(uint32_t i=0;i<mod&&wi->cnt<wi->num;i++){
				result=produce_string(pthread_self(),wi->cnt++,wi->length);
				fout<<*result<<"\n";
				vec.push_back(result);
			}
			rbuf->push_back(vec);
		}
	}
	fout.close();
	return NULL;
}
void* consume_work(void*arg){
	work_iterm* wi=(work_iterm*)arg;
	char buf[20];
	snprintf(buf,sizeof(buf),"%08x.log",pthread_self());
	fstream fout(buf,ios::out);
	vector<string*>vec;
	string* result=NULL;
	while(!wi->stop){
		vec.clear();
		uint32_t mod=1;
		if((mod=(rand()&7)+1)==1){
			rbuf->pop_front(result);
			if(!result){
				cerr<<"error,pop null\n";
				continue;
			}
			wi->cnt++;
			fout<<*result<<"\n";
			delete result;
		}else{
			rbuf->pop_front(vec,mod);
			if(!vec.size()){
				cerr<<"error,pop zero\n";
				fout.flush();
				continue;
			}
			for(uint32_t i=0;i<vec.size();i++){
				wi->cnt++;
				if(!vec[i]){
					cerr<<"error pop null\n";
					continue;
	 			}
				fout<<*vec[i]<<"\n";
				delete vec[i];
			}
		}
		fout.flush();
	}
	cout<<"consume:"<<wi->cnt<<"\n";
	fout.close();
	return NULL;
}

int main(){
	rbuf=new roll_buf<string*>(1024);
	int produce_work_cnt=4;
	int consume_work_cnt=3;
	vector<work_iterm*>vec;
	for(int i=0;i<consume_work_cnt;i++){
		pthread_t pid;
		work_iterm* wi=new work_iterm(10240,256);
		vec.push_back(wi);
		pthread_create(&pid,NULL,consume_work,wi);
		printf("consume %08x\n",pid);
		pthread_detach(pid);
	}
	for(int i=0;i<produce_work_cnt;i++){
		pthread_t pid;
		work_iterm* wi=new work_iterm(10240,256);
		vec.push_back(wi);
		pthread_create(&pid,NULL,produce_work,wi);
		printf("produce %08x\n",pid);
		pthread_detach(pid);
	}
	uint32_t cnt=20;
	while(cnt){
		cnt--;
		cerr<<rbuf->get_element_size()<<"\n";
		sleep(1);
	}
	for(uint32_t i=0;i<vec.size();i++){
		vec[i]->stop=true;
	}
	sleep(1);
	return 0;
}
