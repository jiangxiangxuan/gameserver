
#ifndef _PLATFORM_SERVER_H_
#define _PLATFORM_SERVER_H_

#include "Kernal/KernalServerBase.h"
#include "Kernal/KernalMap.h"
#include "Idbc/redis/idbc_redis_noncluster.h"
#include "Msg.h"
#include "DBAgent.h"
#include "PlatformLogin.h"

class PlatformServer : public KernalServerBase
{
public:
	PlatformServer();
	~PlatformServer();

	static void createInstance();
	static void destroyInstance();
	static PlatformServer *getInstance();

	virtual void onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size );
	virtual void onInit();
	virtual void onRun();
	virtual void onExit();

	IdbcRedis *getRedis()
	{
		return m_pIdbcRedis;
	}

	DBAgent *getDBAgent()
	{
		return &m_DBAgent;
	};
public:
	void sendMsgToClient( int session, int clientID, char *data, int datalen );
	void sendMsgToGame( int serverID, char *data, int datalen );
	int executeDBSql( const char *sql, KernalObject *pObj = NULL, DBEvent_fn func = NULL );
	unsigned int addTimer( unsigned int expire, int times );
	void delTimer( unsigned int id );
public:
	void handleGateWayMsg( int session, int clientID, char *data, int datalen ); // 处理网关数据
	void handleCenterNotifyServerInfo( CenterNotifyServerInfo &value );
	
	void handleDBMsg( int session, int eventid, int error, char *data, int datalen ); // 处理DBA数据
	void handleGameMsg( int serverID, char *data, int datalen ); // 处理游戏消息
	void handleTimerMsg( unsigned int id );	
protected:
	void connectCenterServer();
private:
	static PlatformServer *ms_pPlatformServer;
private:
	IdbcRedis         *m_pIdbcRedis;
	DBAgent            m_DBAgent;
	KernalThread       m_CenterThread;     // 连接中心服务器 线程
	unsigned int       m_ConnCenterTimeID; // 连接中心服务器的定时任务eventid
	int                m_CenterServerID;
private:
    PlatformLogin      m_PlatformLogin;
};

#endif
