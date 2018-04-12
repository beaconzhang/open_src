#pragma one
#include <sys/epoll.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "../socket/socket.h"
using namespace std;

namespace xzhang_epoll{
    template <class T,class C>
    class epoll{
		public:
    	    int efd;
    	    int maxevent;
    	    struct epoll_event* ret;
    	    C* in_roll_buf;
			C* out_roll_buf;
			unordered_map<int,vector<T*>* >um;
			uint64_t count;
			uint32_t num_client;

        public:
            epoll(C* in_roll_buf=NULL,C* out_roll_buf=NULL,int maxevent=10240):\
				in_roll_buf(in_roll_buf),out_roll_buf(out_roll_buf),maxevent(maxevent){
                efd=epoll_create1(O_CLOEXEC);
                ret=new struct epoll_event[maxevent];
				in_roll_buf=new C();
				count=0;
				num_client=0;
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
					   //cout<<"epoll start:"<<i<<"\n";
                        T* tval=(T*)(ret[i].data.ptr);
                        if ((ret[i].events & EPOLLERR) || (ret[i].events & EPOLLHUP) || (!(ret[i].events & EPOLLIN))) {
                          // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                          fprintf(stderr, "epoll error\n");
                          close(tval->fd);
                          delete tval;
						  num_client--;
                        }else{
                            if(tval->is_listen()){
                                //是listen 描述符
                                while(1){
                                    int cfd=tval->read();
                                    if(cfd==0){
                                        break;
                                    }else if(cfd<=0){
                                        cerr<<"listen socket error\n";
										exit(-1);
                                    }else{
                                        //构造对象
                                        struct epoll_event ee;
										xzhang_socket::socket accept_sk(cfd);
										accept_sk.set_noblock();
										accept_sk.set_keepalive();
										accept_sk.reset();
                                        T* ptr=new T(false,cfd,0);
                                        ee.events = EPOLLIN | EPOLLRDHUP |EPOLLERR;
                                        ee.data.ptr=ptr;
                                        int retval = epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &ee);
                                        if (retval == -1) {
                                          perror("epoll_ctl");
										  exit(-1);
                                        }
										num_client++;
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
                                            ee.events=EPOLLIN | EPOLLRDHUP |EPOLLERR ;
                                            int retval = epoll_ctl(efd, EPOLL_CTL_MOD, tval->fd, &ee);
                                            if (retval == -1) {
                                                perror("epoll_ctl");
												exit(-1);
                                             }
                                        }
                                        break;
                                    }else if(nlen<0){
                                        close(tval->fd);
                                        delete tval;
										num_client--;
                                        break;
                                    }else{
										//tval->print();
										//tval->set_pos(0);
                                        vec.push_back(tval);
										count++;
										int fd=tval->fd;
										//delete tval;
                                        tval=new T(false,fd,0);
										//fprintf(stderr,"malloc addr from: %p ~ %p \n",tval,tval+1);
										is_not_exist=true;
                                    }
                                }
                            }
                        }
                   }
                   in_roll_buf->push_back(vec);
				   //vec.resize(0);
				   //in_roll_buf->pop_front_noblock(vec);
				   //for(int i=0;i<vec.size();i++){
				   //		delete vec[i];
				   //}
				   //cerr<<"vec.size()="<<vec.size()<<"\n";
                }
            }
			void free_vec(int fd){
				if(um[fd]){
					if(um[fd]->size()){
						num_client--;
					}
					for(typename vector<T*>::iterator iter=um[fd]->begin();iter!=um[fd]->end();\
							iter++){
						delete *iter;
					}
					um[fd]->resize(0);
                    int retval = epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                    if (retval == -1) {
						perror("error free epoll_ctl\n");
                    }
				}
			}
			void write(){
				vector<T*> vec;
				cout<<"write:"<<efd<<"\n";
				uint64_t tmp_cnt=0;
				while(1){
					vec.clear();
					int n=epoll_wait(efd,ret,maxevent,2000);
					out_roll_buf->pop_front(vec,out_roll_buf->get_element_size());
					if(vec.size()){
						tmp_cnt+=vec.size();
						cout<<"tmp_cnt="<<tmp_cnt<<"\n";
					}
					//for(int i=0;i<vec.size();i++){
					//	delete vec[i];
					//}
					//continue;
					if(!vec.size()){
						usleep(5000);
					}
					for(typename vector<T*>::iterator iter=vec.begin();iter!=vec.end();\
							iter++){
						int fd=(*iter)->get_sockfd();
						vector<T*>*tmp=um[fd];
						if(!tmp||tmp->size()==0){
							num_client++;
							if(!tmp){
								um[fd]=new vector<T*>;
							}
							//添加fd到efd中
							struct epoll_event ee;
							ee.events= EPOLLOUT | EPOLLRDHUP|EPOLLERR ;
							ee.data.fd=fd;
                            int retval = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ee);
                            if (retval == -1) {
                              perror("error write add epoll_ctl\n");
							  delete *iter;
							  continue;
                            }
						}
						um[fd]->push_back(*iter);
					}
					for(int i=0;i<n;i++){
						int fd=ret[i].data.fd;
                        if ((ret[i].events & EPOLLERR) || (ret[i].events & EPOLLHUP) || (!(ret[i].events & EPOLLOUT))) {
                    	    // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                    	    fprintf(stderr, "error write fd epoll error\n");
                    	    close(fd);
							free_vec(fd);
                        } else{
							int num=um[fd]->size()*2;
							if((*um[fd])[0]->get_pos()>=16){
								num--;
							}	
							struct iovec* iov=new struct iovec[num];
							int start=0;
							int iter=0;
							xzhang_socket::tlv* ptlv=(*um[fd])[0]->get_data();
							int pos=(*um[fd])[0]->get_pos();
							if(pos>=16){
								iov[start].iov_base=ptlv->buf+pos-16;
								iov[start++].iov_len=ptlv->length-(pos-16);
								iter++;
							}
							for(;iter<um[fd]->size();iter++){
								ptlv=(*um[fd])[iter]->get_data();
								char* head=(*um[fd])[iter]->get_head();
								pos=(*um[fd])[iter]->get_pos();
								iov[start].iov_base=head+pos;
								iov[start++].iov_len=16-pos;
								iov[start].iov_base=ptlv->buf;
								iov[start++].iov_len=ptlv->length;
							}
							count=writev(fd,iov,start);
							delete[] iov;
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
								while(iter<um[fd]->size()){
									ptlv=(*um[fd])[iter]->get_data();
									pos=(*um[fd])[iter]->get_pos();
									if(pos+count>=16+ptlv->length){
										count-=16+ptlv->length-pos;
										delete (*um[fd])[iter];
										iter++;
									}else{
										(*um[fd])[iter]->set_pos(count+pos);
										break;
									}
								}
								um[fd]->erase(um[fd]->begin(),um[fd]->begin()+iter);
								if(um[fd]->size()==0){
                   					int retval = epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ret[i]);
									num_client--;
                   					if (retval == -1) {
				   						perror("error delete epoll_ctl\n");
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
