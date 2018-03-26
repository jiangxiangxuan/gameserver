
#include "Msg.h"
#include "DBAgent.h"
#include "Idbc/redis/idbc_redis_noncluster.h"
#include "PlatformServer.h"

void onGateWayInternalServerMsg( int session, GateWayInternalServerMsg &value )
{
	PlatformServer::getInstance()->handleGateWayMsg( session, value.clientID, value.data, value.datalen );
}

void onDBServerAckMsg( int session, DBServerAckMsg &value )
{
	PlatformServer::getInstance()->handleDBMsg( session, value.eventid, value.error, value.data, value.datalen );
}

void onPlatformGameServerMsg(int session, PlatformGameServerMsg &value)
{
	PlatformServer::getInstance()->handleGameMsg( value.serverID, value.data, value.datalen );
}

void onCenterNotifyServerInfo(int session, CenterNotifyServerInfo &value)
{
	PlatformServer::getInstance()->handleCenterNotifyServerInfo( value );
}
