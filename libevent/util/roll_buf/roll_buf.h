#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <x86intrin.h>

#include <atomic>
#include <cinttypes>
#include <cstdio>

namespace processor {

#define CACHE_LINE_SIZE 64

// A simple ring buffer for single producers and single consumers.
// Does not support parallel consumers for now.
template<typename T>
class RingBuffer {

public:
  // Events_size must be a power of two.
  explicit RingBuffer(uint64_t capacity) :
    capacity(capacity),
    produce_pos(0),
    consume_pos(0),
    mutex(PTHREAD_MUTEX_INITIALIZER),
    produce_cond(PTHREAD_COND_INITIALIZER),
    consum_cond(PTHREAD_COND_INITIALIZER){
       elemet=new T[capacity]; 
    }

  // No copy constructor.
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator = (const RingBuffer &) = delete;


  inline uint64_t get_capacity() const {
    return capacity;
  }
  iniline uint64_t get_element_size()const{
       return produce_pos-consume_pos;
  }
    void push_back(Ttval){
        pthread_mutex_lock(&mutex);
        while(produce_pos-consume_pos>=capacity){
            pthread_cond_wait(&consume_cond,&mutex);
        }
        element[(produce_pos++)&(capacity-1)]=tval;
        pthread_cond_signal(&produce_cond);
        pthread_mutex_unlock(&mutex);
    }
    void push_back(const vector<T>&vec){
        pthread_mutex_lock(&mutex);
        uint32_t start_pos=0;
        while(start_pos<vec.size()){
            while(produce_pos-consume_pos>=capacity){
                pthread_cond_wait(&consume_cond,&mutex);
            }
            for(;start_pos<vec.size()&&produce_pos-consume_pos<capacity;){
                element[(produce_pos++)&(capacity-1)]=vec[start_pos++];
            }
        }
        if(consume_pos>=capacity){
            consume_pos-=capacity;
            produce_pos-=capacity;
        }
        pthread_cond_signal(&produce_cond);
        pthread_mutex_unlock(&mutex);
    }
    void pop_front(T&tval){
        tval=NULL;
        pthread_mutex_lock(&mutex);
        while(produce_pos<=consume_pos){
            pthread_cond_wait(&produce_cond,&mutex);
        }
        tval=element[consume_pos++];
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
        while(produec_pos<=consume_pos){
            pthread_cond_wait(&produce_cond,&mutex);
        }
        for(;num>0&&consume_pos<produce_pos;){
            vec.push_back(element[consume_pos++]);
            num--;
        }
        if(consume_pos>=capacity){
            consume_pos-=capacity;
            produce_pos-=capacity;
        }
        pthread_cond_signal(&consume_cond);
        pthread_mutex_unlock(&mutex);
    }

  ~RingBuffer() {
    printf("Deleted Ring Buffer\n");
    delete[] element;
  }

private:

  uint64_t capacity;//2的幂次方
  T element;
  uint64_t produce_pos;
  uint64_t consume_pos;
  pthread_mutex_t mutex;
  pthread_cond_t produce_cond;
  pthread_cond_t consume_cond;
} __attribute__ ((aligned(CACHE_LINE_SIZE)));

}  // processor

#endif
