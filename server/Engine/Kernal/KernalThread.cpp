
#include "KernalThread.h"

KernalThread::KernalThread()
	:m_thread_t( 0 )
{

}

KernalThread::~KernalThread()
{

}

bool KernalThread::init( kernalThreadFunc func, void *arg )
{
	return 0 == pthread_create( &m_thread_t, NULL, func, arg );
}

void KernalThread::detach()
{
	pthread_detach(  m_thread_t );
}
