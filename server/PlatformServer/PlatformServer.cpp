
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
	m_threadNum = atoi(getConfig()->getText("config/platform/thread"));

	const char *ip    = getConfig()->getAttributeStr("config/platform/listen", "ip");
	int         port  = getConfig()->getAttributeInt("config/platform/listen", "port");
	listen(ip, port);

	const char *httpIP   = getConfig()->getAttributeStr("config/platform/http", "ip");
	int         httpPort = getConfig()->getAttributeInt("config/platform/http", "port");
	listenHttp(httpIP, httpPort);

	m_pIdbcRedis = new IdbcRedisNonCluster();
	m_pIdbcRedis->loadLib();

	const char *redisip    = getConfig()->getAttributeStr("config/platform/redis", "ip");
	int         redisport  = getConfig()->getAttributeInt("config/platform/redis", "port");
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

void PlatformServer::onMsg( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
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

void PlatformServer::VerifyToken(int session, int clientid, char *data, int msglen)
{
    platformprotocol::CVerifyToken verifyToken;
    ParseFromArrayToObj(verifyToken, data, msglen);
	
	printf("platform VerifyToken token=%s\r\n",verifyToken.token().c_str());
	
	platformprotocol::CVerifyToken verifyToken111;
	verifyToken111.set_token("Token Test 111111");
	
	ProtobufMsgSendToClientByGateWay(m_Epoll,session,clientid,2000,0,verifyToken111);
		
}

void PlatformServer::handleGateWayMsg( int session, int clientID, char *data, int datalen )
{
	//TODO:处理客户端消息
    DealProtobufStart(data)
    DealProtobufMsg(session, clientID, 2000, VerifyToken)
    DealProtobufEnd()
    return;
	int cmd = 0;
	NReadInt32(data,&cmd);
	printf("platform handleGateWayMsg cmd=%d\r\n",cmd);
	int err = 0;
	NReadInt32(data,&err);
	printf("platform handleGateWayMsg err=%d\r\n",err);
	int len = 0;
	NReadInt32(data,&len);
	printf("platform handleGateWayMsg len=%d\r\n",len);
	
	platformprotocol::CVerifyToken verifyToken;
	ParseFromArrayToObj( verifyToken, data, len );
	printf("platform handleGateWayMsg token=%s\r\n",verifyToken.token().c_str());
	
	
	platformprotocol::CVerifyToken verifyToken111;
	verifyToken111.set_token("Token Test 111111");
	
	ProtobufMsgSendToClientByGateWay(m_Epoll,session,clientID,cmd,err,verifyToken111);
		
	/*int pcmd = cmd;                     
	int perr = err;                     
	int len1 = verifyToken111.ByteSize();           
	char *pdata = new char[len1];        
	verifyToken111.SerializeToArray(pdata, len1);   
	char *_buff = new char[len1+12];      
	char *databuff = _buff;             
	NWriteInt32(databuff,&pcmd);        
	NWriteInt32(databuff,&perr);        
	NWriteInt32(databuff,&len1);         
	NWriteBit(databuff,pdata,len1);
	
	sendMsgToClient(session,clientID,_buff,len1+12);
	
	delete []pdata; 
	delete []_buff; 	*/
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
	const char *ip   = getConfig()->getAttributeStr("config/platform/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/platform/listen", "port");

	const char *serverIP   = getConfig()->getAttributeStr("config/center/listen", "ip");
	int         serverPort = getConfig()->getAttributeInt("config/center/listen", "port");

	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
    pthread_mutex_lock(&mutex);

	struct timespec delay;
	struct timeval now;

	do
	{
    gettimeofday(&now, NULL);
    delay.tv_sec = now.tv_sec + 2;
    delay.tv_nsec = now.tv_usec * 1000;
    pthread_cond_timedwait(&cond, &mutex, &delay);

		m_CenterServerID = connect(serverIP, serverPort, false);
        //m_CenterServerID = m_Epoll.connectSocket( serverIP, serverPort );

		//for( int i = 0; i < 200000000; ++i );
		//pthread_delay_n( &delay );

		if( m_CenterServerID > 0 && EINPROGRESS == errno )
		{
			struct timeval tm = {2, 0};
			fd_set wset, rset;
			FD_ZERO( &wset );
			FD_ZERO( &rset );
			FD_SET( m_CenterServerID, &wset );
			FD_SET( m_CenterServerID, &rset );
			int res = select( m_CenterServerID + 1, &rset, &wset, NULL, &tm );
			if( res > 0 && FD_ISSET(m_CenterServerID, &wset)  )
			{
				int error, code;
				socklen_t len;
				len = sizeof(error);
				code = getsockopt(m_CenterServerID, SOL_SOCKET, SO_ERROR, &error, &len);
				if (code < 0 || error)
				{
					//m_Epoll.close( m_CenterServerID );
                    ::close( m_CenterServerID );
				}
				else
				{
					break;
				}
			}
		}
		else if( m_CenterServerID > 0 )
		{
			//m_Epoll.close( m_CenterServerID );
            ::close( m_CenterServerID );
		}
	}
	while( true );

    int size = 0;
    char _buf[BUFF_SIZE] = {0};
    char* dataBuf = _buf;
    NWriteInt32(dataBuf, &m_CenterServerID);
    NWriteInt32(dataBuf, &socket_connect);
    NWriteInt32(dataBuf, &size);
    dataBuf = _buf;
    m_Epoll.sendToPipe( dataBuf, size + 12 );

    gettimeofday(&now, NULL);
    delay.tv_sec = now.tv_sec + 2;
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
