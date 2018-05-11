
#ifndef _KERNAL_TIMER_H_
#define _KERNAL_TIMER_H_

#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <vector>
#include "KernalLock.h"
#include "KernalMap.h"
#include "KernalThread.h"

#define TIMER_SLOT 64 //时间轮槽位

struct KernalTimerNode
{
    unsigned int     id;
    unsigned int     expire;
    unsigned int     expireTime;
    int              time;
    KernalTimerNode  *pre;
    KernalTimerNode  *next;

    KernalTimerNode()
        :id( 0 )
        ,expire( 0 )
        ,expireTime( 0 )
        ,time( -1 )
        ,pre( NULL )
        ,next( NULL )
    {

    }
};

struct KernalTimerNodeList
{
	KernalMutexLocker      locker;
    struct KernalTimerNode *head;
	struct KernalTimerNode *tail;

	void lock()
	{
		locker.lock();
	}
	void unlock()
	{
		locker.unlock();
	}
};

class KernalTimer
{
public:
    KernalTimer();
    ~KernalTimer();

	void init();
    unsigned int addTimer( unsigned int expire, int time/*执行次数：-1 无限*/ );
    void deleteTimer( unsigned int id );

    unsigned int gettime();
public:	
	void initThreadTimer();
    unsigned int popExpired();
	int getMinTimerExpire();  // 获取最近过期时间的定时器的expire
private:    
    unsigned int        m_StartTime;
	KernalMutexLocker   m_TimerLocker;
	std::map< pthread_t, KernalTimerNodeList > m_Timers;
};

#endif
