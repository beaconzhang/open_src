#pragma one
#include <sys/epoll.h>
#include <vector>
#include <iostream>
using namespace std;

namespace xzhang_epoll{
    template <class T,class C>
    class epoll{
        int efd;
        int maxevent;
        struct epoll_event* ret;
        C* roll_buf;
        public:
            epoll(int maxevent=1024){
                efd=epoll_create1(O_CLOEXEC);
                ret=new struct epoll_event[maxevent];
            }
            int add(int fd,struct epoll_event* pe ){
                return epoll_ctl(efd, EPOLL_CTL_ADD, fd, pe);
            }
            int del(int fd,struct epoll_event*pe){
                return epoll_ctl(efd,EPOLL_CTL_DEL,fd,pe);
            }
            int mod(int fd,struct epoll_event*pe){
                return epoll_ctl(efd,EPOLL_CTL_MOD,fd,pe);
            }
            void start(){
                while(1){
                   int n=epoll_wait(efd,ret,maxevent,ret,-1);//1s timeout
                   vector<T*>vec;
                   for(int i=0;i<n;i++){
                        T* tval=(T*)(ret[i].data.ptr);
                        if ((ret[i].events & EPOLLERR) || (ret[i].events & EPOLLHUP) || (!(ret[i].events & EPOLLIN))) {
                          // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                          fprintf(stderr, "epoll error\n");
                          close(tval->fd);
                          delete tval;
                        } else if (ret[i].events & EPOLLRDHUP) {
                          // Stream socket peer closed connection, or shut down writing half of connection.
                          // We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
                          printf("Closed connection on descriptor vis EPOLLRDHUP %d\n", tval->fd);
                          // Closing the descriptor will make epoll remove it from the set of descriptors which are monitored.
                          close(tval->fd);
                          delete tval;
                        }else{
                            if(tval->is_listen()){
                                //是listen 描述符
                                while(1){
                                    int cfd=tval->read();
                                    if(cfd==0){
                                        break;
                                    }else if(cfd<=0){
                                        cerr<<"listen socket error\n";
                                        abort();
                                    }else{
                                        //构造对象
                                        struct epoll_event ee;
                                        T* prt=new T(false,cfd,0);
                                        ee.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
                                        ee.data.ptr=ptr;
                                        int retval = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ee);
                                        if (retval == -1) {
                                          perror("epoll_ctl");
                                          abort();
                                        }
                                    }
                                }
                            }else{
                                //读取数据
                                bool is_not_exist=false;
                                while(1){
                                    int nlen=tval->read();
                                    if(nlen==0){
                                        if(is_not_exist){
                                            //modify epoll
                                            struct epoll_event ee;
                                            ee.data.ptr=tval;
                                            ee.events=EPOLLIN | EPOLLRDHUP | EPOLLET;
                                            int retval = epoll_ctl(epfd, EPOLL_CTL_MOD, cfd, &ee);
                                            if (retval == -1) {
                                                perror("epoll_ctl");
                                                abort();
                                             }
                                        }
                                        break;
                                    }else if(nlen<0){
                                        close(tval->fd);
                                        delete tval;
                                        break;
                                    }else{
                                        vec.push_back(tval);
                                        tval=new T(false,tval->fd,0);
                                    }
                                }
                            }
                        }
                   }
                   roll_buf->push_back(vec);
                }
            }
            ~epoll(){
                if(ret){
                    delete[] ret;
                    ret=NULL;
                }
            }
        private:
            epoll(const epoll&){}
            epoll& operator =(const epoll&){}

    };
}
