#pragma once
#include <time.h>
#include <iostream>
#include <time.h>
#include <string.h>
using std::cerr;
using std::ostream;
namespace xzhang_libevent{
	class clock_tm{
		public:
			clock_tm(){
				memset(&st,0,sizeof(struct timespec));
				//memset(&ed,0,sizeof(struct timespec));
				if(clock_gettime(CLOCK_MONOTONIC,&st)){
					cerr<<__FILE__<<":"<<__LINE__<<":"<<__func__<<" clock_gettime error\n";
				}
			}
			bool start(){
				//memset(&st,0,sizeof(struct timespec));
				return !clock_gettime(CLOCK_MONOTONIC,&st);
			}
			long  end(){
				if(clock_gettime(CLOCK_MONOTONIC,&ed)){
					return -1;
				}
				return (ed.tv_sec-st.tv_sec)*1000000000+ed.tv_nsec-st.tv_nsec;
			}
			friend ostream& operator<<(ostream&out,const clock_tm&tm);
		private:
			struct timespec st;
			struct timespec ed;
	};
	ostream& operator<<(ostream&out,const clock_tm&tm){
		out<<"st:"<<tm.st.tv_sec<<"s "<<tm.st.tv_nsec<<"ns\ted:"<<tm.ed.tv_sec<<"s "<<tm.ed.tv_nsec<<"\n";
		return out;
	}
}

