
#ifndef _KERNAL_TIMER_H_
#define _KERNAL_TIMER_H_

#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include<sys/time.h>
#include <vector>
#include "KernalLock.h"

#define TIMER_SLOT 64 //时间轮槽位

struct KernalTimerNode
{
    unsigned int     id;
    unsigned int     expire;
    unsigned int     expireTime;
    int              time;
    KernalTimerNode *pre;
    KernalTimerNode *next;

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
    struct KernalTimerNode *head;
	struct KernalTimerNode *tail;
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

    void update();
    void expired( int slot );
public:
    unsigned int pop();    
private:   
    void push( unsigned int id );
    void deleteTimer( struct KernalTimerNode *pTimerNode ); 
private:    
    unsigned int        m_StartTime;
    int                 m_CurrentSlot;  // 当前槽位
	KernalMutexLocker   m_TimerLocker;
	KernalMutexLocker   m_ExpriedTimerLocker;

    KernalTimerNodeList m_TimerNodes[ TIMER_SLOT ];
    std::vector< unsigned int > m_expriedTimers;
};

#endif
