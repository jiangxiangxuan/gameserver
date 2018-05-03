
#include "KernalServerBase.h"
#include "KernalGuid.h"

void *KernalServerBaseWorker( void *arg )
{
	KernalServerBase *pServerBase = (KernalServerBase*)arg;
	pServerBase->worker();
	return ((void *)0);
}

void *KernalServerBaseTimerWorker( void *arg )
{
	KernalServerBase *pServerBase = (KernalServerBase*)arg;
	pServerBase->timerWroker();
	return ((void *)0);
}

void *KernalServerBaseEpollWorker( void *arg )
{
	KernalServerBase *pServerBase = (KernalServerBase*)arg;
	pServerBase->epollWroker();
	return ((void *)0);
}

void *KernalServerBaseHeartBeatWorker( void *arg )
{
	KernalServerBase *pServerBase = (KernalServerBase*)arg;
	pServerBase->heartbeatWorker();
	return ((void *)0);
}

KernalServerBase::KernalServerBase()
	:m_quit( NULL ),
	 m_threadNum( 0 )
{

}

KernalServerBase::~KernalServerBase()
{

}

int KernalServerBase::listen( const char *ip, int port )
{
	return m_Epoll.listen( ip, port );
}

int KernalServerBase::connect( const char *ip, int port, int &sfd, bool addToEpoll )
{
	return m_Epoll.connect( ip, port, sfd );
}

int KernalServerBase::listenHttp( const char *ip, int port )
{
	return m_Epoll.listenHttp( ip, port );
}

int KernalServerBase::connectHttp( const char *ip, int port )
{
	return m_Epoll.connectHttp( ip, port );
}

void KernalServerBase::init( const char *configPath )
{
	m_Epoll.create();

	m_EpollThread.init(KernalServerBaseEpollWorker, this);
	m_EpollThread.detach();

	m_TimerThread.init(KernalServerBaseTimerWorker, this);
	m_TimerThread.detach();

	m_HeartBeatThread.init(KernalServerBaseHeartBeatWorker, this);
	m_HeartBeatThread.detach();

	m_Config.init( configPath );
	guidInit();
	oninit();

	if( m_threadNum <= 0 )
	{
		m_threadNum = 1;
	}
	for( int i = 0; i < m_threadNum; ++i )
	{
		KernalThread *pThread = new KernalThread();
		pThread->init(KernalServerBaseWorker, this);
		pThread->detach();
		m_WorkThreads.push( pThread );
	}
}

void KernalServerBase::uninit()
{
	m_Epoll.release();
}

void KernalServerBase::run()
{
	onRun();
	while( !m_quit )
	{
		onProcess();
		usleep(2000);
	}
	onExit();
}

void KernalServerBase::timerWroker()
{

	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_mutex_lock(&mutex);

	struct timespec delay;
	struct timeval now;

	while( !m_quit )
	{
		m_Timer.update();

		unsigned int id = 0;
		do
		{
			id = m_Timer.pop();
			if( 0 == id )
			{
				break;
			}
			pushMsg( TIMER_DATA, KernalNetWorkType_NO, NULL, 0, id );
			m_MessageCond.broadcast();
		}while( true );

		// 暂停
		//usleep( 100 );
		//for( int i = 0; i < 1000; ++i );

		gettimeofday(&now, NULL);
		delay.tv_sec = now.tv_sec;
		delay.tv_nsec = now.tv_usec * ( 1000000 / TIMER_SLOT );
		pthread_cond_timedwait(&cond, &mutex, &delay);

	}

	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy( &mutex );
	pthread_cond_destroy( &cond );

}

void KernalServerBase::epollWroker()
{
	KernalRequestMsg result;
	while( !m_quit )
	{
		KernalSocketMessageType type = m_Epoll.handleMessage( result );
		switch( type )
		{
			case KernalSocketMessageType_SOCKET_DATA: // 如果接收到数据则加入到队列中
			{
				pushMsg( NETWORK_DATA, result.netType, result.data, result.size, result.id );
				m_MessageCond.broadcast();
				break;
			}
			case KernalSocketMessageType_SOCKET_CLOSE: // 关闭连接
			{
				pushMsg( NETWORK_CLOSE, result.netType, NULL, 0, result.id );
				m_MessageCond.broadcast();
				break;
			}
			default:
			{

			}
		}
	}
}

void KernalServerBase::worker()
{
	onWorkerPre();
	while( !m_quit )
	{
		//if( !m_Messages.empty() )
		{
			KernalMessage *pMsg = m_Messages.pop();
			if( !pMsg )
			{
				m_MessageCond.wait();
				continue;
			}
			// 处理消息
			switch( pMsg->type )
			{
				case NETWORK_DATA:
				case TIMER_DATA:
				case NETWORK_CLOSE:
				{
					onMsg(  pMsg->id, pMsg->netType, pMsg->type, (const char *)pMsg->data, pMsg->size );
					break;
				}
				default:
				{
					break;
				}
			}

			delete pMsg;
			pMsg = NULL;
		}
		//else
		{
			//m_MessageCond.wait();
		}
	}
	onWorkerEnd();
}

void KernalServerBase::heartbeatWorker()
{

	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_mutex_lock(&mutex);

	struct timespec delay;
	struct timeval now;

	while( !m_quit )
	{
		m_Epoll.heartbeat();

		// 暂停
		//usleep( 10000000 );
		//for( int i = 0; i < 500000000; ++i );

		gettimeofday(&now, NULL);
		delay.tv_sec = now.tv_sec + 5;
		delay.tv_nsec = now.tv_usec;
		pthread_cond_timedwait(&cond, &mutex, &delay);

	}

	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy( &mutex );
	pthread_cond_destroy( &cond );

}

void KernalServerBase::quit()
{
	flush();

	m_quit = true;
}

void KernalServerBase::flush()
{

}

void KernalServerBase::pushMsg( KernalMessageType type, KernalNetWorkType netType, void *data, unsigned int size, unsigned int id )
{
	KernalMessage *pMsg = new KernalMessage();
	pMsg->type     = type;
	pMsg->netType  = netType;
	pMsg->data     = data;
	pMsg->size     = size;
	pMsg->id       = id;
	m_Messages.push( pMsg );
}

void KernalServerBase::sendMsg( int fd, char *buff, int len )
{
	m_Epoll.send( fd, buff, len );
}

void KernalServerBase::onWorkerPre()
{

}

void KernalServerBase::onWorkerEnd()
{

}
