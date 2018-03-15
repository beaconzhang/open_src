#include <iostream>
#include <string>
#include "../roll_buf/roll_buf.h"
#include "../epoll/epoll.h"
#include "socket.h"
using namespace std;
//cs 模式，同一个socket上，client先发送请求，发送完后，只有在server端回复后才发送下一次请求
//server处理模式是，在服务端使用epoll，接收请求，work处理完后直接将数据发送给client端
//这样做的弊端是服务端的work只有在发送完client的响应后才能继续下一个处理，减少了系统的吞吐量

int main(){


}

