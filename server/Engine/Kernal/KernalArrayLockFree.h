
#ifndef _KERNAL_ARRAY_LOCK_FREE_H_
#define _KERNAL_ARRAY_LOCK_FREE_H_
#include <iostream>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#define CAS(ptr , oldvalue , newvalue) __sync_bool_compare_and_swap(ptr , oldvalue , newvalue)
#define ARRAYDEFAULTSIZE 10000

template<typename T , uint32_t arraysize = ARRAYDEFAULTSIZE> class KernalArrayLockFree
{
public:
    KernalArrayLockFree();
    bool push(T);
    T pop();
private:
    T m_Queue[arraysize];
    uint32_t m_CurrentWriteIndex;
    uint32_t m_CurrentReadIndex;
    uint32_t m_MaxReadIndex;
    inline uint32_t countToIndex(uint32_t);
};

template<typename T , uint32_t arraysize> KernalArrayLockFree<T , arraysize>::KernalArrayLockFree()
{
    m_CurrentWriteIndex = 0;
    m_CurrentReadIndex = 0;
    m_MaxReadIndex = 0;
}

template<typename T , uint32_t arraysize> inline uint32_t KernalArrayLockFree<T , arraysize>::countToIndex(uint32_t count)
{
    return (count%arraysize);
}

template<typename T , uint32_t arraysize> bool KernalArrayLockFree<T , arraysize>::push(T element)
{
    uint32_t CurrentWriteIndex;
    uint32_t CurrentReadIndex;

    do
    {
        CurrentReadIndex = m_CurrentReadIndex;
        CurrentWriteIndex = m_CurrentWriteIndex;


        if(countToIndex(CurrentWriteIndex + 1) == countToIndex(CurrentReadIndex))
		{
            return NULL;
		}
		
        if(!CAS(&m_CurrentWriteIndex , CurrentWriteIndex , CurrentWriteIndex + 1))
		{
            continue;
		}
		
        m_Queue[countToIndex(CurrentWriteIndex)] = element;
        break;

    }while(1);

    while(!CAS(&m_MaxReadIndex , CurrentWriteIndex , CurrentWriteIndex + 1))
    {
        sched_yield();
    }

    return true;
}

template<typename T , uint32_t arraysize> T KernalArrayLockFree<T , arraysize>::pop()
{
    uint32_t CurrentReadIndex;

    do
    {
        CurrentReadIndex = m_CurrentReadIndex;

        if(countToIndex(CurrentReadIndex) == countToIndex(m_MaxReadIndex))
		{
            return NULL;
		}
		
        T result = m_Queue[countToIndex(CurrentReadIndex)];

        if(!CAS(&m_CurrentReadIndex , CurrentReadIndex , CurrentReadIndex + 1))
		{
            continue;
		}
		
        return result;

    }while(1);
}

#endif
