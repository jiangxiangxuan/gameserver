
#include "DBAgent.h"

DBResult::DBResult()
	: m_Row( 0 ),
	  m_Col( 0 ),
	  m_CurRow( 0 ),
	  m_ResData( NULL )
{

}

DBResult::DBResult( int row, int col, const char *data, int datalen )
	: m_Row( row ),
	  m_Col( col ),
	  m_CurRow( 0 ),
	  m_ResData( NULL )
{
	init( row, col, data, datalen );
}

DBResult::~DBResult()
{
	clear();
}

void DBResult::init( int row, int col, const char *data, int datalen )
{
	m_Row = row;
	m_Col = col;
	m_CurRow = 0;
	m_ResData = (char*)malloc( datalen );
	memcpy( m_ResData, data, datalen );

	m_CurRowData = m_ResData;
}

void DBResult::clear()
{
	if( m_ResData )
	{
		free( m_ResData );
		m_ResData = NULL;
	}
}

bool DBResult::nextRow()
{
	++m_CurRow;
	if( m_CurRow > m_Row )
	{
		return false;
	}

	if( 1 == m_CurRow )
	{
		return true;
	}

	int rowlen = m_Col * 4;
	for( int i = 0; i < m_Col; ++i )
	{
		rowlen += *((int*)(m_CurRowData + i * 4 ));
	}
	m_CurRowData += rowlen;
	return true;
}

std::string DBResult::getData( int col )
{
	if( col >= m_Col )
	{
		return "";
	}
	int rowlen = m_Col * 4;
	int collen = *((int*)(m_CurRowData));
	for( int i = 0; i < col; ++i )
	{
		collen = *((int*)(m_CurRowData + (i + 1) * 4 ));
		rowlen += *((int*)(m_CurRowData + i * 4 ));
	}

	std::string result;
	result.assign( m_CurRowData + rowlen, collen );
	return result;
}

DBAgent::DBAgent()
	: m_CurEventID( 0 )
{

}

DBAgent::~DBAgent()
{

}

bool DBAgent::init( KernalEpoll *pEpoll, const char *ip, int port )
{
	m_pEpoll = pEpoll;
	return true;
}

int DBAgent::execute( const char *sql, KernalObject *pObj, DBEvent_fn func )
{
	m_locker.lock();
	DBEvent *event = new DBEvent();
	event->eventid     = ++m_CurEventID;   // 事件ID
	event->pObj        = pObj;
	event->eventFunc   = func;

	m_dbevents.insert( event->eventid, event );

	DBServerReqMsg msg;
	msg.eventid = event->eventid;
	msg.sqlStr  = sql;
	msg.sqlType = DBSqlType_Select;

	for( auto it = m_Servers.begin(); it != m_Servers.end(); ++it )
	{
		MsgSend( (*m_pEpoll), (*it)->id, DBServerReqMsg, 0, msg);
		break;
	}

	m_locker.unlock();
	return event->eventid;
}

void DBAgent::handleData( int error, int eventid, char *data, int datalen )
{
	if( datalen <= 8 )
	{
		return;
	}
	int row = *((int*)(data));
	data += 4;
	int col = *((int*)(data));
	data += 4;
	DBResult result( row, col, data, datalen - 8 );

	DBEvent *event = m_dbevents.find( eventid );

	if( event && event->pObj && event->eventFunc )
	{
		((event->pObj)->*(event->eventFunc))( error, result);
	}

	releaseEvent( eventid );
}

void DBAgent::releaseEvent( int eventid )
{
	m_locker.lock();
	DBEvent *event = m_dbevents.find( eventid );

	m_dbevents.erase( eventid );
	if( event )
	{
		delete event;
	}
	m_locker.unlock();
}

bool DBAgent::checkIsDBA( int id )
{
	bool isFind = false;
	m_locker.lock();
	for( auto it = m_Servers.begin(); it != m_Servers.end(); ++it )
	{
		if( (*it)->id == id )
		{
			isFind = true;
			break;
		}
	}
	m_locker.unlock();

	return isFind;
}

bool DBAgent::handleSocketClose( int id )
{
	m_locker.lock();
	for( auto it = m_Servers.begin(); it != m_Servers.end(); ++it )
	{
		if( (*it)->id == id )
		{
			m_Servers.erase(it);
			delete *it;
			break;
		}
	}
	m_locker.unlock();

	return true;
}

void DBAgent::addDBAServer(CenterNotifyServerInfo &value)
{
	if( SERVER_DBA != value.type )
	{
		return;
	}

	m_locker.lock();
	bool isFind = false;
	for( auto it = m_Servers.begin(); it != m_Servers.end(); ++it )
	{
		if( (*it)->port == value.port && 0 == strcmp((*it)->ip, value.ip.c_str()) )
		{
			isFind = true;
			break;
		}
	}
	if( isFind )
	{
		m_locker.unlock();
		return;
	}

	int id = m_pEpoll->connect( value.ip.c_str(), value.port );
	if( id > 0 )
	{
		DBAServerInfo *pServer = new DBAServerInfo();
		pServer->type = value.type;
		memcpy( pServer->ip, value.ip.c_str(), value.ip.length() );
		pServer->port = value.port;
		pServer->id = id;

		m_Servers.push_back( pServer );
	}
	m_locker.unlock();
}

void DBAgent::removeDBAServer(CenterNotifyServerInfo &value)
{
	if( SERVER_DBA != value.type )
	{
		return;
	}

	m_locker.lock();
	for( auto it = m_Servers.begin(); it != m_Servers.end(); ++it )
	{
		if( (*it)->port == value.port && 0 == strcmp((*it)->ip, value.ip.c_str()) )
		{
			m_Servers.erase(it);
			delete *it;
			break;
		}
	}
	m_locker.unlock();
}
