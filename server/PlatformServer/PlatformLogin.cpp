
#include "PlatformLogin.h"

PlatformLogin::PlatformLogin()
{

}

PlatformLogin::~PlatformLogin()
{

}

void PlatformLogin::VerifyToken( KernalEpoll *pEpoll, int session, int clientid, char *data, int msglen )
{
    platformprotocol::CVerifyToken verifyToken;
    ParseFromArrayToObj(verifyToken, data, msglen);

    printf("platform login verify token = %s \r\n", verifyToken.token().c_str());

    platformprotocol::CVerifyToken verifyToken1;
    verifyToken1.set_token(verifyToken.token());
    
	ProtobufMsgSendToClientByGateWay((*pEpoll),session,clientid, platformprotocol::PLATFORM_VERIFY_TOKEN,0,verifyToken1);
}

