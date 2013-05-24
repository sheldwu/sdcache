#ifndef RING_QUEUE_H_
#define RING_QUEUE_H_
#include<stdlib.h>
#include<stdio.h>

template <class T>
class ring_queue{
    private:
        int m_front;
        int m_rear;
        int m_sz;
        T   m_inval_val;
        T*  m_data;
    public:
        ring_queue(){
            m_data = 0;
        }

        bool init(unsigned int sz, T invalid_val){
            m_sz = sz;
            m_front = m_rear = 0;
            m_data = new T[m_sz + 1];
            m_inval_val = invalid_val;
            return m_data != 0;
        }

        ~ring_queue(){
            delete[] m_data;
        };

        bool isEmpty(){
            return m_front == m_rear;
        }

        bool isFull(){
            return (m_front == m_rear + 1) || (m_rear - m_front == m_sz);
        }

        bool in(T data){
            if(isFull()){
                return false;
            }
            m_data[m_rear] = data;
            if(m_rear == m_sz){ 
                m_rear = 0;
            }else{
                m_rear++;
            }
            return true;
        }

        T out(){
            if(isEmpty()){
                return m_inval_val;
            }
            T res = m_data[m_front];
            if(m_front == m_sz){
                m_front = 0;
            }else{
                m_front++;
            }
            return res;
        }

        void view(char* res){
            sprintf(res, "front: %d; rear: %d</br>\r\n", m_front, m_rear);
        }
};
#endif
