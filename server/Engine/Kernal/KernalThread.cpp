
#include "KernalThread.h"

KernalThread::KernalThread()
	:m_tid( 0 )
{

}

KernalThread::~KernalThread()
{

}

bool KernalThread::init( kernalThreadFunc func, void *arg )
{
	return 0 == pthread_create( &m_tid, NULL, func, arg );
}

void KernalThread::detach()
{
	pthread_detach(  m_tid );
}

void KernalThread::join()
{
	pthread_join( m_tid, NULL );
}

