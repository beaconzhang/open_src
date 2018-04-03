#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <x86intrin.h>

#include <atomic>
#include <cinttypes>
#include <cstdio>
#include <pthread.h>
#include <iostream>
#include <vector>
using namespace std;



// A simple ring buffer for single producers and single consumers.
// Does not support parallel consumers for now.
template<typename T>
class roll_buf {

public:
  // Events_size must be a power of two.
  
  roll_buf() :
    capacity(8096),
    produce_pos(0),
    consume_pos(0),
	in(0),
  	out(0),
    mutex(PTHREAD_MUTEX_INITIALIZER),
    produce_cond(PTHREAD_COND_INITIALIZER),
    consume_cond(PTHREAD_COND_INITIALIZER){
       element=new T[capacity]; 
	   timeout.tv_sec=0;
	   timeout.tv_nsec=1000000;
    }
  explicit roll_buf(uint64_t capacity) :
    capacity(capacity),
    produce_pos(0),
    consume_pos(0),
	in(0),
  	out(0),
    mutex(PTHREAD_MUTEX_INITIALIZER),
    produce_cond(PTHREAD_COND_INITIALIZER),
    consume_cond(PTHREAD_COND_INITIALIZER){
       element=new T[capacity]; 
	   timeout.tv_sec=0;
	   timeout.tv_nsec=1000000;
    }



  inline uint64_t get_capacity() const {
    return capacity;
  }
  inline uint64_t get_element_size()const{
	   //cerr<<"in:"<<in<<" out:"<<out<<"\n";
       return produce_pos-consume_pos;
  }
  	void print(){
		printf("roll_buf:size=%lu\n",get_element_size());
		for(int i=consume_pos;i<produce_pos;i++){
			element[(i)&(capacity-1)]->print();
		}
	}
    void push_back(const T& tval){
        pthread_mutex_lock(&mutex);
        while(produce_pos-consume_pos>=capacity){
            pthread_cond_timedwait(&consume_cond,&mutex,&timeout);
        }
        element[(produce_pos++)&(capacity-1)]=tval;
		in++;
        pthread_cond_signal(&produce_cond);
        pthread_mutex_unlock(&mutex);
    }
    void push_back(const vector<T>&vec){
        pthread_mutex_lock(&mutex);
        uint32_t start_pos=0;
        while(start_pos<vec.size()){
            while(produce_pos-consume_pos>=capacity){
                pthread_cond_timedwait(&consume_cond,&mutex,&timeout);
            }
            for(;start_pos<vec.size()&&produce_pos-consume_pos<capacity;){
                element[(produce_pos++)&(capacity-1)]=vec[start_pos++];
				in++;
            }
        }
        pthread_cond_signal(&produce_cond);
        pthread_mutex_unlock(&mutex);
    }
    void pop_front(T&tval){
        tval=NULL;
        pthread_mutex_lock(&mutex);
        while(produce_pos<=consume_pos){
            pthread_cond_timedwait(&produce_cond,&mutex,&timeout);
        }
        tval=element[consume_pos++];
		out++;
        if(consume_pos>=capacity){
            consume_pos-=capacity;
            produce_pos-=capacity;
        }
        pthread_cond_signal(&consume_cond);
        pthread_mutex_unlock(&mutex);
    }
    void pop_front(vector<T>&vec,uint32_t num=1){
        //返回数目小于等于num，但是大于1
        if(num<=0){
            return;
        }
        vec.clear();
        pthread_mutex_lock(&mutex);
        while(produce_pos<=consume_pos){
            pthread_cond_timedwait(&produce_cond,&mutex,&timeout);
        }
        for(;num>0&&consume_pos<produce_pos;){
            vec.push_back(element[(consume_pos++)&(capacity-1)]);
            num--;
			out++;
        }
        if(consume_pos>=capacity){
            consume_pos-=capacity;
            produce_pos-=capacity;
        }
        pthread_cond_signal(&consume_cond);
        pthread_mutex_unlock(&mutex);
    }
	void pop_front_noblock(vector<T>&vec,uint32_t num=1){
        if(num<=0){
            return;
        }
        vec.clear();
        pthread_mutex_lock(&mutex);
        for(;num>0&&consume_pos<produce_pos;){
            vec.push_back(element[(consume_pos++)&(capacity-1)]);
            num--;
			out++;
        }
        if(consume_pos>=capacity){
            consume_pos-=capacity;
            produce_pos-=capacity;
        }
        pthread_mutex_unlock(&mutex);
	}

  ~roll_buf() {
    printf("Deleted Ring Buffer\n");
    delete[] element;
  }

private:
  // No copy constructor.
  roll_buf(const roll_buf&){}
  roll_buf& operator = (const roll_buf &){}

  uint64_t capacity;//2的幂次方
  T* element;
  uint64_t produce_pos;
  uint64_t consume_pos;
  //debug
  uint64_t in;
  uint64_t out;
  pthread_mutex_t mutex;
  pthread_cond_t produce_cond;
  pthread_cond_t consume_cond;
  struct timespec timeout;
};


#endif
