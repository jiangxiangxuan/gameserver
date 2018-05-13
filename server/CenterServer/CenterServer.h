
#ifndef _CENTER_SERVER_H_
#define _CENTER_SERVER_H_

#include "Kernal/KernalServerBase.h"
#include "Kernal/KernalMap.h"
#include "Msg.h"

struct ServerInfo
{
	ServerType   type;
	unsigned int id;
	char         ip[50];
	int          port;
	int          num;
	bool         isClosed;

	ServerInfo()
	{
		memset( this, 0, sizeof(ServerInfo) );
	}
};

class CenterServer : public KernalServerBase
{
public:
	CenterServer();
	~CenterServer();

	static void createInstance();
	static void destroyInstance();
	static CenterServer *getInstance();

	virtual void onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size );
	virtual void onProcess();
	virtual void onRun();
	virtual void onExit();
	virtual void oninit();
	virtual void onuninit();

public:
	void handleTimerMsg( unsigned int id );
	void handleRegisterServerInfo(int session, CenterRegisterServerInfo &value);
	void handleCenterUpdateServerInfo(int session, CenterUpdateServerInfo &value);
	void handleGateWayMsg( int session, int clientID, char *data, int datalen ); // 处理网关数据

	void sendMsgToGameByPlatform(PlatformGameServerMsg &value);
	void sendMsgToPlatformByGame(PlatformGameServerMsg &value);
protected:
	void notifyServerInfo( ServerType type, char *ip, int port, ServerType serverType, ServerStateType state );
	void sendServerInfo( int id, ServerType type, ServerStateType state );
	void sendServerInfo( int id, ServerType type, ServerStateType state, char *ip, int port );
	
	bool isInternalServer( unsigned int id );
	bool delInternalServer( unsigned int id );
private:
	static CenterServer *ms_pCenterServer;
private:
	KernalMutexLocker  m_Locker;
	KernalMutexLocker  m_ServersLocker;

	std::multimap< ServerType, ServerInfo* > m_Servers;
	typedef std::multimap< ServerType, ServerInfo* >::iterator server_multimap;
};

#endif
