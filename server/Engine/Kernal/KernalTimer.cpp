
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
    //m_TimerLocker.lock();

    unsigned int id = getGuid();
	printf("KernalTimer::addTimer id=%d\n\r",id);
    KernalTimerNode *pTimerNode = new KernalTimerNode();
    pTimerNode->id     = id;
    pTimerNode->expire = expire;
    pTimerNode->time   = time;
    pTimerNode->expireTime = gettime() + pTimerNode->expire;
#if 0
    int slot = id % TIMER_SLOT;
    pTimerNode->next = m_TimerNodes[ slot ].head;
    if( m_TimerNodes[ slot ].head )
    {
        m_TimerNodes[ slot ].head->pre = pTimerNode;
    }
    m_TimerNodes[ slot ].head =  pTimerNode;
#endif
	pthread_t tid = pthread_self();
	auto iter = m_Timers.find( tid );
	if( iter == m_Timers.end() )
	{
		m_Timers.insert( std::make_pair( tid, KernalTimerNodeList() ) );
	}
	
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
	
	//m_TimerLocker.unlock();

    return id;
}

void KernalTimer::deleteTimer( unsigned int id )
{
    //m_TimerLocker.lock();
#if 0
    int slot = id % TIMER_SLOT;
    KernalTimerNode *pTimerNode = m_TimerNodes[ slot ].head;
    while( pTimerNode )
    {
        if( pTimerNode->id == id )
        {
            if( m_TimerNodes[ slot ].head == pTimerNode )
            {
                m_TimerNodes[ slot ].head = pTimerNode->next;
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
#endif
	for( auto iter = m_Timers.begin(); iter != m_Timers.end(); ++iter )
	{
		KernalTimerNode *pTimerNode = iter->second.head;
		while( pTimerNode )
		{
			if( pTimerNode->id == id )
			{
				iter->second.lock();
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
				iter->second.unlock();
				break;
			}
			pTimerNode = pTimerNode->next;
		}
	}
	//m_TimerLocker.unlock();
}

void KernalTimer::deleteTimer( struct KernalTimerNode *pTimerNode )
{
    //m_TimerLocker.lock();
#if 0
    int slot = pTimerNode->id % TIMER_SLOT;

    if( m_TimerNodes[ slot ].head == pTimerNode )
    {
        m_TimerNodes[ slot ].head = pTimerNode->next;
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
#endif

	for( auto iter = m_Timers.begin(); iter != m_Timers.end(); ++iter )
	{
		KernalTimerNode *pTimerN = iter->second.head;
		while( pTimerN )
		{
			if( pTimerN->id == pTimerNode->id )
			{
				//iter->second.lock();
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
				//iter->second.unlock();
				break;
			}
			pTimerNode = pTimerNode->next;
		}
	}
	//m_TimerLocker.unlock();
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

#if 0
void KernalTimer::update()
{
#if 0	
    ++m_CurrentSlot;
    if( TIMER_SLOT == m_CurrentSlot )
    {
        m_CurrentSlot = 0;
    }
    expired( m_CurrentSlot );
#endif
}

void KernalTimer::expired( int slot )
{
#if 0	
    unsigned int curTime = gettime();
    struct KernalTimerNode *pNode = m_TimerNodes[ slot ].head;
    while( pNode )
    {
        if( curTime >= pNode->expireTime )
        {
            pNode->expireTime = curTime + pNode->expire;
            push( pNode->id );

            if( pNode->time > 0 )
            {
                --pNode->time;
            }

            // 删除
            if( 0 == pNode->time )
            {
                deleteTimer( pNode );
            }
        }
        pNode = pNode->next;
    }
#endif	
}

void KernalTimer::push( unsigned int id )
{
#if 0	
    m_ExpriedTimerLocker.lock();

    m_expriedTimers.push_back( id );

    m_ExpriedTimerLocker.unlock();
#endif	
}

unsigned int KernalTimer::pop()
{
    unsigned int id = 0;
#if 0	
    m_ExpriedTimerLocker.lock();

    std::vector< unsigned int >::iterator iter = m_expriedTimers.begin();
    if( iter != m_expriedTimers.end() )
    {
        id = (*iter);
        m_expriedTimers.erase( iter );
    }

    m_ExpriedTimerLocker.unlock();
#endif
    return id;
}
#endif

unsigned int KernalTimer::popExpired()
{
    unsigned int id = 0;
    unsigned int curTime = gettime();
	
	//m_TimerLocker.lock();
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
			
			//struct KernalTimerNode *pTempNode = pNode->next;
            // 删除
            if( 0 == pNode->time )
            {
                deleteTimer( pNode );
            }
			//pNode = pTempNode;
			id = pNode->id;
			break;
		}
		iter->second.unlock();
	}
	
	//m_TimerLocker.unlock();
	return id;
}

int KernalTimer::getMinTimerExpire()
{
	//m_TimerLocker.lock();
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
			printf("KernalTimer::getMinTimerExpire expireTime=%ld curTime=%ld   %ld\r\n", pNode->expireTime, curTime, expireTime);
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
	//m_TimerLocker.unlock();
	return minExpire;
}

void KernalTimer::initThreadTimer()
{
	pthread_t tid = pthread_self();
	auto iter = m_Timers.find( tid );
	if( iter == m_Timers.end() )
	{
		m_Timers.insert( std::make_pair( tid, KernalTimerNodeList() ) );
	}
}
