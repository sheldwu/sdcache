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
        //bWaitAll�Ƿ������߳�  : Ĭ��ֵ1�ȴ������߳�,0���κ��߳̽������˺�������
        //dwMilliseconds        : ��λ���룬Ĭ��ֵ����ʱ��
        //return value          : -1û���κξ��������ֵ WaitForMultipleObjects�����ķ���ֵ
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
        vector<pthread_t> m_vecHandle;   //�߳̾���ͺ���������б�
    private:
        threadManager(const threadManager&){;}//��ֹ����
        void operator=(const threadManager &){;}//��ֹ��ֵ           
    };
#endif
