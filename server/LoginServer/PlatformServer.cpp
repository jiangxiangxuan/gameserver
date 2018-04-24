
#include "PlatformServer.h"

PlatformServer *PlatformServer::ms_pPlatformServer = NULL;

void *PlatformServerConnectCenterWorker( void *arg )
{
	PlatformServer *pPlatformServer = (PlatformServer*)arg;
	pPlatformServer->registerCenterServerInfo();
	return ((void *)0);
}

PlatformServer::PlatformServer()
	: m_pIdbcRedis( NULL ),
	  m_CenterServerID( -1 )
{

}

PlatformServer::~PlatformServer()
{

}

void PlatformServer::createInstance()
{
	ms_pPlatformServer = new PlatformServer();
}

void PlatformServer::destroyInstance()
{
	delete ms_pPlatformServer;
	ms_pPlatformServer = NULL;
}

PlatformServer *PlatformServer::getInstance()
{
	return ms_pPlatformServer;
}

void PlatformServer::oninit()
{
	m_PlatformLogin.setPlatformAddr( getConfig()->getAttributeStr("config/common/platform", "httpaddr") );
	
	m_threadNum = atoi(getConfig()->getText("config/login/thread"));

	const char *ip    = getConfig()->getAttributeStr("config/login/listen", "ip");
	int         port  = getConfig()->getAttributeInt("config/login/listen", "port");
	listen(ip, port);

	const char *httpIP   = getConfig()->getAttributeStr("config/login/http", "ip");
	int         httpPort = getConfig()->getAttributeInt("config/login/http", "port");
	listenHttp(httpIP, httpPort);

	m_pIdbcRedis = new IdbcRedisNonCluster();
	m_pIdbcRedis->loadLib();

	const char *redisip    = getConfig()->getAttributeStr("config/common/redis", "ip");
	int         redisport  = getConfig()->getAttributeInt("config/common/redis", "port");
	m_pIdbcRedis->connect( redisip, redisport );

	const char *dbaip    = getConfig()->getAttributeStr("config/dba/listen", "ip");
	int         dbaport  = getConfig()->getAttributeInt("config/dba/listen", "port");
	m_DBAgent.init( &m_Epoll, dbaip, dbaport );

	connectCenterServer();
}

void PlatformServer::onuninit()
{
	if( m_pIdbcRedis )
	{
		m_pIdbcRedis->unloadLib();
		delete m_pIdbcRedis;
		m_pIdbcRedis = NULL;
	}
}

void PlatformServer::handleTimerMsg( unsigned int id )
{
	//TODO:处理定时器消息
}

void PlatformServer::onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size )
{
	if( TIMER_DATA == type )
	{
		handleTimerMsg( id );
	}
	else if( NETWORK_DATA == type )
	{
		char *buff = (char*)data;
		DealStart( buff );
		DealMsg( id, GateWayInternalServerMsg, buff );
		DealMsg( id, DBServerAckMsg,           buff );
		DealMsg( id, PlatformGameServerMsg,    buff );
        DealMsg( id, CenterNotifyServerInfo,   buff );
		DealEnd();
	}
	else if( NETWORK_CLOSE == type )
	{
		if( m_CenterServerID == id )
		{
			m_CenterServerID = -1;
			connectCenterServer();
		}
		else if( !m_DBAgent.handleSocketClose( id ) )  // 如果不是DBA连接
		{

		}
	}
}

void PlatformServer::onProcess()
{

}

void PlatformServer::onRun()
{

}

void PlatformServer::onExit()
{
	m_pIdbcRedis->close();
}

void PlatformServer::handleGateWayMsg( int session, int clientID, char *data, int datalen )
{
	//TODO:处理客户端消息
    DealProtobufStart(data)
    DealProtobufMsg(&m_Epoll, m_pIdbcRedis, session, clientID, platformprotocol::PLATFORM_VERIFY_TOKEN, (&m_PlatformLogin), VerifyToken)
    DealProtobufEnd()
}

void PlatformServer::handleCenterNotifyServerInfo( CenterNotifyServerInfo &value )
{
	if( SERVERSTATE_RUN == value.state )
	{
		m_DBAgent.addDBAServer(value);
	}
	else
	{
		m_DBAgent.removeDBAServer(value);
	}
}

void PlatformServer::handleDBMsg( int session, int eventid, int error, char *data, int datalen )
{
	//TODO:处理DBA消息
	m_DBAgent.handleData( error, eventid, data, datalen );
}

void PlatformServer::handleGameMsg( int serverID, char *data, int datalen )
{
	//TODO:处理游戏消息
}

void PlatformServer::sendMsgToClient( int session, int clientID, char *data, int datalen )
{
	GateWayInternalServerMsg msg;
	msg.clientID = clientID;
	msg.type     = MESSAGE_DATA;
	msg.initData( data, datalen );
	MsgSend( m_Epoll, session, GateWayInternalServerMsg, 0, msg );
}

void PlatformServer::sendMsgToGame( int serverID, char *data, int datalen )
{
	PlatformGameServerMsg msg;
	msg.lineType = LINETYPE_PLATFORMTOGAME;
	msg.serverID = serverID;
	msg.initData( data, datalen );

	MsgSend( m_Epoll, m_CenterServerID, PlatformGameServerMsg, 0, msg );
}

int PlatformServer::executeDBSql( const char *sql, KernalObject *pObj, DBEvent_fn func )
{
	return m_DBAgent.execute( sql, pObj, func );
}

unsigned int PlatformServer::addTimer( unsigned int expire, int times )
{
	return m_Timer.addTimer( expire, times );
}

void PlatformServer::delTimer( unsigned int id )
{
	m_Timer.deleteTimer( id );
}

void PlatformServer::connectCenterServer()
{
    KernalThread centerThread;
	centerThread.init(PlatformServerConnectCenterWorker, this);
	centerThread.detach();
}

void PlatformServer::registerCenterServerInfo()
{
	const char *ip   = getConfig()->getAttributeStr("config/login/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/login/listen", "port");

	const char *serverIP   = getConfig()->getAttributeStr("config/center/listen", "ip");
	int         serverPort = getConfig()->getAttributeInt("config/center/listen", "port");

	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
    pthread_mutex_lock(&mutex);

	struct timespec delay;
	struct timeval now;
	int sfd = 0;
	do
	{
		gettimeofday(&now, NULL);
		delay.tv_sec = now.tv_sec + 2;
		delay.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&cond, &mutex, &delay);
		
		m_CenterServerID = connect(serverIP, serverPort, sfd, false);
        //m_CenterServerID = m_Epoll.connectSocket( serverIP, serverPort );

		//for( int i = 0; i < 200000000; ++i );
		//pthread_delay_n( &delay );

		if( m_CenterServerID > 0 && EINPROGRESS == errno )
		{
			//struct KernalNetWork *pNet = m_Epoll.getNetWork(m_CenterServerID);
		
			struct timeval tm = {2, 0};
			fd_set wset, rset;
			FD_ZERO( &wset );
			FD_ZERO( &rset );
			FD_SET( sfd, &wset );
			FD_SET( sfd, &rset );
			int res = select( sfd + 1, &rset, &wset, NULL, &tm );
			if( res > 0 && FD_ISSET(sfd, &wset)  )
			{
				gettimeofday(&now, NULL);
				delay.tv_sec = now.tv_sec + 5;
				delay.tv_nsec = now.tv_usec * 1000;
				pthread_cond_timedwait(&cond, &mutex, &delay);

				int error, code;
				socklen_t len;
				len = sizeof(error);
				code = getsockopt(sfd, SOL_SOCKET, SO_ERROR, &error, &len);
				if (code < 0 || error)
				{
					m_Epoll.close( m_CenterServerID );
                    //::close( m_CenterServerID );
				}
				else
				{
					break;
				}
			}
		}
		else if( m_CenterServerID > 0 )
		{
			m_Epoll.close( m_CenterServerID );
            //::close( m_CenterServerID );
		}
	}
	while( true );

    /*int size = 0;
    char _buf[BUFF_SIZE] = {0};
    char* dataBuf = _buf;
    NWriteInt32(dataBuf, &m_CenterServerID);
    NWriteInt32(dataBuf, &socket_connect);
    NWriteInt32(dataBuf, &size);
    dataBuf = _buf;
    m_Epoll.sendToPipe( dataBuf, size + 12 );*/

    gettimeofday(&now, NULL);
    delay.tv_sec = now.tv_sec + 5;
    delay.tv_nsec = now.tv_usec * 1000;
    pthread_cond_timedwait(&cond, &mutex, &delay);

	CenterRegisterServerInfo msg;
	msg.type = SERVER_PLATFORM;
	msg.ip   = ip;
	msg.port = port;

	MsgSend( m_Epoll, m_CenterServerID, CenterRegisterServerInfo, 0, msg );

    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );

}
