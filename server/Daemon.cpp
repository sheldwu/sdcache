#include "Daemon.h"
int CDaemon::m_nChildPid=0;
CServerFrame* CDaemon::m_pServer = NULL;
void CDaemon::initAsDaemon(){
    if(fork() > 0){
        exit(0);
    }
    setsid();

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTERM, sigMasterHandler);
    signal(SIGINT,  sigMasterHandler);
    signal(SIGQUIT, sigMasterHandler);
    signal(SIGKILL, sigMasterHandler);
}

void CDaemon::sigMasterHandler(int sig){       
    kill(m_nChildPid, SIGUSR1);
    fprintf(stdout, "master [%d] sig [%d]!\n",getpid(), sig);
}

void CDaemon::sigChildHandler(int sig){
    m_pServer->closeServer();
    fprintf(stdout, "child  [%d] sig [%d]!\n",getpid(),sig);
}

bool CDaemon::run(int argc,char** argv){
    parseCmdLine(argc,argv);
    m_pName = strrchr(argv[0],'/');
    m_pName != NULL ? m_pName += 1 : m_pName = argv[0];

    HISI i=m_hisOptVal.find('k');
    if (i!=m_hisOptVal.end()){
        if (i->second=="start"){
            return start();
        }else if(i->second=="stop"){
            return stop();
        }else if(i->second=="restart"){
            if(stop()){
                return start();
            }
            return false;
        }
    }
    return true;
}

int CDaemon::parseCmdLine(int argc,char** argv){
    for (int i=1; i<argc; ++i){
        if(argv[i][0] != '-'){
            continue;
        }

        if(i == argc - 1){
            break;
        }

        if (argv[i+1][0] == '-'){
            m_hisOptVal[argv[i][1]]="";
            continue;
        }

        m_hisOptVal[argv[i][1]] = argv[i+1];
        ++i;
    }
    return m_hisOptVal.size();
}

/* modify from book apue
*  when son dies ,father restart it except for:1 son exit with "exit" or kill by signal 9
*/
bool isAbnormalExit(int pid, int status){
    bool bRestart = true;
    if(WIFEXITED(status)){ //exit()or return
        fprintf(stdout, "Child normal termination(pid [%d], status [%d])\n", pid, WEXITSTATUS(status));
        bRestart = false;
    }
    else if(WIFSIGNALED(status)){ //signal
        fprintf(stderr, "Abnormal termination(pid [%d], signal number [%d]\n", pid, WTERMSIG(status));
        if(WTERMSIG(status) == SIGKILL){
            bRestart = false;
            fprintf(stderr, "Killed by user?(exit pid [%d], status = %d)\n", pid, WEXITSTATUS(status));
        }
    }
    else if(WIFSTOPPED(status)){//暂停的子进程退出
         fprintf(stderr, "Child stopped(pid [%d], signal number [%d])\n", pid, WSTOPSIG(status));
    }
    else{
         fprintf(stderr, "Unknown exit(pid [%d], signal number [%d]\n", pid, WSTOPSIG(status));
    }
    return bRestart;
}

bool CDaemon::start(){
    //get daemon pid from file
    char buf[640];
    int masterPid;
    
    string strName = m_runPath + daemon_pid_file;
    strName = strName + "." + m_pName;
    if ( 0<read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0){
        if(kill(masterPid, 0) == 0){
            printf("Instance's running, ready to quit!\n");
            return true;
        }
    }
    initAsDaemon();
    sprintf(buf, "%d", getpid());
    if(!writeBuff2File(strName.c_str(), buf, "w")){
        fprintf(stderr, "Write master pid fail!\n");
    }

    while(true){//daemon fork and son do jobs
        pid_t pid = fork();
        if (pid == 0){
            signal(SIGUSR1, sigChildHandler);
            signal(SIGPIPE, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTERM, SIG_IGN);
            signal(SIGINT,  SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
            
            if(!initServer()){   
                fprintf(stderr, "Server init  fail!\n");
                return false;
            }
            fprintf(stdout, "Server init  ok pid = %d\n",(int)getpid());

            if(!runServer()){
                fprintf(stderr, "run fail!\n");
                return false;
            }
            fprintf(stdout, "Server run ok!\n");
            exit(0);
        }
        m_nChildPid=pid;
        int status;
        pid = wait(&status);
        if(!isAbnormalExit(pid, status)){
            fprintf(stdout, "Child exit!\n");
            break;
        }
    }
    return true;
}

int CDaemon::read1LineFromFile(const char* fileName, char* buf, int maxCount, const char* mode){
    FILE* fp = fopen(fileName, mode);
    if (!fp){
        return 0;
    }
    int ret;
    fgets(buf, maxCount, fp) ? ret = 1 : ret = 0;
    fclose(fp);
    return ret;
}

int CDaemon::writeBuff2File(const char* fileName, const char* buf, const char* mode){
    FILE* fp = fopen(fileName, mode);
    if(!fp){
        return 0;
    }
    int n = fprintf(fp, "%s", buf);
    fclose(fp);
    return n;
}



bool CDaemon::initServer(){
    int nPort = 0, nThreadNum = 0;
    u32 cacheBucketCnt, cacheValSz, cacheSliceSz, cacheExpireTime;
    u64 cacheMaxSz; 
    HISI i = m_hisOptVal.find('n');   
    if(i != m_hisOptVal.end()){
        nThreadNum = sd::get_2pow_size(atoi(i->second.c_str()));
    }

    i = m_hisOptVal.find('p');   
    if(i != m_hisOptVal.end()){
        nPort=atoi(i->second.c_str());
    }

    if(nThreadNum<=0||nPort<=0){
        return false;
    }

    if((i =  m_hisOptVal.find('b')) !=  m_hisOptVal.end()){
        cacheBucketCnt = atoi(i->second.c_str());
    }else{
        cacheBucketCnt = 120000;
    }

    if((i =  m_hisOptVal.find('v')) !=  m_hisOptVal.end()){
        cacheValSz = atoi(i->second.c_str());
    }else{
        cacheValSz = 1000;
    }

    if((i =  m_hisOptVal.find('s')) !=  m_hisOptVal.end()){
        cacheSliceSz = atoi(i->second.c_str());
    }else{
        cacheSliceSz = 100000;
    }

    if((i =  m_hisOptVal.find('m')) !=  m_hisOptVal.end()){
        cacheMaxSz = atoi(i->second.c_str());
    }else{
        cacheMaxSz = 2*1024;//total 2G, quantifier is M
    }

    if((i =  m_hisOptVal.find('e')) !=  m_hisOptVal.end()){
        cacheExpireTime = atoi(i->second.c_str());
    }else{
        cacheExpireTime = 365*86400;//1 Year
    }

    return m_pServer->createServer(nPort, nThreadNum, cacheBucketCnt, cacheValSz, cacheSliceSz, cacheMaxSz, cacheExpireTime);
}

bool CDaemon::stop(){
    char buf[640];
    int masterPid;
    
    string strName = m_runPath + daemon_pid_file;
    strName = strName + "." + m_pName;

    if ( 0<read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0){
        if (kill(masterPid, 0) == 0){
            int tryTime = 200;      
            kill(masterPid, SIGTERM);
            while (kill(masterPid, 0) == 0 && --tryTime){
                sleep(1);           
            }
            if (!tryTime && kill(masterPid, 0) == 0){
                fprintf(stderr, "Time out shutdown fail!\n");       
                return false    ;
            }
            return true;
        }
    }
    printf("Another instance doesn't exist, ready to quit!\n");
    return true;
}

