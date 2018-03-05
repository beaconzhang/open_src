// @author Yu Hongjin (yuhongjin@meituan.com)

#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <iostream>
#include <map>

#include "logging.h"
#include "threadpool.h"
#include "util.h"
#include <sstream>

using std::string;
using std::cout;
using std::cerr;
using std::ostringstream;

#define LOG_DIFF(prefix, key) \
  LOG(WARN) << "[" << key << "] DIFF(" << prefix << ") "
#define LOG_RES(key) \
  LOG(INFO) << "[" << key << "] "

// stdout is a shared resource, so protected it with a mutex
static pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;

static std::string host_a;
static int port_a = 8026;
static std::string host_b;
static int port_b = 8026;

template <class K, class V>
std::string joinMap(const std::map<K, V>& m) {
    std::vector<std::string> v;
    for (auto it = m.begin(); it != m.end(); it++) {
        v.push_back(to(it->first) +
                    ":" + to(it->second));
    }
    return "" + join("|", v) + "";
}

std::string randomString(size_t n) {
    std::string s;
    s.resize(n);

    for (size_t i = 0; i < n; ++i) {
        s[i] = (rand() % 2 ? 'A' : 'a') + rand() % 26;
    }
    return s;
}

class iterm{
    public:
        iterm(){}
        virtual ~iterm(){}
        virtual void work()=0;
        virtual void read(const string&str)=0;
        virtual void write(string&str)=0;
};

class echo_server:public iterm{
    public:
        echo_server(const char*p,int fd=-1,char delimit=3):data(p),fd(fd),delimit(delimit){}
        echo_server(const string&p,int fd=-1,char delimit=3):data(p),fd(fd),delimit(delimit){}
        echo_server(const echo_server&es){
            data=es.data;
            fd=es.fd;
            delimit=es.delimit;
        }
        echo_server(){}
        echo_server& operator =(const echo_server&es){
            if(this!=&es){
                data=es.data;
                fd=es.fd;
                delimit=es.delimit;
            }
            return *this;
        }
        virtual void work(){
            cout<<"data:"<<data<<delimit<<"fd:"<<fd;
        }
        virtual void read(const string&str){
            int count=0;//
            uint32_t start_pos=0;
            uint32_t end_pos=0;
            uint32_t data_size=0;
            bool flag=true;
            for(;end_pos<str.size();end_pos++){
                if(__builtin_expect((str[end_pos]!=delimit),1)){
                    continue;
                }
                switch(count){
                    case 0:
                        string_int(str,start_pos,end_pos,data_size);
                        start_pos=end_pos+1;
                        count++;
                        break;
                    case 1:
                        if(end_pos-start_pos!=data_size){
                            flag=false;
                            goto read_error_label;
                        }
                        data=str.substr(start_pos,end_pos-start_pos);
                        start_pos=end_pos+1;
                        count++;
                        break;
                    default:
                        break;
                }
                if(start_pos<end_pos&&end_pos==str.size()){
                    uint32_t sign_fd;
                    string_int(str,start_pos,end_pos,sign_fd);
                    fd=sign_fd;
                }else{
                    flag=false;
                }
                read_error_label:
                    if(!flag){
                        cerr<<"format error:"<<str<<"\n";
                    } 
            }
        }
        virtual void write(string&str){
            ostringstream ss;
            ss<<data.size()<<delimit<<data<<delimit<<fd;
            str=ss.str();
        }
    private:
        string data;
        int fd;
        char delimit;
};

class QueryTask : public Task {
  public:
    QueryTask(const echo_server&es)
        : es(es) {
        rand_key_ = randomString(32);
    }
    ~QueryTask() { }

    virtual void run() {
        es.work();
        delete this;
    }

  private:
    std::string rand_key_;
    echo_server es;
};

std::string extractHostPort(const std::string& s, int* port) {
    size_t i = s.find(':');
    if (i != std::string::npos && i + 1 < s.size()) {
        *port = atoi(s.c_str() + i + 1);
        return s.substr(0, i);
    }
    return s;
}

std::string extractData(const std::string& line) {
    std::string prefix("serialize reqdata[");
    size_t i = line.find(prefix);
    if (i != std::string::npos) {
        i += prefix.size();
        int n = atoi(line.c_str() + i);
        if (n > 0) {
            i = line.find(',', i);
            if (i != std::string::npos && i + 1 + n <= line.size()) {
                return line.substr(i + 1, n);
            }
        }
    }
    return "";
}
std::string extract_data(const std::string& line){
    std::string prefix("req: ");
    size_t i = line.find(prefix);
    if (i != std::string::npos) {
        i += prefix.size();
        return line.substr(i);
    }
    return "";

}

void printUsage() {
    printf("Usage: qs-diff [OPTION]... HOST_A[:PORT] HOST_B[:PORT] SAMPLE\n");
    printf("\n");
    printf("Options:\n");
    printf("  -j, --threads=N       work thread count, default: 50\n");
    printf("  -n, --count=N         line limit count of sample\n");
    printf("  -p, --offset=P        line offset of sample\n");
    printf("      --log-level=LEVEL log level: DEBUG, INFO, WARN, ERROR\n");
    printf("      --log-file=PREFIX log to file PREFIX.log.#\n");
    printf("  -h, --help            print help\n");
}

int main(int argc, char** argv) {
    struct option options[] = {
        {"threads", required_argument, 0, 'j'},
        {"count", required_argument, 0, 'n'},
        {"offset", required_argument, 0, 'p'},
        {"log-level", required_argument, 0, 'l'},
        {"log-file", required_argument, 0, 'f'},
        {"help", no_argument, 0, 'h'},
    };
    int opt;
    int threads = 1;
    int count = INT_MAX;
    int offset = 0;
    const char* log_level = "DEBUG";
    const char* log_to = NULL;

    while ((opt = getopt_long(argc, argv, "j:n:p:r:h", options, 0)) != -1) {
        switch (opt) {
        case 'j': threads = atoi(optarg); break;
        case 'n': count = atoi(optarg); break;
        case 'p': offset = atoi(optarg); break;
        case 'l': log_level = optarg; break;
        case 'f': log_to = optarg; break;
        case 'h': printUsage(); exit(0); break;
        default:
            printUsage();
            exit(1);
            break;
        }
    }

    if (log_to) {
        std::string log_file(log_to);
        log_file += ".log.";
        int n = 0;
        do {
            const char* fname = (log_file + (char)('0' + n)).c_str();
            if (access(fname, F_OK) != 0) {
                logging::initialize(logging::getLevel(log_level[0]), fname);
                break;
            }
        } while (++n < 10);
    } else {
        logging::initialize(logging::getLevel(log_level[0]));
    }

    if (optind + 2 != argc) {
        printUsage();
        exit(1);
    }

    host_a = extractHostPort(argv[optind++], &port_a);
    //host_b = extractHostPort(argv[optind++], &port_b);

    std::fstream input(argv[optind++]);
    std::string line;
    uint32_t total_count=0;
    ThreadPool* pool = new ThreadPool(threads, 1024);
    while (count > 0 && getline(input, line)) {
        if (offset > 0) {
            offset--;
            continue;
        }
        echo_server data;
        data.read(line);
        pool->addTask(new QueryTask(data));
        total_count++;
        count--;
    }

    printf(" Total count: %lu\n", total_count);
    if (total_count == 0) {
        return 0;
    }


    printf("  Start time: %s\n", timeNowPrintf("%T").c_str());
    pool->counting();

    delete pool;

    return 0;
}

