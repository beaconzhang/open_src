#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "ht-internal.h"
using namespace std;

struct node{
    HT_ENTRY(node) next;
    node(const string&data):data(data){
        next.hte_next=NULL;
    }
    string data;
};
//uint64_t murmur_hash_64(const void * key, int len, uint64_t seed)
uint32_t murmur_hash_64(const void * key, int len, uint64_t seed)
{
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len / 8);

    while (data != end)
    {
#ifdef PLATFORM_BIG_ENDIAN
        uint64_t k = *data++;
        char *p = (char *)&k;
        char c;
        c = p[0]; p[0] = p[7]; p[7] = c;
        c = p[1]; p[1] = p[6]; p[6] = c;
        c = p[2]; p[2] = p[5]; p[5] = c;
        c = p[3]; p[3] = p[4]; p[4] = c;
#else
        uint64_t k = *data++;
#endif

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch (len & 7)
    {
    case 7: h ^= uint64_t(data2[6]) << 48;
    case 6: h ^= uint64_t(data2[5]) << 40;
    case 5: h ^= uint64_t(data2[4]) << 32;
    case 4: h ^= uint64_t(data2[3]) << 24;
    case 3: h ^= uint64_t(data2[2]) << 16;
    case 2: h ^= uint64_t(data2[1]) << 8;
    case 1: h ^= uint64_t(data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    //return h;
    return (uint32_t)h;
}
uint64_t node_hash_fun(const node* nd){
    return murmur_hash_64(nd->data.c_str(),nd->data.size(),0);
}

bool node_equal(const node*n1,const node*n2){
    return n1->data==n2->data;
}

HT_HEAD(node_head,node) head;
HT_PROTOTYPE(node_head,node,next,node_hash_fun,node_equal);
HT_GENERATE(node_head,node,next,node_hash_fun,node_equal,0.8,malloc,realloc,free);
void usage_msg(){
    cout<<"-n --number  insert hashtable number element,defalut 100\n\
       -l --length every element data length  defalut 1024\n"; 
}
char rand_produce_char(){
    switch(rand()%3){
        case 0:
            return rand()%10+'0';
            break;
        case 1:
            return rand()%26+'a';
            break;
        case 2:
            return rand()%26+'A';
            break;
        default:
            cout<<__FILE__<<":"<<__LINE__<<" ["<<__FUNCTION__<<"]ERROR\n";
            return '0';
            break;
    }
    cout<<__FILE__<<":"<<__LINE__<<" ["<<__FUNCTION__<<"]ERROR\n";
    return 0;
}
void print_hashtable(node_head&head){
    cout<<"length:"<<head.hth_table_length<<"\tentry number:"<<head.hth_n_entries<<"\t\
        load limit:"<<head.hth_load_limit<<"\tprime_idx:"<<head.hth_prime_idx<<"\n";
}
int main(int argc,char**argv){
   string data;
   int32_t num=100;
   int32_t len=1024;
   int opt;
   struct option options[]={
       {"number",required_argument,0,'-n'},
       {"length",required_argument,0,'-l'},
   };
   srand((unsigned)time(NULL));
   while((opt=getopt_long(argc,argv,"n:l:",options,0))!=-1){
       int temp=0;
       switch(opt){
           case 'n':
               temp=atoi(optarg);
               num=temp>0?temp:num;
               break;
           case 'l':
               temp=atoi(optarg);
               len=temp>0?temp:len;
               break;
           default:
               usage_msg();
               exit(-1);
               break;
       }
   }
   data.resize(len);
   cout<<"num="<<num<<"\tlen="<<len<<"\n";
   for(int i=0;i<num;i++){
       for(int32_t j=0;j<len;j++){
           data[j]=rand_produce_char();
       }
       node* np=new node(data);
       node* p=HT_FIND(node_head,&head,np);
       if(!p){
           cout<<"insert data:"<<data<<"\thash_value:"<<node_hash_fun(np)<<"\n";
           HT_INSERT(node_head,&head,np);
       }else{
           cout<<"insert data:"<<data<<"\thash_value:"<<node_hash_fun(np)<<"\n";
           HT_INSERT(node_head,&head,np);
           delete p;
       }
   }
   print_hashtable(head);
   //遍历hashtable，并且删除各个元素，在这里不调用内部的api，直接对hashtable进行操作
   for(int32_t i=0;i<head.hth_table_length;i++){
       while(head.hth_table[i]){
           node* p=head.hth_table[i];
           head.hth_table[i]=p->next.hte_next;
           if(node_hash_fun(p)%head.hth_table_length!=i){
               cerr<<"error,value:"<<p->data<<"\thash value:"<<node_hash_fun(p)<<"\tposition:"<<i<<"\thash_table_length:"<<head.hth_table_length<<"\n";
           }
           delete p;
           head.hth_n_entries--;
       }
   }
   print_hashtable(head);
   //释放hash表内存
   HT_CLEAR(node_head,&head);
   print_hashtable(head);
   HT_INIT(node_head,&head);
   
}
