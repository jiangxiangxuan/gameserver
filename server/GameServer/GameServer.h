
#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_

#include "Kernal/KernalServerBase.h"
#include "Kernal/KernalMap.h"
#include "Idbc/redis/idbc_redis_noncluster.h"
#include "Msg.h"

class GameServer : public KernalServerBase
{
public:
	GameServer();
	~GameServer();

	static void createInstance();
	static void destroyInstance();
	static GameServer *getInstance();

	virtual void onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size );
	virtual void onProcess();
	virtual void onRun();
	virtual void onExit();
	virtual void oninit();
	virtual void onuninit();

	IdbcRedis *getRedis()
	{
		return m_pIdbcRedis;
	}

public:
	void handleGateWayMsg( int session, int clientID, char *data, int datalen ); // 处理网关数据
	void handleCenterNotifyServerInfo( CenterNotifyServerInfo &value );

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
	int                m_CenterServerID;
	KernalThread       m_CenterThread;//连接中心服务器 线程
};

#endif
