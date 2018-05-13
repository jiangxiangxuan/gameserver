
#ifndef _DB_SERVER_H_
#define _DB_SERVER_H_

#include "Kernal/KernalServerBase.h"
#include "Kernal/KernalMap.h"
#include "Msg.h"
#include "DataBase.h"

class DBServer : public KernalServerBase
{
public:
	DBServer();
	~DBServer();

	static void createInstance();
	static void destroyInstance();
	static DBServer *getInstance();

	void execute( int session, DBServerReqMsg &value );

	virtual void onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size );
	virtual void onProcess();
	virtual void onRun();
	virtual void onExit();
	virtual void oninit();
	virtual void onuninit();

	virtual void onWorkerBegin();
	virtual void onWorkerEnd();
protected:
	void connectCenterServer();

	void handleTimerMsg( unsigned int id );

	DataBase *getDataBase();
private:
	static DBServer *ms_pDBServer;
private:
	//DataBase       *m_pDataBase;
	KernalMap< unsigned int, DataBase * > m_DataBases;

	int                m_CenterServerID;
	unsigned int       m_ConnCenterTimeID; // 连接中心服务器的定时任务eventid
	KernalThread       m_CenterThread;     // 连接中心服务器 线程
};

#endif
