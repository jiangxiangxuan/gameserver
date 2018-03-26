
#include "KernalLock.h"

/**
*信号量
*/
KernalSem::KernalSem()
{
    sem_init( &m_sem, 0, 0 );
}

KernalSem::~KernalSem()
{
    sem_destroy( &m_sem );
}

bool KernalSem::wait()
{
	return 0 == sem_wait( &m_sem );
}

bool KernalSem::post()
{
	return 0 == sem_post( &m_sem );
}

/**
*互斥锁
*/
KernalMutexLocker::KernalMutexLocker()
{
    pthread_mutex_init( &m_mutexLock, NULL );
}

KernalMutexLocker::~KernalMutexLocker()
{
    pthread_mutex_destroy( &m_mutexLock );
}

bool KernalMutexLocker::lock()
{
	return 0 == pthread_mutex_lock( &m_mutexLock );
}

bool KernalMutexLocker::unlock()
{
	return 0 == pthread_mutex_unlock( &m_mutexLock );
}
/**
*自旋锁
*/
KernalSpinLocker::KernalSpinLocker()
{
    pthread_spin_init( &m_spinLock, 0 );
}

KernalSpinLocker::~KernalSpinLocker()
{
    pthread_spin_destroy( &m_spinLock );
}

bool KernalSpinLocker::lock()
{
	return 0 == pthread_spin_lock( &m_spinLock );
}

bool KernalSpinLocker::unlock()
{
	return 0 == pthread_spin_unlock( &m_spinLock );
}

/**
*条件变量
*/
KernalCond::KernalCond()
{
    pthread_mutex_init( &m_mutexLock, NULL );
    pthread_cond_init( &m_cond, NULL );
}

KernalCond::~KernalCond()
{
    pthread_mutex_destroy( &m_mutexLock );
    pthread_cond_destroy( &m_cond );
}

bool KernalCond::wait()
{
	int ret = 0;
	pthread_mutex_lock( &m_mutexLock );
	ret = pthread_cond_wait( &m_cond, &m_mutexLock );
	pthread_mutex_unlock( &m_mutexLock );
	return 0 == ret;
}

bool KernalCond::signal()
{
	return 0 == pthread_cond_signal( &m_cond );
}

bool KernalCond::broadcast()
{
	return 0 == pthread_cond_broadcast( &m_cond );
}
