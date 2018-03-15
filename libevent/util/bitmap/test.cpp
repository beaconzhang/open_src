#include <iostream>
#include "bitmap.h"
#include <vector>
using namespace std;
bool split(vector<string>&vec,const char*p){
    const char*pos=NULL;
    while(p&&*p){
        while(*p&&isspace(*p)){
            p++;
        }
        if(*p=='\0'){
            break;
        }
        for(pos=p;*pos&&!isspace(*pos);pos++);
        vec.push_back(string(p,pos-p));
        p=pos;
    }
    return !!(vec.size());
}
int main(int argc,char**argv){
    if(argc!=2){
        cerr<<"please max bit\n";
        exit(-1);
    }
    cout<<"usage help:\ncommand [value]:\nset num\nget num\ndump flag\nexit\n";
    string line;
    vector<string>vec;
    xzhang_bitmap::bitmap bm(atoi(argv[1]));
    while(getline(cin,line)){
        vec.clear();
        if(!split(vec,line.c_str())){
            cerr<<"input error\n";
            continue;
        }
        if(vec[0]=="set"&&vec.size()==2){
            bm.set(atoi(vec[1].c_str()));
        }else if(vec[0]=="get"&&vec.size()==2){
            cout<<bm.get(atoi(vec[1].c_str()))<<"\n";
        }else if(vec[0]=="dump"&&vec.size()==2){
            bm.output(!!atoi(vec[1].c_str()));
        }else if(vec[0]=="exit"){
            break;
        }else{
            cerr<<"usage help:\ncommand [value]:\nset num\nget num\ndump flag\nexit\n";
        }

    }
    return 0;
}
