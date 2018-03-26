
#include "GameServer.h"
#include "Kernal/LuaKernalLock.h"

GameServer *GameServer::ms_pGameServer = NULL;

static int lua_sendClientMsg(lua_State *L)
{
    int session = lua_tonumber(L,1);
    int clientID = lua_tonumber(L,2);
    const char *data = lua_tostring(L,3);
    int datalen = lua_tonumber(L,4);
	GameServer::getInstance()->sendMsgToClient( session, clientID, (char*)data, datalen );
	return 0;
}

static int lua_sendPlatformMsg(lua_State *L)
{
    int serverID = lua_tonumber(L,1);
    const char *data = lua_tostring(L,2);
    int datalen = lua_tonumber(L,3);
	GameServer::getInstance()->sendMsgToPlatform( serverID, (char*)data, datalen );
	return 0;
}

static int lua_addTimer(lua_State *L)
{
    unsigned int expire = lua_tonumber(L,1);
    int times = lua_tonumber(L,2);
	unsigned int timerID = GameServer::getInstance()->addTimer( expire, times );
    lua_pushinteger( L, timerID );
	return 1;
}

static int lua_delTimer(lua_State *L)
{
    unsigned int timerID = lua_tonumber(L,1);
	GameServer::getInstance()->delTimer( timerID );
	return 0;
}

void *GameServerConnectCenterWorker( void *arg )
{
	GameServer *pGameServer = (GameServer*)arg;
	pGameServer->registerCenterServerInfo();
	return ((void *)0);
}

GameServer::GameServer()
	: m_pIdbcRedis( NULL ),
	  m_CenterServerID( 0 )
{

}

GameServer::~GameServer()
{

}

void GameServer::createInstance()
{
	ms_pGameServer = new GameServer();
}

void GameServer::destroyInstance()
{
	delete ms_pGameServer;
	ms_pGameServer = NULL;
}

GameServer *GameServer::getInstance()
{
	return ms_pGameServer;
}

void GameServer::oninit()
{
	m_threadNum = atoi(getConfig()->getText("config/game/thread"));

	const char *ip    = getConfig()->getAttributeStr("config/game/listen", "ip");
	int         port  = getConfig()->getAttributeInt("config/game/listen", "port");
	listen(ip, port);

	const char *httpIP   = getConfig()->getAttributeStr("config/game/http", "ip");
	int         httpPort = getConfig()->getAttributeInt("config/game/http", "port");
	listenHttp(httpIP, httpPort);

	m_pIdbcRedis = new IdbcRedisNonCluster();
	m_pIdbcRedis->loadLib();

	const char *redisip    = getConfig()->getAttributeStr("config/game/redis", "ip");
	int         redisport  = getConfig()->getAttributeInt("config/game/redis", "port");
	m_pIdbcRedis->connect( redisip, redisport );

	luaStateInit();

	connectCenterServer();
}

void GameServer::onuninit()
{
	if( m_pIdbcRedis )
	{
		m_pIdbcRedis->unloadLib();
		delete m_pIdbcRedis;
		m_pIdbcRedis = NULL;
	}
}

void GameServer::handleTimerMsg( unsigned int id )
{
	lua_getglobal( m_LuaState, "handleTimerMsg" );
	lua_pushinteger( m_LuaState, id );
	lua_pcall( m_LuaState, 1, 0, 0 );
}

void GameServer::onMsg( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
	m_LuaStateLocker.lock();
	if( TIMER_DATA == type )
	{
		handleTimerMsg( id );

		luaStateError();
	}
	else if( NETWORK_DATA == type )
	{
		char *buff = (char*)data;
		DealStart( buff );
		DealMsg( id, GateWayInternalServerMsg, buff );
		DealMsg( id, PlatformGameServerMsg,    buff );
        DealMsg( id, CenterNotifyServerInfo,   buff );
		DealEnd();

		luaStateError();
	}
	else if( NETWORK_CLOSE == type )
	{
		if( m_CenterServerID == id )
		{
			m_CenterServerID = -1;
			connectCenterServer();
		}
	}
	m_LuaStateLocker.unlock();
}

void GameServer::handleGateWayMsg( int session, int clientID, char *data, int datalen )
{
	//TODO:处理客户端消息

	lua_getglobal( m_LuaState, "handleMsg" );
	lua_pushinteger( m_LuaState, session );
    lua_pushinteger( m_LuaState, clientID );
	lua_pushstring( m_LuaState, data );
    lua_pushinteger( m_LuaState, datalen );
	lua_pcall( m_LuaState, 4, 0, 0 );

    //lua_pop( m_LuaState, 1 );
}

void GameServer::handleCenterNotifyServerInfo( CenterNotifyServerInfo &value )
{
	if( SERVERSTATE_RUN == value.state )
	{

	}
	else
	{

	}
}

void GameServer::handlePlatformMsg( int serverID, char *data, int datalen )
{
	//TODO:处理大厅消息

	lua_getglobal( m_LuaState, "handlePlatformMsg" );
    lua_pushinteger( m_LuaState, serverID );
	lua_pushstring( m_LuaState, data );
    lua_pushinteger( m_LuaState, datalen );
	lua_pcall( m_LuaState, 3, 0, 0 );
}

void GameServer::sendMsgToClient( int session, int clientID, char *data, int datalen )
{
	GateWayInternalServerMsg msg;
	msg.clientID = clientID;
	msg.type     = MESSAGE_DATA;
	msg.initData( data, datalen );
	MsgSend( m_Epoll, session, GateWayInternalServerMsg, 0, msg );
	//m_Epoll.send( id, data, datalen );
}

void GameServer::sendMsgToPlatform( int serverID, char *data, int datalen )
{
	PlatformGameServerMsg msg;
	msg.lineType = LINETYPE_GAMETOPLATFORM;
	msg.serverID = serverID;
	msg.initData( data, datalen );

	MsgSend( m_Epoll, m_CenterServerID, PlatformGameServerMsg, 0, msg );
}

void GameServer::onProcess()
{

}

void GameServer::onRun()
{

}

void GameServer::onExit()
{
	m_pIdbcRedis->close();
}

unsigned int GameServer::addTimer( unsigned int expire, int times )
{
	return m_Timer.addTimer( expire, times );
}

void GameServer::delTimer( unsigned int id )
{
	m_Timer.deleteTimer( id );
}

void GameServer::luaStateInit()
{
	m_LuaState = luaL_newstate();
    luaopen_base( m_LuaState );
	luaL_openlibs( m_LuaState );

    lua_pushcfunction( m_LuaState, lua_sendClientMsg );
    lua_setglobal( m_LuaState, "sendClientMsg" );

    lua_pushcfunction( m_LuaState, lua_sendPlatformMsg );
    lua_setglobal( m_LuaState, "sendPlatformMsg" );

    lua_pushcfunction( m_LuaState, lua_addTimer );
    lua_setglobal( m_LuaState, "addTimer" );

    lua_pushcfunction( m_LuaState, lua_delTimer );
    lua_setglobal( m_LuaState, "delTimer" );

    loadKernalMutexLocker( m_LuaState );

    lua_pop( m_LuaState, 1 );

	luaL_loadfile( m_LuaState, "src/main.lua");
	luaStateError();
	lua_pcall( m_LuaState, 0, 0, 0 );
	luaStateError();

}

void GameServer::connectCenterServer()
{
	KernalThread centerThread;
	centerThread.init(GameServerConnectCenterWorker, this);
	centerThread.detach();
}

void GameServer::registerCenterServerInfo()
{
	const char *ip   = getConfig()->getAttributeStr("config/game/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/game/listen", "port");

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
    WriteInt32(dataBuf, &m_CenterServerID);
    WriteInt32(dataBuf, &socket_connect);
    WriteInt32(dataBuf, &size);
    dataBuf = _buf;
    m_Epoll.sendToPipe( dataBuf, size + 12 );

    gettimeofday(&now, NULL);
    delay.tv_sec = now.tv_sec + 2;
    delay.tv_nsec = now.tv_usec * 1000;
    pthread_cond_timedwait(&cond, &mutex, &delay);

	CenterRegisterServerInfo msg;
	msg.type = SERVER_GAME;
	msg.ip   = ip;
	msg.port = port;

	MsgSend( m_Epoll, m_CenterServerID, CenterRegisterServerInfo, 0, msg );

    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );

}

void GameServer::luaStateError()
{
	const char* err = lua_tostring(m_LuaState, -1);
	if( err )
	{
		printf("Error: %s\n", err);
	}
}
