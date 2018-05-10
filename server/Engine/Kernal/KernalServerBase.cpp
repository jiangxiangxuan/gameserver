
#include "KernalServerBase.h"
#include "KernalGuid.h"

void *KernalServerBaseWorker( void *arg )
{
#if defined(KERNAL_USE_COMMUNICATION_PIPE)
	KernalCommunicationPipe *pComPipe = (KernalCommunicationPipe*)arg;
	pComPipe->pServerBase->worker(pComPipe);
#else
	KernalServerBase *pServerBase = (KernalServerBase*)arg;
	pServerBase->worker();
#endif
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
	srand( (unsigned)time(NULL) );
	
	m_Epoll.create();

	m_EpollThread.init(KernalServerBaseEpollWorker, this);
	m_EpollThread.detach();

#if !defined(KERNAL_USE_COMMUNICATION_PIPE)
	m_TimerThread.init(KernalServerBaseTimerWorker, this);
	m_TimerThread.detach();
#endif

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
#if defined(KERNAL_USE_COMMUNICATION_PIPE)
		// 创建线程通信管道
		KernalCommunicationPipe *pComPipe = new KernalCommunicationPipe();
		pComPipe->tid = pthread_self();
		pComPipe->pServerBase = this;
		int err = socketpair( AF_UNIX, SOCK_STREAM, 0, pComPipe->pipefd );  
		m_WorkThreadsPipe.push_back( pComPipe );
#endif
		KernalThread *pThread = new KernalThread();
#if defined(KERNAL_USE_COMMUNICATION_PIPE)
		pThread->init(KernalServerBaseWorker, pComPipe);
#else	
		pThread->init(KernalServerBaseWorker, this);	
#endif
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
#if !defined(KERNAL_USE_COMMUNICATION_PIPE)
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
#if !defined(KERNAL_USE_COMMUNICATION_PIPE)
			//m_MessageCond.broadcast();
			m_MessageSem.post();
#endif
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
#endif
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
				//m_MessageCond.broadcast();
#if !defined(KERNAL_USE_COMMUNICATION_PIPE)
				m_MessageSem.post();
#endif				
				break;
			}
			case KernalSocketMessageType_SOCKET_CLOSE: // 关闭连接
			{
				pushMsg( NETWORK_CLOSE, result.netType, NULL, 0, result.id );
#if !defined(KERNAL_USE_COMMUNICATION_PIPE)
				//m_MessageCond.broadcast();
				m_MessageSem.post();
#endif				
				break;
			}
			default:
			{

			}
		}
	}
}

#if defined(KERNAL_USE_COMMUNICATION_PIPE)
void KernalServerBase::worker(KernalCommunicationPipe *pComPipe)
#else
void KernalServerBase::worker()
#endif
{
	onWorkerPre();
	while( !m_quit )
	{
		//if( !m_Messages.empty() )
		{
			KernalMessage *pMsg = NULL;
#if defined(KERNAL_USE_COMMUNICATION_PIPE)
			struct timeval tm = {0, 1000000};
			fd_set rset;
			FD_ZERO( &rset );
			FD_SET( pComPipe->pipefd[1], &rset );
			int retval = ::select( pComPipe->pipefd[1] + 1, &rset, NULL, NULL, &tm );
			if( retval > 0 && FD_ISSET(pComPipe->pipefd[1], &rset)  )
			{
				pMsg = new KernalMessage();
				::read(pComPipe->pipefd[1], &pMsg->type, sizeof(pMsg->type));
				::read(pComPipe->pipefd[1], &pMsg->netType, sizeof(pMsg->netType));
				::read(pComPipe->pipefd[1], &pMsg->id, sizeof(pMsg->id));
				::read(pComPipe->pipefd[1], &pMsg->size, sizeof(pMsg->size));
				pMsg->data = ( char* )malloc( pMsg->size );
				memset( pMsg->data, 0, pMsg->size );
				::read(pComPipe->pipefd[1], pMsg->data, pMsg->size);
#else
				pMsg = m_Messages.pop();
				if( !pMsg )
				{
					//m_MessageCond.wait();
					m_MessageSem.wait();
					continue;
				}
#endif
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
#if defined(KERNAL_USE_COMMUNICATION_PIPE)
			}
			
			// 处理定时器
			while( true )
			{
				unsigned int timeID = m_Timer.popExpired();
				if( timeID == 0 )
				{
					break;
				}
				pushMsg( TIMER_DATA, KernalNetWorkType_NO, NULL, 0, timeID );
			}
#endif
		}
		//else
		{
			//m_MessageCond.wait();
			//m_MessageSem.wait();
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
#if defined(KERNAL_USE_COMMUNICATION_PIPE)
	int index = rand()%m_WorkThreadsPipe.size();
	KernalCommunicationPipe *pComPipe = m_WorkThreadsPipe[index];
	if( pComPipe )
	{
		::write( pComPipe->pipefd[0], &type, sizeof(type) );
		::write( pComPipe->pipefd[0], &netType, sizeof(netType) );
		::write( pComPipe->pipefd[0], &id, sizeof(id) );
		::write( pComPipe->pipefd[0], &size, sizeof(size) );
		::write( pComPipe->pipefd[0], (char*)data, size );
	}
#else
	KernalMessage *pMsg = new KernalMessage();
	pMsg->type     = type;
	pMsg->netType  = netType;
	pMsg->data     = data;
	pMsg->size     = size;
	pMsg->id       = id;
	m_Messages.push( pMsg );	
#endif	
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
