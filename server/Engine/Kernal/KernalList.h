
#ifndef _KERNAL_LIST_H_
#define _KERNAL_LIST_H_

#include <stdio.h>
#include "KernalLock.h"

template< typename T >
class KernalListNode
{
public:
	KernalListNode()
		:m_pPrev( NULL ),
		 m_pNext( NULL )
	{

	};
public:
	T* m_pPrev;
	T* m_pNext;
};

template< typename T >
class KernalList
{
public:
	KernalList();
	virtual ~KernalList();

	void push_back( T *pNode );
	void pop( T *pNode );
	T *pop();
	int  size();
	T *getHead();

protected:
	T *m_pHead;
	T *m_pEnd;
	int                  m_Size;
private:
	KernalMutexLocker     m_locker;
};

template< typename T >
KernalList< T >::KernalList()
	:m_pHead( NULL ),
	 m_pEnd( NULL ),
	 m_Size( 0 )
{

}

template< typename T >
KernalList< T >::~KernalList()
{

}

template< typename T >
void KernalList< T >::push_back( T *pNode )
{
	m_locker.lock();

	if( !m_pEnd )
	{
		pNode->m_pPrev = NULL;
		pNode->m_pNext  = NULL;

		m_pHead = pNode;
		m_pEnd  = pNode;
	}
	else
	{
		pNode->m_pPrev = m_pEnd;
		pNode->m_pNext = NULL;

		m_pEnd->m_pNext = pNode;

		m_pEnd = pNode;
	}

	++m_Size;

	m_locker.unlock();
}

template< typename T >
void KernalList< T >::pop( T *pNode )
{
	m_locker.lock();

	if( pNode->m_pPrev )
	{
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
	}
	else
	{
		m_pHead = pNode->m_pNext;
	}

	if( pNode->m_pNext )
	{
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
	}
	else
	{
		m_pEnd = pNode->m_pPrev;
	}

	--m_Size;

	m_locker.unlock();
}

template< typename T >
T *KernalList< T >::pop()
{
	T *pNode = NULL;
	m_locker.lock();
	if( m_pHead )
	{
		pNode = m_pHead;
		m_pHead = m_pHead->m_pNext;
	}
	if( !m_pHead )
	{
		m_pEnd = NULL;
	}

	// 如果链表存在node 则 size - 1
	if( pNode )
	{
		--m_Size;
	}

	m_locker.unlock();

	return pNode;
}

template< typename T >
int KernalList< T >::size( )
{
	return m_Size;
}

template< typename T >
T *KernalList< T >::getHead( )
{
	return m_pHead;
}

#endif
