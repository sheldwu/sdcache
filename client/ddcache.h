#ifndef DDCACHE_H_
#define DDCACHE_H_
#include <string>
#include "zlib.h"
#include <sys/time.h>
typedef unsigned long ul;
#define MAX_VAL_LEN 1024*1024 //1M
#define MAX_VAL_LEN_PLUS MAX_VAL_LEN+sizeof(ul)
using namespace std;
class ddcache{
    public:
        ddcache(){
        };
        ~ddcache(){};
        bool get(string &url, char* chRes, ul* ulLen);
};
#endif
