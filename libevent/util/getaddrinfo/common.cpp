#include "common.h"
void print_family(struct addrinfo *aip)
{
    printf("Family:");
    switch (aip->ai_family) {
    case AF_INET:
        printf("inet");
        break;
    case AF_INET6:
        printf("inet6");
        break;
    case AF_UNIX:
        printf("unix");
        break;
    case AF_UNSPEC:
        printf("unspecified");
        break;
    default:
        printf("unknown");
    }
}
void print_type(struct addrinfo *aip)
{
    printf(" Type:");
    switch (aip->ai_socktype) {
    case SOCK_STREAM:
        printf("stream");
        break;
    case SOCK_DGRAM:
        printf("datagram");
        break;
    case SOCK_SEQPACKET:
        printf("seqpacket");
        break;
    case SOCK_RAW:
        printf("raw");
        break;
    default:
        printf("unknown (%d)", aip->ai_socktype);
    }
}
void print_protocol(struct addrinfo *aip)
{
    printf(" Protocol:");
    switch (aip->ai_protocol) {
    case 0:
        printf("default");
        break;
    case IPPROTO_TCP:
        printf("TCP");
        break;
    case IPPROTO_UDP:
        printf("UDP");
        break;
    case IPPROTO_RAW:
        printf("raw");
        break;
    default:
        printf("unknown (%d)", aip->ai_protocol);
    }
}
void print_flags(struct addrinfo *aip)
{
    printf(" Flags:");
    if (aip->ai_flags == 0) {
        printf(" 0");
    } else {
        if (aip->ai_flags & AI_PASSIVE)
            printf(" passive");
        if (aip->ai_flags & AI_CANONNAME)
            printf(" canon");
        if (aip->ai_flags & AI_NUMERICHOST)
            printf(" numhost");
        if (aip->ai_flags & AI_NUMERICSERV)
            printf(" numserv");
        if (aip->ai_flags & AI_V4MAPPED)
            printf(" v4mapped");
        if (aip->ai_flags & AI_ALL)
            printf(" all");
    }
}
void print_addrinfo_list(struct addrinfo*head){
	for(struct addrinfo* rp=head;rp;rp=rp->ai_next){
		char ip_buf[BUF_SIZE];
		char* p=NULL;
		print_family(rp);
		print_type(rp);
		print_protocol(rp);
		print_flags(rp);
		printf("\nCanonical name:%s\n",rp->ai_canonname);
		if(rp->ai_family==AF_INET){
			p=(char*)&((struct sockaddr_in*)(rp->ai_addr))->sin_addr;
		}else if(rp->ai_family==AF_INET6){
			p=(char*)&((struct sockaddr_in6*)(rp->ai_addr))->sin6_addr;
		}else{
			fprintf(stderr,"can't recognition type of socket\n");
		}
		if(p){
			if(!inet_ntop(rp->ai_family,p,ip_buf,sizeof(ip_buf))){
				fprintf(stderr,"inet_ntop:%s\n",strerror(errno));
			}else{
				fprintf(stdout,"ip_addr:%s\n",ip_buf);
			}
			p=NULL;
		}
		cout<<"\n";
	}
	
}
