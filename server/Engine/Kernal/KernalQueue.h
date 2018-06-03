
#ifndef _KERNAL_QUEUE_H_
#define _KERNAL_QUEUE_H_

#include <stdio.h>
#include <queue>
#include <vector>
#include "KernalLock.h"

template< typename T >
class KernalQueue
{
public:
	KernalQueue();
	virtual ~KernalQueue();

	void push( T *pNode );
	bool empty();
	int size();
	T *front( );
	T *back( );
	T *pop();
private:
	std::queue<T*>      m_queue;
	KernalMutexLocker    m_locker;
};

template< typename T >
KernalQueue< T >::KernalQueue()
{

}

template< typename T >
KernalQueue< T >::~KernalQueue()
{

}

template< typename T >
void KernalQueue< T >::push( T *pNode )
{
	m_locker.lock();

	m_queue.push( pNode );

	m_locker.unlock();
}

template< typename T >
T *KernalQueue< T >::pop()
{
	if( empty() )
	{
		return NULL;
	}
	
	T *t = NULL;
	m_locker.lock();
	if( m_queue.size() )
	{
		t = m_queue.front();
		m_queue.pop();
	}
	m_locker.unlock();
	return t;
}

template< typename T >
bool KernalQueue< T >::empty()
{
	return 0 == m_queue.size();
}

template< typename T >
int KernalQueue< T >::size()
{
	return m_queue.size();
}

template< typename T >
T *KernalQueue< T >::front( )
{
	return pop();
}

#endif
