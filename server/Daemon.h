#ifndef DAEMON_H_
#define DAEMON_H_
#include "ComDef.h"
#include "ServerFrame.h"
#include <signal.h>
#include <sys/wait.h>
#define daemon_pid_file "daemon.pid"
class CDaemon
{
public:
    CDaemon(CServerFrame* psf){
        char buf[128];
        getcwd(buf, 128);
        m_runPath = buf;
        m_runPath += "/";
        m_pServer = psf;
    }
    ~CDaemon(){};

    //read 1st line 
    int read1LineFromFile(const char* fileName, char* buf, int maxCount, const char* mode);
    //write to file
    int writeBuff2File(const char* fileName, const char* buf, const char* mode);


    int parseCmdLine(int argc,char** argv);
    bool start();
    bool stop();
    static void initAsDaemon();
    static void sigMasterHandler(int sig);
    static void sigChildHandler(int sig);
    bool run(int argc,char** argv);

private:
    bool initServer();
    bool runServer(){return m_pServer->runServer();};

private:
    HIS             m_hisOptVal;
    string          m_runPath;
    char*           m_pName;//excute file name
    static  int     m_nChildPid;
    static CServerFrame*    m_pServer;
};
#endif
