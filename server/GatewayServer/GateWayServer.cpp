
#include "GateWayServer.h"

GateWayServer *GateWayServer::ms_pGateWayServer = NULL;

#if 0
void *GateWayServerConnectCenterWorker( void *arg )
{
	GateWayServer *pGateWayServer = (GateWayServer*)arg;
	pGateWayServer->registerCenterServerInfo();
	return ((void *)0);
}
#endif

GateWayServer::GateWayServer()
	: m_CenterServerID( -1 )
{

}

GateWayServer::~GateWayServer()
{

}

void GateWayServer::createInstance()
{
	ms_pGateWayServer = new GateWayServer();
}

void GateWayServer::destroyInstance()
{
	delete ms_pGateWayServer;
	ms_pGateWayServer = NULL;
}

GateWayServer *GateWayServer::getInstance()
{
	return ms_pGateWayServer;
}

void GateWayServer::oninit()
{
	m_threadNum = atoi(getConfig()->getText("config/gateway/thread"));

	const char *ip   = getConfig()->getAttributeStr("config/gateway/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/gateway/listen", "port");

	listen(ip, port);

#if 1
	const char *httpIP   = getConfig()->getAttributeStr("config/gateway/http", "ip");
	int         httpPort = getConfig()->getAttributeInt("config/gateway/http", "port");

	listenHttp(httpIP, httpPort);
#endif

	m_CenterMinCmd = getConfig()->getAttributeInt("config/gateway/centercmd", "min");
	m_CenterMaxCmd = getConfig()->getAttributeInt("config/gateway/centercmd", "max");

	m_PlatformMinCmd = getConfig()->getAttributeInt("config/gateway/platformcmd", "min");
	m_PlatformMaxCmd = getConfig()->getAttributeInt("config/gateway/platformcmd", "max");

	m_GameMinCmd = getConfig()->getAttributeInt("config/gateway/gamecmd", "min");
	m_GameMaxCmd = getConfig()->getAttributeInt("config/gateway/gamecmd", "max");

}

void GateWayServer::onuninit()
{

}

void GateWayServer::handleTimerMsg( unsigned int id )
{

}

void GateWayServer::onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size )
{
	printf("GateWayServer::onMsg cid=%ld  id=%ld\r\n",m_CenterServerID,id);
	if( TIMER_DATA == type )
	{
		handleTimerMsg( id );
	}
	else if( NETWORK_DATA == type )
	{
		
#if 0
		char msg[128] = {0};
		memset(msg,0,sizeof(msg));
		sprintf(msg, "{\"state\":1,\"data\":\"error:netid=%d,datasize=%d\"}", id, size);
		sendMsgToClient( id, 0, msg, strlen(msg) );
		return;
#endif	
	
		// 将消息转发给内部服务器
		if(  id != m_CenterServerID && !isInternalServer( id ) && ( KernalNetWorkType_CONNECTED == netType || KernalNetWorkType_CONNECTED_HTTP == netType ) )
		{
			int cmd = 0;
			memcpy( &cmd, data, 4 );
			if( KernalNetWorkType_CONNECTED == netType && cmd >= m_CenterMinCmd && cmd < m_CenterMaxCmd )
			{
				sendMsgToCenter( id, type, data, size );
			}
			else if( KernalNetWorkType_CONNECTED == netType && cmd >= m_PlatformMinCmd && cmd < m_PlatformMaxCmd )
			{
				sendMsgToPlatform( id, type, data, size );
			}
			else if( KernalNetWorkType_CONNECTED == netType && cmd >= m_GameMinCmd )
			{
				sendMsgToGame( id, type, data, size );
			}
			else
			{
				char msg[128] = {0};
				memset(msg,0,sizeof(msg));
				sprintf(msg, "{\"state\":1,\"data\":\"error:netid=%d,datasize=%d\"}", id, size);
				sendMsgToClient( id, 0, msg, strlen(msg) );
			}
		}
		else if( data ) // 将消息转发给客户端
		{
			char *buff = (char*)data;
			DealStart(buff);
			DealMsg(id, GateWayInternalServerMsg, buff);
			DealMsg(id, CenterNotifyServerInfo,   buff);
			DealEnd();
		}
	}
	else if( NETWORK_CONNECT == type )
	{
		printf("GateWayServer::onMsg NETWORK_CONNECT cid=%ld  id=%ld\r\n",m_CenterServerID,id);
		if( m_CenterServerID == id )
		{
			const char *ip   = getConfig()->getAttributeStr("config/gateway/listen", "ip");
			int         port = getConfig()->getAttributeInt("config/gateway/listen", "port");

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
		offlineByClientID( id );
	}
}

void GateWayServer::sendMsgToClient( int id, int uid, char *data, int datalen )
{
	// 如果是登录验证
	int cmd = 0;
	memcpy( &cmd, data, 4 );
	if( 2000 == cmd && 0 != uid )
	{
		m_Players.insert( std::pair< int, int >( id, uid ) );
	}
	m_Epoll.send( id, data, datalen );
}

void GateWayServer::sendMsgToCenter( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
	if( -1 == m_CenterServerID )
	{
		return;
	}
	GateWayInternalServerMsg msg;
	msg.UID      = getUidByClientID( id );
	msg.clientID = id;
	msg.type     = ( NETWORK_DATA == type ) ? MESSAGE_DATA: MESSAGE_CONNECTCLOSE ;
	msg.initData( (char*)data, size );

	MsgSend( m_Epoll, m_CenterServerID, GateWayInternalServerMsg, 0, msg );
}

void GateWayServer::sendMsgToPlatform( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
	GateWayInternalServerMsg msg;
	msg.clientID = id;
	msg.type     = ( NETWORK_DATA == type ) ? MESSAGE_DATA: MESSAGE_CONNECTCLOSE ;
	msg.initData( (char*)data, size );
	
	auto servers = m_Servers.equal_range( SERVER_PLATFORM );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			MsgSend( m_Epoll, it->second->id, GateWayInternalServerMsg, 0, msg );
			break;
		}
	}
}

void GateWayServer::sendMsgToGame( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
    GateWayInternalServerMsg msg;
	msg.UID      = getUidByClientID( id );
    msg.clientID = id;
	msg.type     = ( NETWORK_DATA == type ) ? MESSAGE_DATA: MESSAGE_CONNECTCLOSE ;
	msg.initData( (char*)data, size );

	auto servers = m_Servers.equal_range( SERVER_GAME );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			MsgSend( m_Epoll, it->second->id, GateWayInternalServerMsg, 0, msg );
			break;
		}
	}
}

void GateWayServer::onProcess()
{

}

void GateWayServer::onRun()
{
	connectCenterServer();
}

void GateWayServer::onExit()
{

}

void GateWayServer::connectCenterServer()
{
#if 0	
	KernalThread centerThread;
	centerThread.init(GateWayServerConnectCenterWorker, this);
	centerThread.detach();
#endif
	const char *serverIP   = getConfig()->getAttributeStr("config/center/listen", "ip");
	int         serverPort = getConfig()->getAttributeInt("config/center/listen", "port");

	int sfd = 0;
	m_CenterServerID = connect(serverIP, serverPort, sfd, false);
	printf("GateWayServer::connectCenterServer cid = %ld  fd=%ld\r\n", m_CenterServerID, sfd);
}

#if 0
void GateWayServer::registerCenterServerInfo()
{
	const char *ip   = getConfig()->getAttributeStr("config/gateway/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/gateway/listen", "port");

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
		delay.tv_sec = now.tv_sec + 5;
		delay.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&cond, &mutex, &delay);

		m_CenterServerID = connect(serverIP, serverPort, sfd, false);
		//m_CenterServerID = m_Epoll.connectSocket( serverIP, serverPort );
		//for( int i = 0; i < 200000000; ++i );
		//pthread_delay_n( &delay );
		if( m_CenterServerID > 0 && EINPROGRESS == errno )
		{
			//struct KernalNetWork *pNet = m_Epoll.getNetWork(m_CenterServerID);
		
			struct timeval tm = {10, 0};
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
	msg.type = SERVER_GATEWAY;
	msg.ip   = ip;
	msg.port = port;

	MsgSend( m_Epoll, m_CenterServerID, CenterRegisterServerInfo, 0, msg );

	pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );

}
#endif

void GateWayServer::handleCenterNotifyServerInfo( CenterNotifyServerInfo &value )
{
	if( SERVERSTATE_RUN == value.state )
	{
		connServer( value );
	}
	else
	{
		closeServer( value );
	}
}

void GateWayServer::closeServer( int id )
{
	bool isFind = false;
	for( auto iter = m_Servers.begin(); iter != m_Servers.end(); ++iter )
	{
		auto servers = m_Servers.equal_range( iter->first );
		for( auto it = servers.first; it != servers.second; ++it )
		{
			if( it->second && it->second->id == id )
			{
				delete it->second;
				it = m_Servers.erase( it );
				isFind = true;
				break;
			}
		}
		if( isFind )
		{
			break;
		}
	}
}

void GateWayServer::closeServer( CenterNotifyServerInfo &value )
{
	auto servers = m_Servers.equal_range( value.type );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second && 0 == strcmp(it->second->ip, value.ip.c_str()) && it->second->port == value.port )
		{
			m_Servers.erase( it );
			break;
		}
	}
}

void GateWayServer::connServer( CenterNotifyServerInfo &value )
{
	bool isFind = false;
	auto servers = m_Servers.equal_range( value.type );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second && it->second->port == value.port && 0 == strcmp(it->second->ip, value.ip.c_str()) )
		{
			isFind = true;
			break;
		}
	}
	if( isFind )
	{
		return;
	}
	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
    pthread_mutex_lock(&mutex);

	struct timespec delay;
	struct timeval now;

	gettimeofday(&now, NULL);
	delay.tv_sec = now.tv_sec + 5;
	delay.tv_nsec = now.tv_usec * 1000;
	pthread_cond_timedwait(&cond, &mutex, &delay);

	int sfd = 0;
	int id = connect( value.ip.c_str(), value.port, sfd );
	
	gettimeofday(&now, NULL);
	delay.tv_sec = now.tv_sec + 5;
	delay.tv_nsec = now.tv_usec * 1000;
	pthread_cond_timedwait(&cond, &mutex, &delay);

	if( id > 0 )
	{
		ServerInfo *pServer = new ServerInfo();
		pServer->type = value.type;
		memcpy( pServer->ip, value.ip.c_str(), value.ip.length() );
		pServer->port = value.port;
		pServer->id = id;

		m_Servers.insert( std::pair<ServerType, ServerInfo*>( value.type, pServer) );
	}
	
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );

}

bool GateWayServer::isInternalServer( unsigned int id )
{
	bool isFind = false;
	for( auto iter = m_Servers.begin(); iter != m_Servers.end(); ++iter )
	{
		auto servers = m_Servers.equal_range( iter->first );
		for( auto it = servers.first; it != servers.second; ++it )
		{
			if( it->second && it->second->id == id )
			{
				isFind = true;
				break;
			}
		}
		if( isFind )
		{
			break;
		}
	}

	return isFind;
}

int GateWayServer::getUidByClientID( int id )
{
	std::map< int, int >::iterator iter = m_Players.find( id );
	if( iter != m_Players.end() )
	{
		return iter->second;
	}
	return 0;
}

void GateWayServer::offlineByClientID( int id )
{
	std::map< int, int >::iterator iter = m_Players.find( id );
	if( iter != m_Players.end() )
	{
		m_Players.erase( iter );  
	}
}
