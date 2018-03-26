
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
    m_TimerLocker.lock();

    unsigned int id = getGuid();

    KernalTimerNode *pTimerNode = new KernalTimerNode();
    pTimerNode->id     = id;
    pTimerNode->expire = expire;
    pTimerNode->time   = time;
    pTimerNode->expireTime = gettime() + pTimerNode->expire;

    int slot = id % TIMER_SLOT;
    pTimerNode->next = m_TimerNodes[ slot ].head;
    if( m_TimerNodes[ slot ].head )
    {
        m_TimerNodes[ slot ].head->pre = pTimerNode;
    }
    m_TimerNodes[ slot ].head =  pTimerNode;

	m_TimerLocker.unlock();

    return id;
}

void KernalTimer::deleteTimer( unsigned int id )
{
    m_TimerLocker.lock();

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
    }

	m_TimerLocker.unlock();
}

void KernalTimer::deleteTimer( struct KernalTimerNode *pTimerNode )
{
    m_TimerLocker.lock();

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

void KernalTimer::update()
{
    ++m_CurrentSlot;
    if( TIMER_SLOT == m_CurrentSlot )
    {
        m_CurrentSlot = 0;
    }
    expired( m_CurrentSlot );
}

void KernalTimer::expired( int slot )
{
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
}

void KernalTimer::push( unsigned int id )
{
    m_ExpriedTimerLocker.lock();

    m_expriedTimers.push_back( id );

    m_ExpriedTimerLocker.unlock();
}

unsigned int KernalTimer::pop()
{
    unsigned int id = 0;
    m_ExpriedTimerLocker.lock();

    std::vector< unsigned int >::iterator iter = m_expriedTimers.begin();
    if( iter != m_expriedTimers.end() )
    {
        id = (*iter);
        m_expriedTimers.erase( iter );
    }

    m_ExpriedTimerLocker.unlock();

    return id;
}
