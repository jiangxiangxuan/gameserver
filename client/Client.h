
#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "../server/Engine/Kernal/KernalServerBase.h"
#include "Kernal/KernalMap.h"
#include "Msg.h"

class Client : public KernalServerBase
{
public:
	Client();
	~Client();

	static void createInstance();
	static void destroyInstance();
	static Client *getInstance();

	virtual void onMsg( unsigned int id, KernalMessageType type, const char *data, unsigned int size );
	virtual void onProcess();
	virtual void onRun();
	virtual void onExit();
	virtual void oninit();
	virtual void onuninit();

	void clientWorker();
	void handleTimerMsg( unsigned int id );
private:
	KernalStack<KernalThread>   m_ClientWorkThreads;   //工作线程
	static Client *ms_pClient;
};

#endif
