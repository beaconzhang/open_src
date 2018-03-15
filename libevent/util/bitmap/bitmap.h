#pragma one
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

namespace xzhang_bitmap{
    class bitmap{
        private:
            unsigned char * p;
            unsigned max_bit;
            unsigned capacity;
        public:
            bitmap(unsigned max_bit=1024):max_bit(max_bit){
                if(max_bit<=0){
                    perror("error,max_bit must great 0\n");
                    exit(-1);
                }
                capacity=(max_bit+7+1)>>3;
                p=(unsigned char*)malloc(capacity);
                if(!p){
                    perror("error,malloc failed\n");
                    exit(-1);
                }
                memset(p,0,capacity);
            }
            void set(unsigned  num){
                if(num>(capacity<<3)){
                    perror("overflow\n");
                    return;
                }
                p[num>>3]|=1<<(num&7);
            }
            bool get(unsigned num){
                if(num>(capacity<<3)){
                    return false;
                }
                return !!(p[num>>3]&(1<<(num&7)));
            }
            void clear(){
                memset(p,0,capacity);
            }
            void output(bool flag){
                for(unsigned i=0;i<(capacity<<3);i++){
                    if(get(i)==flag){
                        printf("%u\n",i);
                    }
                }
            }
            ~bitmap(){
                if(p){
                    free(p);
                }
            }
        public:
            bitmap(const bitmap&){}
            bitmap& operator = (const bitmap&){
                return *this;
            }
    };
}

