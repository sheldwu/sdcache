#ifndef HTTPANALYSE_H_
#define HTTPANALYSE_H_ 

#include <string>
#include "hash_wrap.h"

#define ADD_INT_XMLNODE(xml,chBuf,nodeName,value) {sprintf(chBuf, "<%s>%d</%s>\n", nodeName,value,nodeName);xml += chBuf;}



using namespace std;

class CHttpAnalyse{
public:
    string urlEncode(string sIn);
    string urlDecode(const string& in,string& ret);
    int parseRequest(string strRecvPara,hash_map<string,string>& hmStrStr);
    CHttpAnalyse(){};
    ~CHttpAnalyse(){};
private:
    inline unsigned char toHex(const unsigned char &x){
        return  x > 9 ? x + 55: x + 48;
    }
};

#endif

