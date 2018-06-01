
#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_

#include "Kernal/KernalServerBase.h"
#include "Kernal/KernalMap.h"
#include "Idbc/redis/idbc_redis_noncluster.h"
#include "Msg.h"
#include "DBAgent.h"

class GameServer : public KernalServerBase
{
public:
	GameServer();
	~GameServer();

	static void createInstance();
	static void destroyInstance();
	static GameServer *getInstance();

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
	int executeDBSql( const char *sql, KernalObject *pObj = NULL, DBEvent_fn func = NULL );	
public:
	void handleGateWayMsg( int session, int clientID, char *data, int datalen ); // 处理网关数据
	void handleCenterNotifyServerInfo( CenterNotifyServerInfo &value );
	
	void handleDBMsg( int session, int eventid, int error, char *data, int datalen ); // 处理DBA数据
	void handleTimerMsg( unsigned int id );
	void handlePlatformMsg( int serverID, char *data, int datalen ); // 处理大厅消息
	//void sendMsgToClient( int id, char *data, int datalen );
	void sendMsgToClient( int session, int clientID, char *data, int datalen );
	void sendMsgToPlatform( int serverID, char *data, int datalen );
	unsigned int addTimer( unsigned int expire, int times );
	void delTimer( unsigned int id );	
protected:
	void connectCenterServer();
private:
	static GameServer *ms_pGameServer;
private:
	IdbcRedis         *m_pIdbcRedis;
	DBAgent            m_DBAgent;
	int                m_CenterServerID;
	unsigned int       m_ConnCenterTimeID; // 连接中心服务器的定时任务eventid
	KernalThread       m_CenterThread;     // 连接中心服务器 线程
};

#endif
