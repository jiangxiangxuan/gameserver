
#include "GateWayServer.h"

GateWayServer *GateWayServer::ms_pGateWayServer = NULL;

void *GateWayServerConnectCenterWorker( void *arg )
{
	GateWayServer *pGateWayServer = (GateWayServer*)arg;
	pGateWayServer->registerCenterServerInfo();
	return ((void *)0);
}

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

	connectCenterServer();
}

void GateWayServer::onuninit()
{

}

void GateWayServer::handleTimerMsg( unsigned int id )
{

}

void GateWayServer::onMsg( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
	printf("GateWayServer::onMsg \r\n");
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
		sendMsgToClient(id,msg,strlen(msg));
		return;
#endif	
	
		struct KernalNetWork *pNetWork = m_Epoll.getNetWork( id );

		// 将消息转发给内部服务器
		if(  id != m_CenterServerID && !isInternalServer( id ) && ( KernalNetWorkType_CONNECTED == pNetWork->type || KernalNetWorkType_CONNECTED_HTTP == pNetWork->type ) )
		{
			int cmd = 0;
			memcpy( &cmd, data, 4 );
			if( KernalNetWorkType_CONNECTED == pNetWork->type && cmd >= m_CenterMinCmd && cmd < m_CenterMaxCmd )
			{
				sendMsgToCenter( id, type, data, size );
			}
			else if( KernalNetWorkType_CONNECTED == pNetWork->type && cmd >= m_PlatformMinCmd && cmd < m_PlatformMaxCmd )
			{
				sendMsgToPlatform( id, type, data, size );
			}
			else if( KernalNetWorkType_CONNECTED == pNetWork->type && cmd >= m_GameMinCmd )
			{
				sendMsgToGame( id, type, data, size );
			}
			else
			{
				char msg[128] = {0};
				memset(msg,0,sizeof(msg));
				sprintf(msg, "{\"state\":1,\"data\":\"error:netid=%d,datasize=%d\"}", id, size);
				sendMsgToClient(id,msg,strlen(msg));
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
	else if( NETWORK_CLOSE == type )
	{
		if( m_CenterServerID == id )
		{
			m_CenterServerID = -1;
			connectCenterServer();
		}
	}
}

void GateWayServer::sendMsgToClient( int id, char *data, int datalen )
{
	m_Epoll.send( id, data, datalen );
}

void GateWayServer::sendMsgToCenter( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
	if( -1 == m_CenterServerID )
	{
		return;
	}
	GateWayInternalServerMsg msg;
	msg.clientID = id;
	msg.initData( (char*)data, size );
	msg.type = ( NETWORK_DATA == type ) ? MESSAGE_DATA: MESSAGE_CONNECTCLOSE ;

	MsgSend( m_Epoll, m_CenterServerID, GateWayInternalServerMsg, 0, msg );
}

void GateWayServer::sendMsgToPlatform( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
	printf("GateWayServer::sendMsgToPlatform  %d %d %d\n\r",id,type,size);
	GateWayInternalServerMsg msg;
	msg.clientID = id;
	msg.initData( (char*)data, size );
	msg.type = ( NETWORK_DATA == type ) ? MESSAGE_DATA: MESSAGE_CONNECTCLOSE ;
	auto servers = m_Servers.equal_range( SERVER_PLATFORM );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			printf("GateWayServer::sendMsgToPlatform 111  %d %d %d %d\n\r",id,type,size,it->second->id );
			MsgSend( m_Epoll, it->second->id, GateWayInternalServerMsg, 0, msg );
			break;
		}
	}
}

void GateWayServer::sendMsgToGame( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
        GateWayInternalServerMsg msg;
        msg.clientID = id;
        msg.initData( (char*)data, size );
        msg.type = ( NETWORK_DATA == type ) ? MESSAGE_DATA: MESSAGE_CONNECTCLOSE ;

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

}

void GateWayServer::onExit()
{

}

void GateWayServer::connectCenterServer()
{
	KernalThread centerThread;
	centerThread.init(GateWayServerConnectCenterWorker, this);
	centerThread.detach();
}

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

	do
	{
		gettimeofday(&now, NULL);
		delay.tv_sec = now.tv_sec + 5;
		delay.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&cond, &mutex, &delay);

		m_CenterServerID = connect(serverIP, serverPort, false);
		//m_CenterServerID = m_Epoll.connectSocket( serverIP, serverPort );
		printf("GateWayServer::registerCenterServerInfo %d  %d  %d\r\n",m_CenterServerID,errno,EINPROGRESS);
		//for( int i = 0; i < 200000000; ++i );
		//pthread_delay_n( &delay );
		if( m_CenterServerID > 0 && EINPROGRESS == errno )
		{
			struct KernalNetWork *pNet = m_Epoll.getNetWork(m_CenterServerID);
		
			struct timeval tm = {5, 0};
			fd_set wset, rset;
			FD_ZERO( &wset );
			FD_ZERO( &rset );
			FD_SET( pNet->fd, &wset );
			FD_SET( pNet->fd, &rset );
			int res = select( pNet->fd + 1, &rset, &wset, NULL, &tm );
			printf("GateWayServer::registerCenterServerInfo 000 %d  %d \r\n",res,errno);
			if( res > 0 && FD_ISSET(pNet->fd, &wset)  )
			{
				int error, code;
				socklen_t len;
				len = sizeof(error);
				code = getsockopt(pNet->fd, SOL_SOCKET, SO_ERROR, &error, &len);
				printf("GateWayServer::registerCenterServerInfo 000-111 %d  %d \r\n",code,error);
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

	printf("GateWayServer::registerCenterServerInfo 111 %d\r\n",m_CenterServerID);
	MsgSend( m_Epoll, m_CenterServerID, CenterRegisterServerInfo, 0, msg );
	printf("GateWayServer::registerCenterServerInfo 222 %d\r\n",m_CenterServerID);

	pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );

}

void GateWayServer::handleCenterNotifyServerInfo( CenterNotifyServerInfo &value )
{
	printf("GateWayServer::handleCenterNotifyServerInfo %d %d\r\n",value.state,SERVERSTATE_RUN);
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
	printf("GateWayServer::connServer %s  %d\r\n",value.ip.c_str(), value.port);
	int id = m_Epoll.connect( value.ip.c_str(), value.port );
	if( id > 0 )
	{
		ServerInfo *pServer = new ServerInfo();
		pServer->type = value.type;
		memcpy( pServer->ip, value.ip.c_str(), value.ip.length() );
		pServer->port = value.port;
		pServer->id = id;

		m_Servers.insert( std::pair<ServerType, ServerInfo*>( value.type, pServer) );
	}
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
