#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "ddcache.h"
#define SERVER_LEN 1024
#define HEADER_LEN 10240
#define RECV_LEN 10*1024*1024

typedef unsigned long long  u64;
typedef unsigned long       ul;
typedef unsigned int        u32;
typedef int             SOCKET;
using namespace std;
//using namespace sd;
class CHttpRequest{
    public:
        bool getUrl(string &url, string &res, sd::ddcache<string>* cacheNameToHost, 
                int nRecvTimeout, int nSendTimeout);
        CHttpRequest(){};
        ~CHttpRequest(){};
    private:
        bool parseUrl(const char *url, char *serverstrp, int *portp, const char **pathstrp);
        char* getHostByName(char* chName, char* const chHost, int nLen);
};
#endif
