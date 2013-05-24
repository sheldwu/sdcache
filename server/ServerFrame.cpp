#include "ServerFrame.h"
#include "CommonFunc.h"

#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
bool CServerFrame::m_bShutdown = false;

bool CServerFrame::createServer(u_short nPort,u_short nThreadCount, u32 bucketCnt, u32 valSz, u32 sliceSz, u64 maxSz, u32 expireTime){
    m_lsnPort      = nPort;
    m_nThreadCount  = nThreadCount;
    m_BucketCnt     = bucketCnt;
    m_valSz         = valSz;      
    m_sliceSz       = sliceSz;    
    m_maxSz         = maxSz;      
    m_expireTime    = expireTime; 
    HSS hssConfig;
    Configer cfger;
    if(!cfger.parseConfig(cfg, hssConfig)){
        printf("config file load fail\n");
        return false;
    }
    if((m_hostCacheTime = atoi(hssConfig["hostCacheTime"].c_str())) <= 0){
        m_hostCacheTime = 3600;
    }
    if((m_clientRecvTimeout = atoi(hssConfig["clientRecvTimeout"].c_str())) <= 0){
        m_clientRecvTimeout = 3;
    }
    if((m_clientSendTimeout = atoi(hssConfig["clientSendTimeout"].c_str())) <= 0){
        m_clientSendTimeout = 3;
    }
    if((m_targetRecvTimeout = atoi(hssConfig["targetRecvTimeout"].c_str())) <= 0){
        m_targetRecvTimeout = 3;
    }
    if((m_targetSendTimeout = atoi(hssConfig["targetSendTimeout"].c_str())) <= 0){
        m_targetSendTimeout = 3;
    }
    printf("\nConfig\nhostCacheTime %d\nclientRecvtimeout: %d\nclientSendTimeout:%d\ntargetRecvTimeout:%d\ntargetSendTiemout:%d\n", m_hostCacheTime, m_clientRecvTimeout, m_clientSendTimeout, m_targetRecvTimeout, m_targetSendTimeout);
    m_cacheNameToHost.init(10, 32, 4096, 
            "./cache/host_cache.bin", 20, m_hostCacheTime, false);//refresh every half an hour

    if(!BindToLocalHost()){
        return false;
    }
    return true;
}

bool  CServerFrame::BindToLocalHost(){
    m_lsnSock = socket(AF_INET, SOCK_STREAM, 0);
    FALSE_RETURN_STRERROR(INVALID_SOCKET == m_lsnSock)

    //reuse address immediately in case server down and tcp status is TIME_WAIT
    int nRet = 1;
    FALSE_RETURN_STRERROR(SOCKET_ERROR == setsockopt(m_lsnSock, SOL_SOCKET,
                             SO_REUSEADDR, (char*)&nRet, sizeof(nRet)))

    struct sockaddr_in addrSock;
    addrSock.sin_family = AF_INET;
    addrSock.sin_port = htons(m_lsnPort);
    addrSock.sin_addr.s_addr = htonl(INADDR_ANY);

    int retval = bind(m_lsnSock, (sockaddr*)&addrSock, sizeof(sockaddr));
    if(SOCKET_ERROR == retval){
        fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
        closesocket(m_lsnSock);
        return false;
    }
    return true;
}

bool CServerFrame::runServer(){
    FALSE_RETURN_STRERROR(SOCKET_ERROR == listen(m_lsnSock,LISTEN_QUEUE_LEN))
    threadManager thrMngr;  
    vector<SPara> vpara(m_nThreadCount);
    vector<ring_queue<SSockItem> > vrq(m_nThreadCount);

    SSockItem invalidSsock = {SOCKET_ERROR, "", 0};
    for(size_t i = 0; i < vrq.size(); ++i){
        FALSE_RETURN_STRERROR(!vrq[i].init(QUEUE_MAX_LEN, invalidSsock));
    }
    int i;
    for (i = 0; i < m_nThreadCount; i++){       
        vpara[i].rq = &vrq[i];
        vpara[i].psf = this;
        vpara[i].threadNo = i;
        if (0 != thrMngr.createThread(serverThread, &vpara[i])){
            break;  
        }       
    }
    printf("expect thread count %d, real count %d\n",m_nThreadCount,i);
    FALSE_RETURN_STRERROR(i == 0)
    printf("Expected Param\nbucket_count:%u\nslice_size:%u\nvalue_size:%uBytes\nexpire_time:%us\nmax_size:%uM\n",m_BucketCnt, m_sliceSz, m_valSz,m_expireTime, m_maxSz);
    printf("server start to run.........\n");

    SOCKET hClientSock;
    char chRecvBuf [RECV_BUF_LEN];
    int nRetCode;
    CHttpAnalyse xa;
    while(!m_bShutdown){
        hClientSock = accept(m_lsnSock, NULL, NULL);
        CCommonFunc::setTimeOut(hClientSock, m_clientRecvTimeout, m_clientSendTimeout);
        if(hClientSock == SOCKET_ERROR && !m_bShutdown){
            COMMON_ERROR_INFO
            continue;
        }
        u64 hasher;
        SSockItem sSock; 
        hash_map<string, string> hssPara;
        string strRec;
        while(true){
            memset(chRecvBuf, 0, sizeof(chRecvBuf));
            nRetCode = recv(hClientSock, chRecvBuf, RECV_BUF_LEN - 1, 0);   
            if(nRetCode > 0){
                strRec += chRecvBuf;
                if(strRec.find(" HTTP") != string::npos){
                    break;
                }
            }else{
                break;
            }
        }

        const char* end;
        const char* mid;
        end = strstr(strRec.c_str(), " HTTP");
        mid = strstr(strRec.c_str(), "?");
        if(end && mid && end>mid){
            strRec = string(mid+1,end);
        }
        xa.parseRequest(strRec, hssPara);

        //for debug show ring_queues
        if(hssPara.find("view") != hssPara.end()){
            char chViewRes[128];
            string strView = "";
            char tmp[128];
            for(size_t i = 0; i< m_nThreadCount; i++){
                vrq[i].view(chViewRes);
                sprintf(tmp, "thread %d: ", (int)i);
                strView = strView + tmp + chViewRes;
            }
            if(SOCKET_ERROR == send(hClientSock, strView.c_str(), strView.length(), 0)){
                fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
            }
            closesocket(hClientSock);
            continue;
        }


        sSock.sock = hClientSock;
        sSock.debug = false;
        if(hssPara.find(URL) != hssPara.end()){
            hasher          = sd::hash_wrap(hssPara[URL]);
            sSock.url       = hssPara[URL];
            sSock.hasher    = hasher;
        }else{
            sSock.url       = "";
            sSock.hasher    = 0;
        }
        if(hssPara.find(DEBUG_MODE) != hssPara.end()){
            sSock.debug = true;
        }

        //hash into thread request queue
        if(!vrq[sSock.hasher & (m_nThreadCount - 1)].in(sSock)){
            closesocket(hClientSock);//sock queue's full, abandon
        }
    }

    if (thrMngr.waitMultipleThread() != 0){
        return false;//waitting for all thread over
    }
    printf("Server shutdown ok............\n");
    return true;
}

void* CServerFrame::serverThread(void *lpParameter){
    SPara *pPara=(SPara*)lpParameter;
    CServerFrame* psf = pPara->psf;
    ring_queue<SSockItem>* rq = pPara->rq;
    sd::ddcache<string> cache;
    char chCacheFile[1024];
    //here must set names different or threads will be use same memory
    sprintf(chCacheFile, "./cache/cache.thread%d.bin", pPara->threadNo);
    cache.init(psf->m_BucketCnt, psf->m_valSz, psf->m_sliceSz, 
            chCacheFile, psf->m_maxSz, psf->m_expireTime, false);
    SSockItem sSock;
    CHttpRequest hr;
    char* chRes = new char[MAX_VAL_LEN];
    string strSend;
    while(!m_bShutdown){
        while(rq->isEmpty() && !m_bShutdown){
            usleep(100);
        }
        if(m_bShutdown){
            break;
        }
        sSock = rq->out();
        if(sSock.debug == true){
            char chView[1024];
            cache.view_header(chView, 1024);
            snprintf(chRes, MAX_VAL_LEN, "threadNO: %d</br>\n%s", pPara->threadNo, chView);
            psf->_addHeader(strSend, chRes, "text/html");
        }else{
            if(cache.get(sSock.url, chRes) != DC_OK){
                if(hr.getUrl(sSock.url, strSend, &(psf->m_cacheNameToHost),
                            psf->m_targetRecvTimeout, psf->m_targetSendTimeout)){//get from url
                    cache.put(sSock.url, strSend.c_str());
                }else{//show error str;
                    psf->_errorStr(strSend);
                    psf->_addHeader(strSend, strSend.c_str(), "text/xml");
                }
            }else{
                strSend = chRes;
            }
        }

        SOCKET hClientSock = sSock.sock;
        if(SOCKET_ERROR == send(hClientSock, strSend.c_str(), strSend.length()+1, 0)){
            fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
        }
        closesocket(hClientSock);
    }
    delete[] chRes;
    return 0;
}//while(!m_bShutdown)

bool CServerFrame::closeServer(){
    m_bShutdown = true;
    FALSE_RETURN_STRERROR(SOCKET_ERROR == closesocket(m_lsnSock));
    int sockfd;
    struct sockaddr_in dest;
    FALSE_RETURN_STRERROR((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)

    //create a test socket to ensure server socket is closed
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(m_lsnPort);
    FALSE_RETURN_STRERROR(inet_aton("127.0.0.1", (struct in_addr *) &dest.sin_addr.s_addr) == 0)

    connect(sockfd, (struct sockaddr *) &dest, sizeof(dest));
    closesocket(sockfd);
    return true;
}

int CServerFrame::getLocalIP(char* dest, socklen_t size){
    int sock;
    struct sockaddr_in sin;
    struct ifreq ifr;
    sock = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sock == -1){
        return -1; 
    }   

    strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if(ioctl(sock, SIOCGIFADDR, &ifr) < 0){ 
        return -2; 
    }   
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    if(inet_ntop(AF_INET, &(sin.sin_addr), dest, size) == NULL){
        return -3; 
    }   
    return 0;
}

void CServerFrame::_errorStr(string &strSend){
    char* chTmp = "<?xml version=\"1.0\" encoding=\"GBK\"?><result><Header><ErrorCode>-1</ErrorCode></Header><Body><ErrorInfo>url illegal</ErrorInfo></Body></result>";
    strSend = chTmp;
}


void CServerFrame::_addHeader(string &strRes, const char* chContent, char* chType){
    int nContentLen = strlen(chContent);
    char chTmp[2048];
    sprintf(chTmp, "HTTP/1.1 200 OK\r\nConnection: close\r\nServer: FrameServer/1.0.0\r\nContent-Type: %s; charset=GB2312\r\nContent-Length: %d\r\n\r\n%s", chType, nContentLen, chContent);
    strRes = chTmp;
}
