#ifndef SERVERFRAME_H_
#define SERVERFRAME_H_
#include "ddcache.h"
#include "ring_queue.h"
#include "SimpleThread.h"
#include "ComDef.h"
#include "HttpAnalyse.h"
#include "hash_wrap.h"
#include "HttpRequest.h"
#include "Configer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>

#define QUEUE_MAX_LEN 64
#define RECV_BUF_LEN 1024
#define SOCKET_ERROR -1
#define INVALID_SOCKET  -1 
#define closesocket close
#define TYPE_LEN 100
#define LISTEN_QUEUE_LEN 1024

#define URL "url"
#define DEBUG_MODE "debug"
#define ETH_NAME "eth0"
#define cfg "../config/server.conf"

//using namespace sd;
typedef unsigned long long  u64;
typedef unsigned long       ul;
typedef unsigned int        u32;
typedef int             SOCKET;

class CServerFrame{
public:
    CServerFrame(void){};
    ~CServerFrame(void){};

    /*
     *function create http server
     *para @nPort start port
     *para @nThreadCount thread count
     *para @bucketCnt cache bucket cnt
     *para @valSz cache value size
     *para @sliceSz cache slice size
     *para @maxSz cache max size
     *para @expireTime cache expire seconds 
     *return bool if create suceessfully
     * */
    bool createServer(u_short nPort,u_short nThreadCount, 
                    u32 bucketCnt, u32 valSz, u32 sliceSz, u64 maxSz, u32 expireTime);
    /*
     *function close server
     *return if close successfully
     * */
    bool closeServer();
    /*
     *function start run server
     *return if run server successfully
     * */
    bool runServer();
    
public:

protected:
    bool BindToLocalHost();
    static void* serverThread(void *lpParameter );
    int getLocalIP(char* dest, socklen_t size);
    void _errorStr(string &strSend);
    bool _compress(Bytef* dest, uLongf* ulDestLen, Bytef* source, uLong ulSrcLen);
    void _addHeader(string &strSend, const char* chContent, char* chType);
private:
    u_short m_lsnPort;                  //listen port
    u_short m_nThreadCount;             //server thread count
    SOCKET  m_lsnSock;                  //listen socket
    sd::ddcache<string> m_cacheNameToHost;     //host to ip table, void use func gethostbyname each time

    static bool m_bShutdown ;           // Signals client/server threads to die

    u32     m_BucketCnt;    //cache bucket count
    u32     m_valSz;        //cache value size
    u32     m_sliceSz;      //cache slice size
    u32     m_maxSz;        //cache max size M
    u32     m_expireTime;   //cache expire seconds
    int     m_clientRecvTimeout;
    int     m_clientSendTimeout;
    int     m_targetRecvTimeout;
    int     m_targetSendTimeout;
    int     m_hostCacheTime;
}; 

struct SSockItem{
    SOCKET  sock;
    string  url;
    u64     hasher;
    bool    debug;
};

struct SPara{
    ring_queue<SSockItem>*  rq;
    CServerFrame*           psf;
    int                     threadNo;
};
#endif
