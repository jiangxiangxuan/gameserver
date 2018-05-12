﻿
#include "KernalServerBase.h"
#include "KernalGuid.h"

void *KernalServerBaseWorker( void *arg )
{
	KernalCommunicationPipe *pComPipe = (KernalCommunicationPipe*)arg;
	pComPipe->pServerBase->worker(pComPipe);
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
		// 创建线程通信管道
		KernalCommunicationPipe *pComPipe = new KernalCommunicationPipe();
		pComPipe->tid = pthread_self();
		pComPipe->pServerBase = this;
		int err = socketpair( AF_UNIX, SOCK_STREAM, 0, pComPipe->pipefd );  
		m_WorkThreadsPipe.push_back( pComPipe );
		KernalThread *pThread = new KernalThread();
		pThread->init(KernalServerBaseWorker, pComPipe);
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
				break;
			}
			case KernalSocketMessageType_SOCKET_CONNECT:// 连接 socket
			{
				pushMsg( NETWORK_CONNECT, result.netType, NULL, 0, result.id );
				break;
			}
			case KernalSocketMessageType_SOCKET_CLOSE: // 关闭连接
			{
				pushMsg( NETWORK_CLOSE, result.netType, NULL, 0, result.id );
				break;
			}
			default:
			{

			}
		}
	}
}

void KernalServerBase::worker(KernalCommunicationPipe *pComPipe)
{
	onWorkerPre();
	m_Timer.initThreadTimer();
	while( !m_quit )
	{		
		KernalMessage *pMsg = NULL;
		
		int minExpire = m_Timer.getMinTimerExpire(); // 获取最近过期时间的定时器的expire
		fd_set rset;
		FD_ZERO( &rset );
		FD_SET( pComPipe->pipefd[1], &rset );
		int retval = 0; //::select( pComPipe->pipefd[1] + 1, &rset, NULL, NULL, &tm );
		if( -1 == minExpire )
		{
			retval = ::select( pComPipe->pipefd[1] + 1, &rset, NULL, NULL, NULL );
		}
		else
		{
			struct timeval tm = {0, minExpire * 10000};
			retval = ::select( pComPipe->pipefd[1] + 1, &rset, NULL, NULL, &tm );
		}
		if( retval > 0 && FD_ISSET(pComPipe->pipefd[1], &rset)  )
		{
			// 处理网络消息
			pMsg = new KernalMessage();
			::read(pComPipe->pipefd[1], &pMsg->type, sizeof(pMsg->type));
			::read(pComPipe->pipefd[1], &pMsg->netType, sizeof(pMsg->netType));
			::read(pComPipe->pipefd[1], &pMsg->id, sizeof(pMsg->id));
			::read(pComPipe->pipefd[1], &pMsg->size, sizeof(pMsg->size));
			pMsg->data = ( char* )malloc( pMsg->size );
			memset( pMsg->data, 0, pMsg->size );
			::read(pComPipe->pipefd[1], pMsg->data, pMsg->size);

			// 处理消息
			switch( pMsg->type )
			{
				case NETWORK_DATA:
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
			
		// 处理定时器消息
		while( true )
		{
			unsigned int timeID = m_Timer.popExpired();
			if( timeID == 0 )
			{
				break;
			}
			onMsg(  timeID, KernalNetWorkType_NO, TIMER_DATA, NULL, 0 );
		}		
	}
	onWorkerEnd();
}

void KernalServerBase::heartbeatWorker()
{
#if 0	
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
#endif
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
