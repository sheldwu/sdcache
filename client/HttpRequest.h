#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ext/hash_map>
#include <sys/time.h>
#include "hash_wrap.h"
#define SERVER_LEN 1024
#define HEADER_LEN 10240
#define RECV_BUF 10*1024*1024
using namespace std;
using namespace __gnu_cxx;
class CHttpRequest{
    public:
        bool getUrl(string &url, char* chRecv, int nRecvTimeout, int nSendTimeout);
        CHttpRequest(){};
        ~CHttpRequest(){};
    private:
        bool parseUrl(const char *url, char *serverstrp, int *portp, const char **pathstrp);
        char* getHostByName(char* chName, char* const chHost, int nLen);
        bool setTimeOut(int sock, int nRecvOut, int nSendOut);

    private:
        hash_map<string, string> hssDNS;
};
#endif
