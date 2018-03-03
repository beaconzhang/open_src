#include "socket.h"

using xzhang_socket::read_data;

int read_data::read(){
    if(type){
        //0表示没有新的连接，-1表示出错，>0表示新的连接描述符
        struct sockaddr in_addr;
        socklen_t in_len;
        int infd;
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

        in_len = sizeof in_addr;
        // No need to make these sockets non blocking since accept4() takes care of it.
        infd = accept4(fd, &in_addr, &in_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (infd == -1) {
          if ((errno == EAGAIN) ||
              (errno == EWOULDBLOCK)) {
            return 0;  // We have processed all incoming connections.
          } else {
            perror("accept error");
            return -1;
          }
        }

        // Print host and service info.
        retval = getnameinfo(&in_addr, in_len,
                             hbuf, sizeof hbuf,
                             sbuf, sizeof sbuf,
                             NI_NUMERICHOST | NI_NUMERICSERV);
        if (retval == 0) {
          printf("Accepted connection on descriptor %d (host=%s, port=%s)\n", infd, hbuf, sbuf);
        }
        return infd;
    }else{
        //0表示继续读取，-1表示对方关闭，-2表示套接字出错，>0表示读取完毕
        if(pos<16){
            int count=read(fd,head+pos,16-pos);
            if(count==-1){
                if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
                    return -2;
                }else{
                    return 0;
                }
            }else if(count==0){
                return -1;
            }else{
                pos+=count;
                if(pos==16){
                    char temp=head[8];
                    head[8]='\0';
                    int type=-1;
                    int len=-1;
                    sscanf(head,"%x",&type);
                    head[8]=temp;
                    sscanf(head+8,"%x",&len);
                    data.init(type,len);
                    int ret=-1;
                    if(len==0||(ret=_read())>0){
                        return pos;
                    }else{
                        return ret;
                    }
                }else{
                    return 0;
                }
            }
        }
    }
    return 0;
}
int read_data::_read(){
    int count=read(fd,data.buf+pos-16,data.length-(pos-16));
    if(count==-1){
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            return -2;
        }else{
            return 0;
        }
     }else if(count==0){
        return -1;
     }else{
        pos+=count;
        if(pos-16==data.legnth){
            return pos;
        }else{
            return 0;
        }
    }
    return 0;
}
int read_data::write(){
    //return 0 继续，-1 对方关闭，-2 socket出错，大于0写完毕
    if(type){
        return 1;
    }
    if(pos==16+data.length){
        return pos;
    }
    int count=0;
    if(pos<16){
        //use writev;
        struct iovec iov[2];
        iov[0].iov_base=head+pos;
        iov[0].iov_len=16-pos;
        iov[1].iov_base=data.buf;
        iov[1].iov_len=data.length;
        count=writev(fd,iov,2);
    }else{
        count=write(fd,data.buf+pos-16,data.length-(pos-16));
    }
    if(count==-1){
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            return -2;
        }else{
            return 0;
        }
     }else if(count==0){
        return -1;
     }else{
        pos+=count;
        if(pos-16==data.legnth){
            return pos;
        }else{
            return 0;
        }
    }
    return 0;
}

int 
