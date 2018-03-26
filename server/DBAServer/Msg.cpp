
#include "Msg.h"
#include "DBServer.h"

void onDBServerReqMsg(int session, DBServerReqMsg &value)
{
	DBServer::getInstance()->execute( session, value );
}
