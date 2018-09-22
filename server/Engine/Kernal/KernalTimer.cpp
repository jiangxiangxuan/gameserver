
#include "KernalTimer.h"
#include "KernalGuid.h"

KernalTimer::KernalTimer()
{

}

KernalTimer::~KernalTimer()
{

}

void KernalTimer::init()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

    m_StartTime = tv.tv_sec;
    tv.tv_usec / 10000;
}

unsigned int KernalTimer::addTimer( unsigned int expire, int time )
{
    unsigned int id = getGuid();
    KernalTimerNode *pTimerNode = new KernalTimerNode();
    pTimerNode->id     = id;
    pTimerNode->expire = expire;
    pTimerNode->time   = time;
    pTimerNode->expireTime = gettime() + pTimerNode->expire;
	
	m_TimerLocker.lock();
	pTimerNode->next = m_Timers.head;
	if( m_Timers.head )
	{
		m_Timers.head->pre = pTimerNode;
	}
	m_Timers.head = pTimerNode;
	m_TimerLocker.unlock();

    return id;
}

void KernalTimer::delTimer( unsigned int id )
{
	m_TimerLocker.lock();
	KernalTimerNode *pTimerNode = m_Timers.head;
	while( pTimerNode )
	{
		if( pTimerNode->id == id )
		{
			if( m_Timers.head == pTimerNode )
			{
				m_Timers.head = pTimerNode->next;
			}
			else
			{
				if( pTimerNode->pre )
				{
					pTimerNode->pre->next = pTimerNode->next;
				}
				if( pTimerNode->next )
				{
					pTimerNode->next->pre = pTimerNode->pre;
				}
			}
			delete pTimerNode;
			pTimerNode = NULL;
			break;
		}
		pTimerNode = pTimerNode->next;
	}
	m_TimerLocker.unlock();
}

unsigned int KernalTimer::gettime()
{
    unsigned int t;
    struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint64_t)tv.tv_sec * 100;
	t += tv.tv_usec / 10000;
	return t;
}

unsigned int KernalTimer::popExpired()
{
    unsigned int id = 0;
    unsigned int curTime = gettime();
	
	m_TimerLocker.lock();
	struct KernalTimerNode *pNode = m_Timers.head;
	while( pNode )
	{
		int expireTime = (int)( pNode->expireTime ) - (int)( curTime );
		if( expireTime > 0 )
		{
			pNode = pNode->next;
			continue;
		}
			
		pNode->expireTime = curTime + pNode->expire;
			
        if( pNode->time > 0 )
        {
            --pNode->time;
        }
			
		id = pNode->id;
			
        // 如果定时器执行次数为0则删除
       if( 0 == pNode->time )
       {		
			if( m_Timers.head == pNode )
			{
				m_Timers.head = pNode->next;
			}
			else
			{
				if( pNode->pre )
				{
					pNode->pre->next = pNode->next;
				}
				if( pNode->next )
				{
					pNode->next->pre = pNode->pre;
				}
			}
			delete pNode;
			pNode = NULL;
       }
	}
	m_TimerLocker.unlock();
	
	return id;
}

int KernalTimer::getMinTimerExpire()
{
    unsigned int curTime = gettime();
	int minExpire = -1;
	
	m_TimerLocker.lock();
	struct KernalTimerNode *pNode = m_Timers.head;
	while( pNode )
	{
		int expireTime = (int)( pNode->expireTime ) - (int)( curTime );
		if( expireTime < 0 )
		{
			expireTime = 0;
		}
			
		if( -1 == minExpire )
		{
			minExpire = expireTime;
		}
			
		if( minExpire > expireTime )
		{
			minExpire = expireTime;
		}
			
		pNode = pNode->next;
		if( 0 == minExpire )
		{
			break;
		}
	}
	m_TimerLocker.unlock();
	return minExpire;
}

