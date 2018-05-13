
#ifndef _DB_AGENT_H_
#define _DB_AGENT_H_

#include <string.h>
#include <vector>
#include "Kernal/KernalObject.h"
#include "Kernal/KernalMap.h"
#include "Kernal/KernalLock.h"
#include "Kernal/KernalEpoll.h"
#include "Kernal/KernalThread.h"

#include "../Protocol/ServerMsg.h"

class DBResult;
typedef void (KernalObject::*DBEvent_fn)( int, DBResult& );
struct DBEvent
{
	int eventid;
	KernalObject *pObj;
	DBEvent_fn    eventFunc;
};

struct DBAServerInfo
{
	ServerType   type;
	unsigned int id;
	char         ip[50];
	int          port;

	DBAServerInfo()
	{
		memset( this, 0, sizeof(DBAServerInfo) );
	}
};

// 结果处理
class DBResult
{
public:
	DBResult();
	DBResult( int row, int col, const char *data, int datalen );
	~DBResult();

	void init( int row, int col, const char *data, int datalen );
	void clear();

	bool nextRow();
	std::string getData( int col );
	int getRow()
	{
		return m_Row;
	}
private:
	char *m_ResData;
	char *m_CurRowData;
	int   m_Row;
	int   m_Col;
	int   m_CurRow;
};

class DBAgent
{
public:
	DBAgent();
	~DBAgent();

	bool init( KernalEpoll *pEpoll );
	int execute( const char *sql, KernalObject *pObj = NULL, DBEvent_fn func = NULL );
	void handleData( int error, int eventid, char *data, int datalen );
	void releaseEvent( int eventid );

	bool checkIsDBA( int id );

public:
	bool handleSocketClose( int id );
	void addDBAServer(CenterNotifyServerInfo &value);
	void removeDBAServer(CenterNotifyServerInfo &value);
private:
	KernalEpoll      *m_pEpoll;         //EPOLL
	KernalMutexLocker m_locker;
	int               m_CurEventID;
	bool              m_Quit;
	KernalMap<int, DBEvent*> m_dbevents;

	std::vector< DBAServerInfo* > m_Servers;
};

#endif
