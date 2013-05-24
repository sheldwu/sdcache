#include "ddcache.h"
#include "HttpRequest.h"
bool ddcache::get(string &url, char* chRes, ul* ulLen){
    CHttpRequest hr;
    char* chBuf = new char[MAX_VAL_LEN_PLUS];
    hr.getUrl(url, chBuf, 3, 3);
    int a;
    if((a = uncompress((Byte*)chRes, (uLongf*)ulLen, (Byte*)(chBuf+sizeof(ul)), *(ul*)chBuf)) != Z_OK){
        printf("uncompress return :%d\n", a);
        delete[] chBuf;
        return false;
    }
    delete[] chBuf;
    return true;
};
/*
bool ddcache::get(string &url, char* chRes, ul* ulLen){
    CHttpRequest hr;
    hr.getUrl(url, chRes, 3, 3);
    return true;
}
*/
