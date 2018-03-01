#include "clock_time.h"
#include <unistd.h>

int main(){
	using namespace xzhang_libevent;

	clock_tm tm;
	sleep(0.001);
	std::cout<<"sleep "<<tm.end()<<"ns\n"<<tm;
}
