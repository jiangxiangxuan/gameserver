
#include "PlatformServer.h"
#include "Kernal/LuaKernalLock.h"

PlatformServer *PlatformServer::ms_pPlatformServer = NULL;

static int lua_sendClientMsg(lua_State *L)
{
    int session = lua_tonumber(L,1);
    int clientID = lua_tonumber(L,2);
    const char *data = lua_tostring(L,3);
    int datalen = lua_tonumber(L,4);
	PlatformServer::getInstance()->sendMsgToClient( session, clientID, (char*)data, datalen );
	return 0;
}

static int lua_sendGameMsg(lua_State *L)
{
    int serverID = lua_tonumber(L,1);
    const char *data = lua_tostring(L,2);
    int datalen = lua_tonumber(L,3);
	PlatformServer::getInstance()->sendMsgToGame( serverID, (char*)data, datalen );
	return 0;
}

static int lua_executeSql(lua_State *L)
{
    const char *sql = lua_tostring(L,1);
	int eventid = PlatformServer::getInstance()->executeDBSql( sql );
    lua_pushinteger( L, eventid );
	return 1;
}

static int lua_addTimer(lua_State *L)
{
    unsigned int expire = lua_tonumber(L,1);
    int times = lua_tonumber(L,2);
	unsigned int timerID = PlatformServer::getInstance()->addTimer( expire, times );
    lua_pushinteger( L, timerID );
	return 1;
}

static int lua_delTimer(lua_State *L)
{
    unsigned int timerID = lua_tonumber(L,1);
	PlatformServer::getInstance()->delTimer( timerID );
	return 0;
}

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

	luaStateInit();

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

	lua_close( m_LuaState );
}

void PlatformServer::handleTimerMsg( unsigned int id )
{
	lua_getglobal( m_LuaState, "handleTimerMsg" );
	lua_pushinteger( m_LuaState, id );
	lua_pcall( m_LuaState, 1, 0, 0 );
}

void PlatformServer::onMsg( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
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
		DealMsg( id, DBServerAckMsg,           buff );
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
		else if( !m_DBAgent.handleSocketClose( id ) )  // 如果不是DBA连接
		{

		}
	}
	m_LuaStateLocker.unlock();
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

	lua_getglobal( m_LuaState, "handleMsg" );
	lua_pushinteger( m_LuaState, session );
    lua_pushinteger( m_LuaState, clientID );
	lua_pushstring( m_LuaState, data );
    lua_pushinteger( m_LuaState, datalen );
	lua_pcall( m_LuaState, 4, 0, 0 );

    //lua_pop( m_LuaState, 1 );
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
	//m_DBAgent.handleData( error, eventid, data, datalen );

	if( datalen <= 8 )
	{
		return;
	}

	char *buff = data;
	int row = *((int*)(buff));
	buff += 4;
	int col = *((int*)(buff));
	buff += 4;

	lua_getglobal( m_LuaState, "handleDBAMsg" );
	lua_pushinteger( m_LuaState, error );
    lua_pushinteger( m_LuaState, eventid );
    lua_pushinteger( m_LuaState, row );
    lua_pushinteger( m_LuaState, col );
	lua_pushlightuserdata( m_LuaState, buff );
    lua_pushinteger( m_LuaState, datalen - 8 );

	lua_pcall( m_LuaState, 6, 0, 0 );

	m_DBAgent.releaseEvent( eventid );
    //lua_pop( m_LuaState, 1 );
}

void PlatformServer::handleGameMsg( int serverID, char *data, int datalen )
{
	//TODO:处理游戏消息

	lua_getglobal( m_LuaState, "handleGameMsg" );
    lua_pushinteger( m_LuaState, serverID );
	lua_pushstring( m_LuaState, data );
    lua_pushinteger( m_LuaState, datalen );
	lua_pcall( m_LuaState, 3, 0, 0 );
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

void PlatformServer::luaStateInit()
{
    m_LuaState = luaL_newstate();
    luaopen_base( m_LuaState );
    luaL_openlibs( m_LuaState );

    lua_pushcfunction( m_LuaState, lua_sendClientMsg );
    lua_setglobal( m_LuaState, "sendClientMsg" );

    lua_pushcfunction( m_LuaState, lua_sendGameMsg );
    lua_setglobal( m_LuaState, "sendGameMsg" );

    lua_pushcfunction( m_LuaState, lua_executeSql );
    lua_setglobal( m_LuaState, "executeSql" );

    lua_pushcfunction( m_LuaState, lua_addTimer );
    lua_setglobal( m_LuaState, "addTimer" );

    lua_pushcfunction( m_LuaState, lua_delTimer );
    lua_setglobal( m_LuaState, "delTimer" );

    loadKernalMutexLocker( m_LuaState );
    loadDBResult( m_LuaState );

    lua_pop( m_LuaState, 1 );

    luaL_loadfile( m_LuaState, "src/main.lua");
    lua_pcall( m_LuaState, 0, 0, 0 );

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

        //for( int i = 0; i < 200000000; ++i );
        //pthread_delay_n( &delay );

		m_CenterServerID = connect(serverIP, serverPort, false);
  	    //m_CenterServerID = m_Epoll.connectSocket( serverIP, serverPort );

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

    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );

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
	msg.type = SERVER_PLATFORM;
	msg.ip.assign( ip );
	msg.port = port;

	MsgSend( m_Epoll, m_CenterServerID, CenterRegisterServerInfo, 0, msg );
}

void PlatformServer::luaStateError()
{
	const char* err = lua_tostring(m_LuaState, -1);
	if( err )
	{
		printf("Error: %s\n", err);
	}
}
