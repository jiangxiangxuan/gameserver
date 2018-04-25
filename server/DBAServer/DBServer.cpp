
#include "DBServer.h"

DBServer *DBServer::ms_pDBServer = NULL;

void *DBServerConnectCenterWorker( void *arg )
{
	DBServer *pDBServer = (DBServer*)arg;
	pDBServer->registerCenterServerInfo();
	return ((void *)0);
}

DBServer::DBServer()
{

}

DBServer::~DBServer()
{

}

void DBServer::createInstance()
{
	ms_pDBServer = new DBServer();
}

void DBServer::destroyInstance()
{
	delete ms_pDBServer;
	ms_pDBServer = NULL;
}

DBServer *DBServer::getInstance()
{
	return ms_pDBServer;
}

void DBServer::oninit()
{
	m_threadNum = atoi(getConfig()->getText("config/dba/thread"));

	const char *ip   = getConfig()->getAttributeStr("config/dba/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/dba/listen", "port");
	listen(ip, port);

	connectCenterServer();
}

void DBServer::onWorkerPre()
{
	DataBase *pDataBase = new DataBase();
	pDataBase->loadLib();

	// 连接数据库
	const char *dbip   = getConfig()->getAttributeStr("config/dba/db", "ip");
	int         dbport = getConfig()->getAttributeInt("config/dba/db", "port");
	const char *dbuser   = getConfig()->getAttributeStr("config/dba/db", "user");
	const char *dbpwd    = getConfig()->getAttributeStr("config/dba/db", "pwd");
	const char *dbname = getConfig()->getAttributeStr("config/dba/db", "dbname");
	pDataBase->connect(dbip, dbport, dbuser, dbpwd, dbname);

	m_DataBases.insert( pthread_self(), pDataBase );
}

void DBServer::onWorkerEnd()
{
	DataBase *pDataBase = m_DataBases.find( pthread_self() );
	if( pDataBase )
	{
		pDataBase->close();
		pDataBase->unloadLib();
	}
	m_DataBases.erase( pthread_self() );
}

void DBServer::onuninit()
{

}

void DBServer::handleTimerMsg( unsigned int id )
{

}

void DBServer::onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size )
{
	if( TIMER_DATA == type )
	{
		handleTimerMsg( id );
	}
	else if( NETWORK_DATA == type )
	{
		char *buff = (char*)data;
		DealStart(buff);
		DealMsg( id, DBServerReqMsg, buff );
		DealEnd();
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

void DBServer::onProcess()
{

}

void DBServer::onRun()
{

}

void DBServer::onExit()
{

}

void DBServer::execute( int session, DBServerReqMsg &value )
{
	DataBase * pDataBase = getDataBase();
	pDataBase->query( value.sqlStr.c_str() );
	pDataBase->store();
	int colNum = pDataBase->getColNum();
	int rowNum = 0;
	MYSQL_ROW row;
	char dataBuff[65535] = {'\0'};
	char *data = dataBuff;
	data += 4;
	NWriteInt32(data,  &colNum);
	if( colNum > 0 )
	{
		while(row = pDataBase->fetch())
		{
			char *rowdata = data;
			data += colNum * 4;
			for( int i = 0; i < colNum; i++ )
			{
				int len = strlen(row[i]);
				NWriteInt32(rowdata,  &len);
				NWriteBit(data, row[i], len );
			}
			++rowNum;
		}
	}
	pDataBase->freeResult();
	int datalen = data - dataBuff;
	data -= datalen;
	NWriteInt32(data,  &rowNum);
    DBServerAckMsg ack;
	ack.eventid = value.eventid;
	ack.error   = 0;
	ack.datalen = datalen;
	ack.data = (char*)malloc(datalen);
	memcpy(ack.data, dataBuff, datalen);

	MsgSend( m_Epoll, session, DBServerAckMsg, 0, ack);
}

DataBase *DBServer::getDataBase()
{
	DataBase *pDataBase = m_DataBases.find( pthread_self() );
	return pDataBase;
}

void DBServer::connectCenterServer()
{
	KernalThread centerThread;
	centerThread.init(DBServerConnectCenterWorker, this);
	centerThread.detach();
}

void DBServer::registerCenterServerInfo()
{
	const char *ip   = getConfig()->getAttributeStr("config/dba/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/dba/listen", "port");

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
	msg.type = SERVER_DBA;
	msg.ip   = ip;
	msg.port = port;

	MsgSend( m_Epoll, m_CenterServerID, CenterRegisterServerInfo, 0, msg );

    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond );

}
