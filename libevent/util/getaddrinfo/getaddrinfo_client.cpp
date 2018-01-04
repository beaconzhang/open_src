#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
using namespace std;
#define BUF_SIZE 1024
int main(int argc,char**argv){
	struct addrinfo hints;
	struct addrinfo *result,*rp;
	int sfd,s,j;
	size_t len;
	ssize_t nread;
	char buf[BUF_SIZE];
	if(argc<3){
		cerr<<"Usage:%s host port msg...\n";
		exit(EXIT_FAILURE);
	}
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_flags=0;
	hints.ai_protocol=0;
	s=getaddrinfo(argv[1],argv[2],&hints,&result);
	if(s!=0){
		fprintf(stderr,"getaddrinfo:%s\n",gai_strerror(s));
		exit(EXIT_FAILURE);
	}
	for(rp=result;rp;rp=rp->ai_next){
		sfd=socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol);
		if(sfd==-1){
			continue;
		}
		if(connect(sfd,rp->ai_addr,rp->ai_addrlen)!=-1){
			break;
		}
		close(sfd);
	}
	if(!rp){
		fprintf(stderr,"Could not connect\n");
		exit(EXIT_FAILURE);
	}
	for(j=3;j<argc;j++){
		len=strlen(argv[j])+1;
		if(len>BUF_SIZE){
			fprintf(stderr,"Ignore long message in argument %s\n",j);
			continue;
		}
		if(write(sfd,argv[j],len)!=len){
			fprintf(stderr,"Message in argument %d partial/failed write\n",j);
			continue;
		}
		if((nread=read(sfd,buf,sizeof(buf)))==-1){
			fprintf(stderr,"Message in argument %d error read\n",j);
			exit(EXIT_FAILURE);
		}
		printf("Send message byte:%d\nReceived %zd bytes:%s\n",len,nread,buf);
	}
	return 0;

}
