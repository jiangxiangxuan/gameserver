
#include "Msg.h"
#include "GateWayServer.h"

void onGateWayInternalServerMsg(int session, GateWayInternalServerMsg &value)
{
	GateWayServer::getInstance()->sendMsgToClient( value.clientID, value.UID, value.data, value.datalen );
}

void onCenterNotifyServerInfo(int session, CenterNotifyServerInfo &value)
{
	GateWayServer::getInstance()->handleCenterNotifyServerInfo( value );
}
