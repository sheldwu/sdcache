#ifndef COMMON_FUNC_H_
#define COMMON_FUNC_H_
class CCommonFunc{
    public:
    static bool setTimeOut(int sock, int nRecvOut, int nSendOut){
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
};
#endif
