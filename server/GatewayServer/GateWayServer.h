
#ifndef _GATEWAY_SERVER_H_
#define _GATEWAY_SERVER_H_

#include "Kernal/KernalServerBase.h"
#include "Kernal/KernalMap.h"
#include "Msg.h"

struct ServerInfo
{
	ServerType   type;
	unsigned int id;
	char         ip[50];
	int          port;

	ServerInfo()
	{
		memset( this, 0, sizeof(ServerInfo) );
	}
};

class GateWayServer : public KernalServerBase
{
public:
	GateWayServer();
	~GateWayServer();

	static void createInstance();
	static void destroyInstance();
	static GateWayServer *getInstance();

	virtual void onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size );
	virtual void onProcess();
	virtual void onRun();
	virtual void onExit();
	virtual void oninit();

public:
	void sendMsgToClient( int id, int uid, char *data, int datalen );
	void sendMsgToCenter( unsigned int id, KernalMessageType type, const char *data, unsigned int size );
	void sendMsgToPlatform( unsigned int id, KernalMessageType type, const char *data, unsigned int size );
	void sendMsgToGame( unsigned int id, KernalMessageType type, const char *data, unsigned int size );
	void handleTimerMsg( unsigned int id );
	void handleCenterNotifyServerInfo( CenterNotifyServerInfo &value );
protected:
	void connectCenterServer();

	void connInternalServer( CenterNotifyServerInfo &value ); // 连接内部服务器
	void closeInternalServer( CenterNotifyServerInfo &value );// 关闭内部服务器连接
	bool closeInternalServer( unsigned int id );              // 关闭内部服务器连接

	bool isInternalServer( unsigned int id );
	
	int getUidByClientID( int id );
	void offlineByClientID( int id );
private:
	KernalMutexLocker     m_ServersLocker;
	
	static GateWayServer *ms_pGateWayServer;
	int                   m_CenterServerID;
	unsigned int          m_ConnCenterTimeID; // 连接中心服务器的定时任务eventid
	KernalThread          m_CenterThread;     // 连接中心服务器 线程

	std::multimap< ServerType, ServerInfo* > m_Servers;
	typedef std::multimap< ServerType, ServerInfo* >::iterator server_multimap;
	
	std::map< int, int >  m_Players; // clientid 对应的 uid
	
	int                   m_CenterMinCmd;
	int                   m_CenterMaxCmd;
	
	int                   m_PlatformMinCmd;
	int                   m_PlatformMaxCmd;
	
	int                   m_GameMinCmd;
	int                   m_GameMaxCmd;

};

#endif
