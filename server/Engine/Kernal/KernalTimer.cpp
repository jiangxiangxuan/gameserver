
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

	pthread_t tid = pthread_self();
	auto iter = m_Timers.find( tid );
	
	if( iter != m_Timers.end() )
	{
		iter->second.lock();
		pTimerNode->next = iter->second.head;
		if( iter->second.head )
		{
			iter->second.head->pre = pTimerNode;
		}
		iter->second.head = pTimerNode;
		iter->second.unlock();
	}
    return id;
}

void KernalTimer::delTimer( unsigned int id )
{
	for( auto iter = m_Timers.begin(); iter != m_Timers.end(); ++iter )
	{
		iter->second.lock();
		KernalTimerNode *pTimerNode = iter->second.head;
		while( pTimerNode )
		{
			if( pTimerNode->id == id )
			{
				if( iter->second.head == pTimerNode )
				{
					iter->second.head = pTimerNode->next;
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
		iter->second.unlock();
	}
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
	
	pthread_t tid = pthread_self();
	auto iter = m_Timers.find( tid );
	if( iter != m_Timers.end() )
	{
		iter->second.lock();
		struct KernalTimerNode *pNode = iter->second.head;
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
				if( iter->second.head == pNode )
				{
					iter->second.head = pNode->next;
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
			break;
		}
		iter->second.unlock();
	}
	
	return id;
}

int KernalTimer::getMinTimerExpire()
{
    unsigned int curTime = gettime();
	int minExpire = -1;
	
	pthread_t tid = pthread_self();
	auto iter = m_Timers.find( tid );
	if( iter != m_Timers.end() )
	{
		iter->second.lock();
		struct KernalTimerNode *pNode = iter->second.head;
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
		iter->second.unlock();
	}
	return minExpire;
}

void KernalTimer::initThreadTimer()
{
	m_TimerLocker.lock();
	pthread_t tid = pthread_self();
	auto iter = m_Timers.find( tid );
	if( iter == m_Timers.end() )
	{
		m_Timers.insert( std::make_pair( tid, KernalTimerNodeList() ) );
	}
	m_TimerLocker.unlock();
}
