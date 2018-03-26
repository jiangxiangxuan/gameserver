
#ifndef _KERNAL_LOCKER_H
#define _KERNAL_LOCKER_H

#include <semaphore.h>
#include <pthread.h>

/**
*信号量
*/
class KernalSem
{
public:
	KernalSem();
	~KernalSem();

	bool wait();
	bool post();
private:
    sem_t m_sem;
};

/**
*互斥锁
*/
class KernalMutexLocker
{
public:
	KernalMutexLocker();
	~KernalMutexLocker();

	bool lock();
	bool unlock();
private:
	pthread_mutex_t m_mutexLock;
};

/**
*自旋锁
*/
class KernalSpinLocker
{
public:
	KernalSpinLocker();
	~KernalSpinLocker();

	bool lock();
	bool unlock();
private:
	pthread_spinlock_t m_spinLock;
};

/**
*条件变量
*/
class KernalCond
{
public:
	KernalCond();
	~KernalCond();

	bool wait();
	bool signal();
	bool broadcast();
private:
	pthread_mutex_t m_mutexLock;
	pthread_cond_t  m_cond;
};

#endif
