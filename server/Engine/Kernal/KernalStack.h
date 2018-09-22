
#ifndef _KERNAL_STACK_H_
#define _KERNAL_STACK_H_

#include <stdio.h>
#include <stack>
#include "KernalLock.h"

template< typename T >
class KernalStack
{
public:
	KernalStack();
	virtual ~KernalStack();

	void push( T *pNode );
	bool empty();
	T *pop( );
private:
	std::stack<T*>       m_stack;
	KernalSpinLocker    m_locker;
};

template< typename T >
KernalStack< T >::KernalStack()
{

}

template< typename T >
KernalStack< T >::~KernalStack()
{

}

template< typename T >
void KernalStack< T >::push( T *pNode )
{
	m_locker.lock();

	m_stack.push( pNode );

	m_locker.unlock();
}

template< typename T >
bool KernalStack< T >::empty()
{
	return m_stack.empty();
}

template< typename T >
T *KernalStack< T >::pop( )
{
	T *t = NULL;
	m_locker.lock();
	if( !m_stack.empty() )
	{
		t = m_stack.top();
	}
	m_stack.pop();
	m_locker.unlock();
	return t;
}

#endif
