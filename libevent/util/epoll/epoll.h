#pragma one
#include <sys/epoll.h>
#include <vector>
#include <iostream>
#include <unorder_map>
#include "bitmap.h"
using namespace std;
using namespace xzhang_bitmap;

namespace xzhang_epoll{
    template <class T,class C>
    class epoll{
        int efd;
        int maxevent;
        struct epoll_event* ret;
        C* in_roll_buf;
		C* out_roll_buf;
		unorder_map<int,vector<T*> >um;

        public:
            epoll(C* out_roll_buf=NULL,int maxevent=1024):out_roll_buf(out_roll_buf){
                efd=epoll_create1(O_CLOEXEC);
                ret=new struct epoll_event[maxevent];
				in_roll_buf=new C();
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
                   int n=epoll_wait(efd,ret,maxevent,-1);//1s timeout
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
										is_not_exist=true;
                                    }
                                }
                            }
                        }
                   }
                   in_roll_buf->push_back(vec);
                }
            }
			void free_vec(fd){
				if(um[fd]){
					for(vector<T*>::iterator iter=um[fd]->begin();iter!=um[fd]->end();\
							iter++){
						delete *iter;
					}
					um[fd].resize(0);
                    int retval = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ret[i]);
                    if (retval == -1) {
						perror("epoll_ctl");
                    }
				}
			}
			void write(){
				vector<T*> vec;
				while(1){
					vec.clear();
					int n=epoll_wait(efd,ret,maxevent,1);
					out_roll_buf->pop_front(vec,out_roll_buf->get_element_size());
					for(vector<T*>::iterator iter=vec.begin();iter!=vec.end();\
							iter++){
						int fd=(*iter)->get_sockfd();
						vector<T*>tmp=um[fd];
						if(!tmp){
							um[fd]=new vector<T*>;
							//添加fd到efd中
							struct epoll_event ee;
							ee.events= EPOLLOUT | EPOLLRDHUP | EPOLLET;
							ee.data.fd=fd;
                            int retval = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ee);
                            if (retval == -1) {
                              perror("epoll_ctl");
                              abort();
                            }
						}
						um[fd]->push_back(*iter);
					}
					for(int i=0;i<n;i++){
						int fd=ret[i].data.fd;
                        if ((ret[i].events & EPOLLERR) || (ret[i].events & EPOLLHUPi || (!(ret[i].events & EPOLLOUT))) {
                    	    // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                    	    fprintf(stderr, "epoll error\n");
                    	    close(fd);
							free_vec(fd);
                        } else{
							int num=um[fd].size()*2;
							if(um[fd][0]->get_pos()>=16){
								num--;
							}	
							struct iovec* iov=new struct iovec[num];
							int start=0;
							int iter=0;
							xzhang_socket::tlv* ptlv=um[fd][0]->get_data();
							if(um[fd][0]->get_pos()>=16){
								iov[start].iov_base=ptlv->buf+pos-16;
								iov[start++].iov_len=ptlv->length-(pos-16);
								iter++;
							}
							for(;iter<um[fd].szie();iter++){
								ptlv=um[fd][iter]->get_data();
								char* head=um[fd][iter]->get_head();
								pos=um[fd][iter]->get_pos();
								iov[start].iov_base=head+pos;
								iov[start++].iov_length=16-pos;
								iov[start].iov_base=ptlv->buf;
								iov[start++].iov_len=ptlv->length;
							}
							count=writev(fd,iov,start);
							delete iov;
    						if(count==-1){
        						if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
									free_vec(fd);
        						}else{
									continue;
        						}
     						}else if(count==0){
								free_vec(fd);
     						}else{
								iter=0;
								while(iter<um[fd].size()){
									ptlv=um[fd][iter]->get_data();
									pos=um[fd][iter]->get_pos();
									if(pos+count>=16+tlv->length){
										count-=16+tlv->length-pos;
										delete um[fd][iter];
										iter++;
									}else{
										um[fd][iter]->set_pos(count+pos);
										break;
									}
								}
								um[fd]->erase(um[fd]->begin(),um[fd]->begin()+iter);
								if(um[fd]->size()==0){
                   					int retval = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ret[i]);
                   					if (retval == -1) {
				   						perror("epoll_ctl");
                   					}
								}
    						}
                        }
						
					}
					
				}
			}
            ~epoll(){
                if(ret){
                    delete[] ret;
					//delete roll_buf;
                    ret=NULL;
					in_roll_buf=NULL;
					out_roll_buf=NULL;
                }
            }
        private:
            epoll(const epoll&){}
            epoll& operator =(const epoll&){}

    };
}
