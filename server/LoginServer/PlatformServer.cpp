
#include "PlatformServer.h"

PlatformServer *PlatformServer::ms_pPlatformServer = NULL;

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
	m_DBAgent.init( &m_Epoll );
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
	// 如果是连接中心服务器定时任务
	if( m_ConnCenterTimeID == id )
	{
		connectCenterServer();
	}	
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
	else if( NETWORK_CONNECT == type )
	{
		// 如果连接的是中心服务器
		if( m_CenterServerID == id )
		{
			const char *ip   = getConfig()->getAttributeStr("config/login/listen", "ip");
			int         port = getConfig()->getAttributeInt("config/login/listen", "port");

			CenterRegisterServerInfo msg;
			msg.type = SERVER_GATEWAY;
			msg.ip   = ip;
			msg.port = port;
			MsgSend( m_Epoll, m_CenterServerID, CenterRegisterServerInfo, 0, msg );
		}
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
	connectCenterServer();
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
	m_Timer.delTimer( id );
}

void PlatformServer::connectCenterServer()
{
	const char *serverIP   = getConfig()->getAttributeStr("config/center/listen", "ip");
	int         serverPort = getConfig()->getAttributeInt("config/center/listen", "port");

	int sfd = 0;
	m_CenterServerID = connect(serverIP, serverPort, sfd, true);
	
	// 如果没有连接上中心服务器 则定时任务连接
	if( -1 == m_CenterServerID )
	{
		m_ConnCenterTimeID = m_Timer.addTimer( 500, 1 );
	}
	else
	{
		m_ConnCenterTimeID = 0;
	}
}
