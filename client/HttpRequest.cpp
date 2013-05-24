#include "HttpRequest.h"
#include "CommonFunc.h"

/*
 *@author wuxudong wuxudong@dangdang.com
 *@param url         :    target url
 *@param serverstrp  :    char* to store host
 *@param portp       :    int* to store port
 *@param pathstrp    :    char** potiner be change to the path(rest of the url)
 *return             :    parse res   
*/
bool CHttpRequest::parseUrl(const char *url, char *serverstrp, int *portp,  const char **pathstrp){
    char buf[SERVER_LEN];
    const char* s;//start pos
    const char* e;//end pos of server+port
    const char* s1;//start pos of path 
    const char* m;//pos of ':'
    if((s = strstr(url, "http://")) != NULL){
        s = url + 7;
    }else{
        s = url;
    }
    e = strstr(s, "/");

    if(e == NULL){//no '/' use '?' as delimiter,but remember to add '?' to path
        e = strstr(s, "?");
        if(e != NULL){
            s1 = e;
        }
    }else{
        s1 = e + 1;
    }

    if(e == NULL){
        if(strlen(s) > SERVER_LEN-1){//server too long
            return false;
        }
        strcpy(buf, s);
        *pathstrp = "\0";
    }else{
        if(e-s > SERVER_LEN-1){//server too long
            return false;
        }
        memcpy(buf, s, e - s);
        buf[e-s] = '\0';
        *pathstrp = s1;
    }

    if((m = strstr(buf, ":")) == NULL){
        strcpy(serverstrp, buf);
        *portp = 80;
    }else{
        memcpy(serverstrp, buf, m-buf);
        (serverstrp)[m-buf] = '\0';
        *portp = atoi(m+1);
    }
    return true;
}

/*
 *function get data from url
 * @para string url         : target url
 * @para char* chRecv       : start address of char to recv reply, suppose to be MAX_VAL_LEN_PLUS
 * @para int nRecvTimeout   : socket recv timeout
 * @para int nSendTimeout   : socket send timeout
 * return bool              : if success get
 * */
bool CHttpRequest::getUrl(string &url, char* chRecv, int nRecvTimeout, int nSendTimeout){
    char chServer[SERVER_LEN];
    char chHost[SERVER_LEN];
    const char *path;
    int port;
    if(!parseUrl(url.c_str(), chServer, &port, &path)){
        return false;
    }

    if(hssDNS.find(chServer) == hssDNS.end()){
        if(getHostByName(chServer, chHost, SERVER_LEN) == NULL){
            return false;
        }
        hssDNS.insert(make_pair(chServer, chHost));
    }
    strcpy(chHost,(hssDNS.find(chServer))->second.c_str());

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    CCommonFunc::setTimeOut(sockfd, nRecvTimeout, nSendTimeout);//set socket in&out timeout
    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);
    memset(&clientAddr, 0, len);
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(chHost);//
    clientAddr.sin_port = htons(port);
    if(connect(sockfd, (sockaddr*)&clientAddr, len)<0){
        return false;
    }
    char chHeader[HEADER_LEN];
   sprintf(chHeader, "GET /%s HTTP/1.1\r\nAccept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/x-ms-application, application/x-ms-xbap, application/vnd.ms-xpsdocument, application/xaml+xml, application/x-silverlight, application/vnd.ms-excel\r\nAccept-Language: zh-cn\r\nUA-CPU: x86\r\nAccept-Encoding: gzip, deflate\r\nUser-Agent: Mozilla/4.0 (compatible MSIE 7.0 Windows NT 5.1 .NET CLR 2.0.50727 .NET CLR 3.0.04506.648 .NET CLR 3.5.21022)\r\nHost: %s\r\nConnection: Keep-Alive", path, chServer);
   //  sprintf(chHeader, "GET /%s HTTP/1.1\r\n\r\n", path, chServer);
    if(send(sockfd, chHeader, strlen(chHeader), 0) < 0){
        return false;
    }
    int ret_code = 1, nRecved = 0;
    char* chBuf = new char[RECV_BUF];
    while(true){
        bzero(chBuf, RECV_BUF);
        ret_code = recv(sockfd, chBuf, RECV_BUF - 1, 0);
        if(ret_code < 0){
            delete[] chBuf;
            return false;
        }
        if(ret_code == 0){
            break;
        }
        memcpy(chRecv + nRecved, chBuf, ret_code);
        nRecved += ret_code;
    }
    close(sockfd);
    delete[] chBuf;
    return true;
}

char* CHttpRequest::getHostByName(char* chName, char* const chHost, int nLen){
    struct hostent *hePtr;
    if((hePtr = gethostbyname(chName)) == NULL){
        return NULL;
    }
    if(inet_ntop(hePtr->h_addrtype, hePtr->h_addr, chHost, nLen) == NULL){
        return NULL;
    }
    return chHost;
}

bool CHttpRequest::setTimeOut(int sock, int nRecvOut, int nSendOut){
    struct timeval to;
    to.tv_sec   = nSendOut;
    to.tv_usec  = 0;

    struct timeval in;
    in.tv_sec   = nRecvOut;
    in.tv_usec  = 0;

    bool bSuc = true;
    if(-1 == setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&in,sizeof(in))){
        bSuc = false;
    }
    if(-1  == setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&to,sizeof(to))){
        bSuc = false;
    }
    return bSuc;
};
