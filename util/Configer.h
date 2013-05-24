#ifndef CONFIGER_H_
#define CONFIGER_H_
#include <ext/hash_map>
#include <stdio.h>
#include <string>
#include "hash_wrap.h"
using namespace std;
using namespace __gnu_cxx;
#define HSS hash_map<string, string> 
class Configer{
    public:
        Configer(){};
        ~Configer(){};
    public:
        bool parseConfig(char* chFile, HSS& hssConf){
            char chBuf[2048];
            char* chPtr;
            FILE* fd = fopen(chFile, "r");
            if(!fd){return false;}
            while(!feof(fd)){
                if(!fgets(chBuf, 2048, fd)){
                    break;
                }
                int nLen = strlen(chBuf);
                for(chPtr = chBuf+nLen-1; chPtr >= chBuf; chPtr--){
                    if(*chPtr == '\r' || *chPtr == '\n'){
                        *chPtr = '\0';
                        break;
                    }
                }
                for(chPtr = chBuf; *chPtr != '='; chPtr++){
                }
                *chPtr = '\0';
                hssConf[chBuf] = chPtr+1;
            }
            return true;
        }
};
#endif
