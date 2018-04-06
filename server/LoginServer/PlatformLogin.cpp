
#include "PlatformLogin.h"

PlatformLogin::PlatformLogin()
{

}

PlatformLogin::~PlatformLogin()
{

}

void PlatformLogin::setPlatformAddr( const char *addr )
{
	memset(m_platformAddr, 0, sizeof(m_platformAddr) );
	sprintf( m_platformAddr, "%s", addr );
}

void PlatformLogin::VerifyToken( KernalEpoll *pEpoll, int session, int clientid, char *data, int msglen )
{
    platformprotocol::CVerifyToken verifyToken;
    ParseFromArrayToObj(verifyToken, data, msglen);

    printf("platform login verify token = %s \r\n", verifyToken.token().c_str());
    
	char url[256] = { 0 };
	sprintf( url, "%s/platform/verifytoken?token=%s", m_platformAddr, verifyToken.token().c_str() );
	
    KernalHttpRequest httpRequest;
    std::string httpdata = httpRequest.Request( url );
    Json::Value jvalue = m_Json.parseJson(httpdata.c_str());
    printf("platform login http request state=%d data=%s \r\n", jvalue["state"].asInt(),httpdata.c_str());

    platformprotocol::CVerifyToken verifyToken1;
    verifyToken1.set_token(verifyToken.token());
    
	ProtobufMsgSendToClientByGateWay((*pEpoll),session,clientid, platformprotocol::PLATFORM_VERIFY_TOKEN,0,verifyToken1);
}
