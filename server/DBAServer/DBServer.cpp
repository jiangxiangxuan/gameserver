
#include "DBServer.h"

DBServer *DBServer::ms_pDBServer = NULL;

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

void DBServer::onInit()
{	
	m_Log.init( (char*)(getConfig()->getText("config/dba/log")) );
	m_threadNum = atoi(getConfig()->getText("config/dba/thread"));

	const char *ip   = getConfig()->getAttributeStr("config/dba/listen", "ip");
	int         port = getConfig()->getAttributeInt("config/dba/listen", "port");
	listen(ip, port);
	
	// 初始化数据库连接
	for( int i = 1; i <= m_threadNum; ++i )
	{
		createDataBase( i );
	}
}

void DBServer::createDataBase( int arg )
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

	m_DataBases.insert( arg, pDataBase );
}

void DBServer::handleTimerMsg( unsigned int id )
{
	// 如果是连接中心服务器定时任务
	if( m_ConnCenterTimeID == id )
	{
		connectCenterServer();
	}
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
	else if( NETWORK_CONNECT == type )
	{
		// 如果连接的是中心服务器
		if( m_CenterServerID == id )
		{
			const char *ip   = getConfig()->getAttributeStr("config/dba/listen", "ip");
			int         port = getConfig()->getAttributeInt("config/dba/listen", "port");

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
	}
}

void DBServer::onRun()
{
	connectCenterServer();
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
	char *dataBuff = pDataBase->getDataBuff();
	memset( dataBuff, 0, DATABASE_BUFF_SIZE );
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

	MsgSend( m_Epoll, session, DBServerAckMsg, 0, ack );
}

DataBase *DBServer::getDataBase()
{
	//DataBase *pDataBase = m_DataBases.find( pthread_self() );
	DataBase *pDataBase = m_DataBases.find( getWorkerThreadKey() );
	return pDataBase;
}

void DBServer::connectCenterServer()
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
