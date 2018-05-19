
#include "CenterServer.h"

CenterServer *CenterServer::ms_pCenterServer = NULL;

CenterServer::CenterServer()
{

}

CenterServer::~CenterServer()
{

}

void CenterServer::createInstance()
{
	ms_pCenterServer = new CenterServer();
}

void CenterServer::destroyInstance()
{
	delete ms_pCenterServer;
	ms_pCenterServer = NULL;
}

CenterServer *CenterServer::getInstance()
{
	return ms_pCenterServer;
}

void CenterServer::oninit()
{	
	m_Log.init( (char*)(getConfig()->getText("config/center/log")) );

	m_threadNum = atoi(getConfig()->getText("config/center/thread"));

	const char *ip    = getConfig()->getAttributeStr("config/center/listen", "ip");
	int         port  = getConfig()->getAttributeInt("config/center/listen", "port");
	listen(ip, port);

	const char *httpIP   = getConfig()->getAttributeStr("config/center/http", "ip");
	int         httpPort = getConfig()->getAttributeInt("config/center/http", "port");
	listenHttp(httpIP, httpPort);
}

void CenterServer::onuninit()
{

}

void CenterServer::handleTimerMsg( unsigned int id )
{
	printf("centerserver on handleTimerMsg tid=%ld   id=%ld\r\n",pthread_self(), id);
	
	// 删除定时任务
	m_Timer.delTimer( id );
}

void CenterServer::onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size )
{	
	if( TIMER_DATA == type )
	{
		handleTimerMsg( id );
	}
	else if( NETWORK_DATA == type )
	{
		// 中心服务器无网关 直接和客户端通信
		if( KernalNetWorkType_CONNECTED == netType )
		{
			char *buff = (char*)data;
			DealStart(buff);
			DealMsg( id, CenterRegisterServerInfo, buff );
			DealMsg( id, CenterUpdateServerInfo, buff );
			DealMsg( id, GateWayInternalServerMsg, buff );
			DealMsg( id, PlatformGameServerMsg, buff );
			DealEnd();
		}
		else if( KernalNetWorkType_CONNECTED_HTTP == netType )
		{
			// 测试定时任务
			unsigned int timeid = m_Timer.addTimer( 800, -1 );
			printf("centerserver on onMsg add timer tid=%ld   id=%ld\r\n",pthread_self(), timeid);
	
			char buff[50];
			memset( buff, 0, sizeof(buff) );

			auto servers = m_Servers.equal_range( SERVER_GATEWAY );
			for( auto iter = servers.first; iter != servers.second; ++iter )
			{
				sprintf( buff, "{\"state\":\"1\",\"ip\":\"%s\",\"port\":\"%d\"}",  iter->second->ip, iter->second->port );
				break;
			}
			if( strlen(buff) == 0 )
			{
				sprintf( buff, "{\"state\":\"0\"}" );
			}
			m_Epoll.send( id, buff, strlen(buff) );
		}
	}
	else if( NETWORK_CLOSE == type )
	{
		if( delInternalServer( id ) ) // 如果是内部服务器
		{
			
		}
	}
}

void CenterServer::onProcess()
{

}

void CenterServer::onRun()
{

}

void CenterServer::onExit()
{

}

void CenterServer::handleGateWayMsg( int session, int clientID, char *data, int datalen )
{
	//TODO:处理客户端消息
}

void CenterServer::handleRegisterServerInfo(int session, CenterRegisterServerInfo &value)
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

	ServerInfo *pServer = new ServerInfo();
	pServer->type = value.type;
	pServer->id   = session;
	memcpy( pServer->ip, value.ip.c_str(), value.ip.length() );
	pServer->port = value.port;

	m_Servers.insert( std::pair<ServerType, ServerInfo*>( value.type, pServer) );
	m_ServersLocker.unlock();

	// 通知相关服务器变化信息
	//notifyServerInfo( SERVER_GATEWAY, pServer->ip, pServer->port, value.type, SERVERSTATE_RUN );

	if( SERVER_PLATFORM == value.type || SERVER_GAME == value.type )
	{
		notifyServerInfo( SERVER_GATEWAY, pServer->ip, pServer->port, value.type, SERVERSTATE_RUN );
	}
	else if( SERVER_DBA == value.type )
	{
		notifyServerInfo( SERVER_PLATFORM, pServer->ip, pServer->port, value.type, SERVERSTATE_RUN );
		notifyServerInfo( SERVER_GAME, pServer->ip, pServer->port, value.type, SERVERSTATE_RUN );
	}

	// 如果是网关 则发送 大厅 和 游戏服务器信息
	if( SERVER_GATEWAY == value.type )
	{
		m_ServersLocker.lock();
		sendServerInfo( session, SERVER_PLATFORM, SERVERSTATE_RUN );
		sendServerInfo( session, SERVER_GAME, SERVERSTATE_RUN );
		m_ServersLocker.unlock();
	}
}

void CenterServer::handleCenterUpdateServerInfo(int session, CenterUpdateServerInfo &value)
{
	for( auto iter = m_Servers.begin(); iter != m_Servers.end(); ++iter )
	{
		if( iter->second->id == session )
		{
			iter->second->num = value.num;
			break;
		}
	}
}

void CenterServer::notifyServerInfo( ServerType type, char *ip, int port, ServerType serverType, ServerStateType state )
{
	if( type == serverType )
	{
		return;
	}
	m_ServersLocker.lock();
	auto servers = m_Servers.equal_range( type );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			sendServerInfo(it->second->id, serverType, state, ip, port );
		}
	}
	m_ServersLocker.unlock();
}

void CenterServer::sendServerInfo( int id, ServerType type, ServerStateType state )
{
	auto servers = m_Servers.equal_range( type );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			sendServerInfo(id, type, state, it->second->ip, it->second->port );
		}
	}
}

void CenterServer::sendServerInfo( int id, ServerType type, ServerStateType state, char *ip, int port )
{
	CenterNotifyServerInfo msg;
	msg.type  = type;
	msg.state = state;
	msg.ip.assign( ip );
	msg.port  = port;
	MsgSend( m_Epoll, id, CenterNotifyServerInfo, 0, msg );
}

void CenterServer::sendMsgToGameByPlatform(PlatformGameServerMsg &value)
{
	auto servers = m_Servers.equal_range( SERVER_GAME );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			MsgSend( m_Epoll, it->second->id, PlatformGameServerMsg, 0, value );
			break;
		}
	}
}

void CenterServer::sendMsgToPlatformByGame(PlatformGameServerMsg &value)
{
	auto servers = m_Servers.equal_range( SERVER_PLATFORM );
	for( auto it = servers.first; it != servers.second; ++it )
	{
		if( it->second )
		{
			MsgSend( m_Epoll, it->second->id, PlatformGameServerMsg, 0, value );
			break;
		}
	}
}

bool CenterServer::isInternalServer( unsigned int id )
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

bool CenterServer::delInternalServer( unsigned int id )
{
	m_ServersLocker.lock();
	bool isFind = false;
	ServerType serverType;
	char ip[50];
	memset( ip, 0, sizeof( ip ) );
	int port;
	for( auto iter = m_Servers.begin(); iter != m_Servers.end(); ++iter )
	{
		auto servers = m_Servers.equal_range( iter->first );
		for( auto it = servers.first; it != servers.second; ++it )
		{
			if( it->second && it->second->id == id )
			{
				serverType = it->second->type;
				memcpy( ip, it->second->ip, sizeof( it->second->ip ) );
				port = it->second->port;

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
	
	// 通知网关相关服务器变化信息
	if( isFind )
	{
		if( SERVER_PLATFORM == serverType || SERVER_GAME == serverType )
		{
			notifyServerInfo( SERVER_GATEWAY, ip, port, serverType, SERVERSTATE_CLOSE );
		}
		else if( SERVER_DBA == serverType )
		{
			notifyServerInfo( SERVER_PLATFORM, ip, port, serverType, SERVERSTATE_CLOSE );
			notifyServerInfo( SERVER_GAME, ip, port, serverType, SERVERSTATE_CLOSE );
		}
	}
	return isFind;
}
