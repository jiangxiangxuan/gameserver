
#include "GateWayServer.h"

GateWayServer *GateWayServer::ms_pGateWayServer = NULL;

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
	
		// 将客户端的消息转发给内部服务器
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
		else if( data ) // 将内部服务器的消息转发给客户端
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
		// 如果连接的是中心服务器
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
		if( m_CenterServerID == id ) // 如果是中心服务器
		{
			m_CenterServerID = -1;
			connectCenterServer();
		}
		else if( closeInternalServer( id ) ) // 如果是内部服务器
		{
			
		}
		else
		{
			offlineByClientID( id );
		}
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
	
	// TODO:待优化
	m_ServersLocker.lock();
	auto servers = m_Servers.equal_range( SERVER_PLATFORM );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			MsgSend( m_Epoll, it->second->id, GateWayInternalServerMsg, 0, msg );
			break;
		}
	}
	m_ServersLocker.unlock();
}

void GateWayServer::sendMsgToGame( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
    GateWayInternalServerMsg msg;
	msg.UID      = getUidByClientID( id );
    msg.clientID = id;
	msg.type     = ( NETWORK_DATA == type ) ? MESSAGE_DATA: MESSAGE_CONNECTCLOSE ;
	msg.initData( (char*)data, size );
	
	// TODO:待优化
	m_ServersLocker.lock();
	auto servers = m_Servers.equal_range( SERVER_GAME );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			MsgSend( m_Epoll, it->second->id, GateWayInternalServerMsg, 0, msg );
			break;
		}
	}
	m_ServersLocker.unlock();
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
	const char *serverIP   = getConfig()->getAttributeStr("config/center/listen", "ip");
	int         serverPort = getConfig()->getAttributeInt("config/center/listen", "port");

	int sfd = 0;
	m_CenterServerID = connect(serverIP, serverPort, sfd, false);
}

void GateWayServer::handleCenterNotifyServerInfo( CenterNotifyServerInfo &value )
{
	if( SERVERSTATE_RUN == value.state )
	{
		connInternalServer( value );
	}
	else
	{
		closeInternalServer( value );
	}
}

void GateWayServer::closeInternalServer( CenterNotifyServerInfo &value )
{
	m_ServersLocker.lock();
	auto servers = m_Servers.equal_range( value.type );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second && 0 == strcmp(it->second->ip, value.ip.c_str()) && it->second->port == value.port )
		{
			delete it->second;
			it = m_Servers.erase( it );
			m_Servers.erase( it );
			break;
		}
	}
	m_ServersLocker.unlock();
}

void GateWayServer::connInternalServer( CenterNotifyServerInfo &value )
{
	m_ServersLocker.lock();
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
		m_ServersLocker.unlock();
		return;
	}
	
	int sfd = 0;
	int id = connect( value.ip.c_str(), value.port, sfd );
	
	if( id > 0 )
	{
		ServerInfo *pServer = new ServerInfo();
		pServer->type = value.type;
		memcpy( pServer->ip, value.ip.c_str(), value.ip.length() );
		pServer->port = value.port;
		pServer->id = id;

		m_Servers.insert( std::pair<ServerType, ServerInfo*>( value.type, pServer) );
	}
	m_ServersLocker.unlock();
}

bool GateWayServer::isInternalServer( unsigned int id )
{
	m_ServersLocker.lock();
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
	m_ServersLocker.unlock();
	return isFind;
}

bool GateWayServer::closeInternalServer( unsigned int id )
{
	m_ServersLocker.lock();
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
	m_ServersLocker.unlock();
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
