#include "HttpAnalyse.h"

//analyse request and store in hash_map
int CHttpAnalyse::parseRequest(string strRecvPara,hash_map<string,string>& hmStrStr){
    if(strRecvPara.length()==0)
        return 0;

    string::iterator itStr;
    string::iterator itStrT;
    string m_strEncodedKeyWord;
    string strName,strValue;

    for (itStr=strRecvPara.begin();itStr!=strRecvPara.end();++itStr){
        itStrT=itStr;
        while(itStr!=strRecvPara.end()&&*itStr!='='){
            ++itStr;
        }

        if (itStr==strRecvPara.end()){
            break;          
        }else{
            strName.assign(itStrT,itStr);
            if (strValue.empty()){
                strValue="";
            }
            itStrT=itStr;
        }
        
        while (itStr!=strRecvPara.end()&&*itStr!='&'){
            ++itStr;
        }
        
        strValue.assign(++itStrT,itStr);
        if(strValue.empty()){
            strValue="";
        }
        
        string ret;
        urlDecode(strValue,ret);
        ret=ret.c_str();

        hmStrStr[strName]=ret;  
        if(itStr==strRecvPara.end()){
            break;
        }
    }
    return 0;
}

string CHttpAnalyse::urlEncode(string sIn){
    string sOut;
    const int nLen = sIn.length();
    //const char* p=sIn.c_str();
    unsigned char ch;
    for(int i=0;i<nLen;i++){
        ch=sIn[i];
        if(isalnum(ch)){
            sOut+=ch;
        }else if(isspace(ch)){
            sOut+="%20";
        }else{
            sOut+="%";
            sOut+=toHex(ch>>4);
            sOut+=toHex(ch%16);
        }
    }
    return sOut;
}

string CHttpAnalyse::urlDecode(const string& in,string& ret){
    string hex;
    string::const_iterator it = in.begin();
    while(it != in.end()){
        switch(*it){
        case '+':
            ret.push_back(' ');
            ++it;
            break;
        case '%':
            hex = "";
            ++it;
            if(it != in.end()){
                hex.push_back(*it);
                ++it;
                if(it != in.end()){
                    hex.push_back(*it);
                    long r = strtol(hex.c_str(), NULL, 16);
                    if((0 < r) && (r < 256)){
                        ret.push_back(r);
                    }
                    ++it;
                }
            }
            break;
        default:
            ret.push_back(*it);
            ++it;
        }
    }
    return ret;
}

