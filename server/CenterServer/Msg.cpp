
#include "Msg.h"
#include "CenterServer.h"

void onCenterRegisterServerInfo(int session, CenterRegisterServerInfo &value)
{
    CenterServer::getInstance()->handleRegisterServerInfo(session, value);
}

void onCenterUpdateServerInfo(int session, CenterUpdateServerInfo &value)
{
    CenterServer::getInstance()->handleCenterUpdateServerInfo(session, value);
}

void onGateWayInternalServerMsg(int session, CenterUpdateServerInfo &value)
{
    CenterServer::getInstance()->handleGateWayMsg(session, value);
}

void onPlatformGameServerMsg(int session, PlatformGameServerMsg &value)
{
    if( LINETYPE_PLATFORMTOGAME == value.lineType )
    {
        CenterServer::getInstance()->sendMsgToGameByPlatform(value);     
    }
    else if( LINETYPE_GAMETOPLATFORM == value.lineType )
    {
        CenterServer::getInstance()->sendMsgToPlatformByGame(value);
    }
}
