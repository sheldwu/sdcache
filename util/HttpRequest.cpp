#include "HttpRequest.h"
#include "CommonFunc.h"

/*
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

    if(e == NULL){//no '/' use '?' as delimiter,but remember to add '?'
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

bool CHttpRequest::getUrl(string &url, string &res, sd::ddcache<string>* cacheNameToHost, 
                int nRecvTimeout, int nSendTimeout){
    char server[SERVER_LEN];
    char chHost[SERVER_LEN];
    const char *path;
    int port;
    if(!parseUrl(url.c_str(), server, &port, &path)){
        return false;
    }
    if(cacheNameToHost->get(server, chHost) != DC_OK){
        if(getHostByName(server, chHost, SERVER_LEN) == NULL){
            return false;
        }
        cacheNameToHost->put(server, chHost);
    }

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
    res = "";
    char chHeader[HEADER_LEN];
    sprintf(chHeader, "GET /%s HTTP/1.1\r\nAccept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/x-ms-application, application/x-ms-xbap, application/vnd.ms-xpsdocument, application/xaml+xml, application/x-silverlight, application/vnd.ms-excel\r\nAccept-Language: zh-cn\r\nUA-CPU: x86\r\nAccept-Encoding: gzip, deflate\r\nUser-Agent: Mozilla/4.0 (compatible MSIE 7.0 Windows NT 5.1 .NET CLR 2.0.50727 .NET CLR 3.0.04506.648 .NET CLR 3.5.21022)\r\nHost: www.baidu.com\r\nConnection: Keep-Alive", path);
    if(send(sockfd, chHeader, strlen(chHeader), 0) < 0){
        return false;
    }
    int ret_code = 1;
    char* chRecv = new char[RECV_LEN];
    while(true){
        bzero(chRecv, RECV_LEN);
        ret_code = recv(sockfd, chRecv, RECV_LEN -1, 0);
        if(ret_code < 0){
            delete[] chRecv;
            return false;
        }
        if(ret_code == 0){
            break;
        }
        res += chRecv;
    }
    close(sockfd);
    delete[] chRecv;
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
