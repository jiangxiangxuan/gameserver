
#ifndef _PLATFORM_LOGIN_H_
#define _PLATFORM_LOGIN_H_
#include "Kernal/KernalEpoll.h"
#include "Kernal/KernalHttpRequest.h"
#include "Kernal/KernalJson.h"
#include "Msg.h"

class PlatformLogin {
public:
    PlatformLogin();
    ~PlatformLogin();
public:
	void setPlatformAddr( const char *addr );
public:
	void VerifyToken( KernalEpoll *pEpoll, int session, int clientid, char *data, int msglen );
private:
    KernalJson m_Json;
	
	char m_platformAddr[128]; // 平台http接口地址
};

#endif
