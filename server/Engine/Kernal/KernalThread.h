
#ifndef _KERNAL_THREAD_H_
#define _KERNAL_THREAD_H_

#include <pthread.h>
#include <sys/time.h>

typedef void* (*kernalThreadFunc)( void *);

class KernalThread
{
public:
	KernalThread();
	~KernalThread();

	bool init( kernalThreadFunc func, void *arg );
	void detach();
private:
	pthread_t m_thread_t;	
};

#endif
