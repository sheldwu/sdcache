#include "Daemon.h"

int main(int argc, char* argv[]){
    if(argc < 7){
        fprintf(stderr, "usage %s -n THREAD_NUMBER -p LISTEN_PORT  -k START|STOP [-b BUCKET_COUNT -v VALUE_SIZE -s SLICE_SIZE -m MAX_SIZE -e EXPIRE_TIME]\n", argv[0]);
    }
    CServerFrame csf;
    CDaemon daemon(&csf);
    daemon.run(argc, argv);
    return 0;
}
