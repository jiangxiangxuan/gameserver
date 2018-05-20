
#include "PlatformLogin.h"
#include "Common/CacheKey.h"

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

void PlatformLogin::VerifyToken( KernalEpoll *pEpoll, IdbcRedis *pIdbcRedis, int session, int clientid, char *data, int msglen )
{
#if 1  // 测试
	int uid1 = 111;
    platformprotocol::SVerifyToken sverifyTokent;
	protocol::PlayerInfo* playerInfo = sverifyTokent.mutable_playerinfo();
	playerInfo->set_uid( uid1 );
	ProtobufMsgSendToClientByGateWay( (*pEpoll), session, clientid, uid1, platformprotocol::PLATFORM_VERIFY_TOKEN, 0, sverifyTokent );
	return;
#endif
    platformprotocol::CVerifyToken verifyToken;
    ParseFromArrayToObj(verifyToken, data, msglen);

    printf("platform login verify token = %s \r\n", verifyToken.token().c_str());
    
	char url[256] = { 0 };	
	verifyToken.set_token("0e5c4f65c9ea990c36c41439aca88aef8d7e0ab1");
	sprintf( url, "%s/platform/verifytoken?token=%s", m_platformAddr, verifyToken.token().c_str() );
	
    KernalHttpRequest httpRequest;
    std::string httpdata = httpRequest.Request( url );
    Json::Value jvalue = m_Json.parseJson(httpdata.c_str());
    printf("platform login http request state=%d data=%s \r\n", jvalue["state"].asInt(),httpdata.c_str());
	
    platformprotocol::SVerifyToken sverifyToken;
	
	int state = jvalue["state"].asInt();
	int uid   = 0;
	if( 0 == state )
	{
		uid = jvalue["data"]["uid"].asInt();
		
		char userkey[128] = { 0 };
		sprintf( userkey, g_GameUserInfo, uid );
		
		char tmp[8] = { 0 };
		sprintf( tmp, "%d", uid );
		pIdbcRedis->sethmkey( userkey, g_GameUserInfo_Uid, tmp );
		
		sprintf( tmp, "%d", session );
		pIdbcRedis->sethmkey( userkey, g_GameUserInfo_GateWay, tmp );
		
		pIdbcRedis->sethmkey( userkey, g_GameUserInfo_NickName, jvalue["data"]["nickname"].asString().c_str() );
		pIdbcRedis->sethmkey( userkey, g_GameUserInfo_UserName, jvalue["data"]["username"].asString().c_str() );
		
		protocol::PlayerInfo* playerInfo = sverifyToken.mutable_playerinfo();
		playerInfo->set_uid( uid );
	}	
    
	ProtobufMsgSendToClientByGateWay( (*pEpoll), session, clientid, uid, platformprotocol::PLATFORM_VERIFY_TOKEN, 0, sverifyToken );
}
