#include "socket.h"

using xzhang_socket::read_data;
#define SOMAXCONN 1024

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
xzhang_socket::socket::socket(string hostinfo){
    size_t pos=hostinfo.find(':');
    if(pos==string::npos){
        perror("please input host:service\n");
        return;
    }
    host=hostinfo.substr(0,pos);
    port=hostinfo.substr(pos+1);
}
int xzhang_socket::socket::set_noblock(){
  int flags, s;
  flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1){
      perror("fcntl");
      return -1;
  }
  flags |= O_NONBLOCK;
  s = fcntl(fd, F_SETFL, flags);
  if (s == -1){
      perror("fcntl");
      return -1;
  }
  return 0;
}
int xzhang_socket::socket::create_server(){
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int retval;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;     // Return IPv4 and IPv6 choices.
  hints.ai_socktype = SOCK_STREAM; // We want a TCP socket.
  hints.ai_flags = AI_PASSIVE;     // All interfaces.

  retval = getaddrinfo(NULL, service.c_str(), &hints, &result);
  if (retval != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
    return -1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1)
      continue;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
            perror("setsockopt(SO_REUSEADDR) failed");
    }
    retval = bind(fd, rp->ai_addr, rp->ai_addrlen);
    if (retval == 0) {
      // We managed to bind successfully!
      break;
    }

    close(fd);
  }
  if (rp == NULL) {
    fprintf(stderr, "Could not bind\n");
    return -1;
  }
  freeaddrinfo(result);
  set_noblock();
  retval = listen(fd, SOMAXCONN);
  if (retval == -1) {
    perror ("listen");
    abort ();
  }
  return fd;
}

int xzhang_socket::socket create_client(){
    int return_value;
    struct addrinfo hint, *result, *result_check;

    if ( host == "" || service == "" )
	return -1;

    memset(&hint,0,sizeof hint);

    // set address family
	hint.ai_family = AF_INET;

    // Transport protocol is TCP
    hint.ai_socktype = SOCK_STREAM;

    if ( 0 != (return_value = getaddrinfo(host.c_str(),service.c_str(),&hint,&result))){
	    const char* errstring = gai_strerror(return_value);
    	perror(errstring);
	    return -1;
    }
    // As described in "The Linux Programming Interface", Michael Kerrisk 2010, chapter 59.11 (p. 1220ff)
    for ( result_check = result; result_check != NULL; result_check = result_check->ai_next ) // go through the linked list of struct addrinfo elements
    {
	    fd = socket(result_check->ai_family, result_check->ai_socktype | flags, result_check->ai_protocol);

	    if ( fd < 0 ) // Error!!!
	        continue;

	    if ( -1 != connect(fd,result_check->ai_addr,result_check->ai_addrlen)) // connected without error
	        break;

	    close(fd);
    }

    // We do now have a working socket STREAM connection to our target

    if ( result_check == NULL ) // Have we?
    {
	    perror("create_inet_stream_socket: Could not connect to any address!\n");
        int errno_saved = errno;
        close(fd);
        errno = errno_saved;
    	return -1;
    }
    // Yes :)
    freeaddrinfo(result);
    return fd;
}

int xzhang_socket::socket::get_hostinfo_str(&value){
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(fd, (struct sockaddr *)&addr, &addr_size);
    if(res){
        perror(gai_strerror(res));
        return res;
    }
    res = getnameinfo(&addr, addr_size, hbuf, sizeof hbuf,sbuf, sizeof sbuf,NI_NUMERICHOST | NI_NUMERICSERV);
    if (res == 0) {
      printf("Accepted connection on descriptor %d (host=%s, port=%s)\n", fd, hbuf, sbuf);
    }
    return res;
}
int xzhang_socket::socket::set_keepalive(){
    int optval = 1;
    int ret=0;
    if((ret=setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)))){
        perror(gai_strerror(ret));
        return ret;
    }
    optval=3;
    if((ret=setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof optvalt))){
        perror(gai_strerror(ret));
        return ret;
    }
    optval=10;
    if((ret=setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof optvalt))){
        perror(gai_strerror(ret));
        return ret;
    }
    optval=3;
    if((ret=setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof optvalt))){
        perror(gai_strerror(ret));
        return ret;
    }
    return ret;
}

int xzhang_socket::socket::is_alive(){
    if(fd<=0) 
      return 0; 
    struct tcp_info info; 
    memset(&info,sizeof(info),0);
    int len=sizeof(info); 
    getsockopt(fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len); 
    if((info.tcpi_state==TCP_ESTABLISHED)) 
    { 
        //myprintf("socket connected\n"); 
        return 1; 
    }else{ 
        ////myprintf("socket disconnected\n"); 
        return 0; 
    }
}

int xzhang_socket::socket::close_rd(){
    return shutdown(fd,SHUT_RD);
}

int xzhang_socket::socket::close_rd(){
    return shutdown(fd,SHUT_WR);
}

int xzhang_socket::socket::close(){
    if(fd<=0){
        return 0;
    }
    int ret=close(fd);
    fd=-1;
    return ret;
}
