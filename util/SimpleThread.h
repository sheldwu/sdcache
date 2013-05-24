#ifndef SIMPLETHREAD_H_
#define SIMPLETHREAD_H_

#define INFINITE 0

typedef void *(* PThreadFunc)(void* param);
typedef int DWORD;

class threadManager{
    public:
        threadManager(){}
        ~threadManager(){}

        //close threads which might still be running
        void clear(){
            m_vecHandle.clear();
        }

        //pFunc pointer of thread_fun: unsigned __stdcall ThreadFunc(void *param )
        //pPara thread_fun para
        //return value success:0 ;otherwise errno
        int createThread(PThreadFunc pFunc, void *pPara){   
            pthread_t pt;
            int nErrorCode=pthread_create(&pt, NULL, pFunc, pPara);
            if(nErrorCode != 0){
                return nErrorCode;
            }
            m_vecHandle.push_back(pt);  //add to thread array, used by WaitForMultipleObjects
            return nErrorCode;
        }



        //wait for all thread stop
        //bWaitAll是否所有线程  : 默认值1等待所有线程,0有任何线程结束，此函数返回
        //dwMilliseconds        : 单位毫秒，默认值无穷时间
        //return value          : -1没有任何句柄，其他值 WaitForMultipleObjects函数的返回值
        DWORD  waitMultipleThread( bool bWaitAll = 1,DWORD dwMilliseconds = INFINITE){
            if (m_vecHandle.empty()){
                return -1;  
            }
            int nErrorcode;
            for (size_t i=0; i < m_vecHandle.size(); ++i){
                nErrorcode=pthread_join(m_vecHandle[i], NULL); 
                if (nErrorcode!=0){
                    return nErrorcode;  
                }
            }   
            return 0;
        }

    private:
        vector<pthread_t> m_vecHandle;   //线程句柄和函数对象的列表
    private:
        threadManager(const threadManager&){;}//禁止拷贝
        void operator=(const threadManager &){;}//禁止赋值           
    };
#endif
