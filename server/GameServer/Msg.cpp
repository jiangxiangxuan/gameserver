
#include "Msg.h"
#include "Idbc/redis/idbc_redis_noncluster.h"
#include "GameServer.h"

void onGateWayInternalServerMsg( int session, GateWayInternalServerMsg &value )
{
	GameServer::getInstance()->handleGateWayMsg( session, value.clientID, value.data, value.datalen );
}

void onPlatformGameServerMsg(int session, PlatformGameServerMsg &value)
{
	GameServer::getInstance()->handlePlatformMsg( value.serverID, value.data, value.datalen );
}

void onCenterNotifyServerInfo(int session, CenterNotifyServerInfo &value)
{
	GameServer::getInstance()->handleCenterNotifyServerInfo( value );
}
