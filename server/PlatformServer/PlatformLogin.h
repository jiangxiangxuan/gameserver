
#ifndef _PLATFORM_LOGIN_H_
#define _PLATFORM_LOGIN_H_
#include "Kernal/KernalEpoll.h"
#include "Msg.h"

class PlatformLogin {
public:
    PlatformLogin();
    ~PlatformLogin();
public:
	void VerifyToken( KernalEpoll *pEpoll, int session, int clientid, char *data, int msglen );
private:    
};

#endif
