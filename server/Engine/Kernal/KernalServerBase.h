﻿
#ifndef _KERNAL_SERVER_BASE_H_
#define _KERNAL_SERVER_BASE_H_

#include <stdio.h>
#include <list>
#include <initializer_list>
#include "KernalBuffer.h"
#include "KernalConfig.h"
#include "KernalQueue.h"
#include "KernalStack.h"
#include "KernalMap.h"
#include "KernalEpoll.h"
#include "KernalThread.h"
#include "KernalHash.h"
#include "KernalTimer.h"
#include "KernalUtil.h"
#include "KernalHttpRequest.h"
#include "KernalArrayLockFree.h"
#include "KernalLog.h"

enum KernalMessageType
{
	NETWORK_DATA,     // SOCKET 消息、网络消息（接收数据）
	NETWORK_CLOSE,    // SOCKET 连接关闭
	NETWORK_CONNECT,  // SOCKET 连接
	TIMER_DATA,       // 定时器 消息
};

// 消息处理
class KernalMessage 
{
public:
	KernalMessage()
		:size( 0 ),
		 data( NULL )
	{

	}
	~KernalMessage()
	{
		if( data )
		{
			free( data );
		}
	}
public:
	KernalMessageType  type;    // 消息类型
    KernalNetWorkType  netType; // 如果是网络数据网络连接类型
	void              *data;
	unsigned int       size;
	unsigned int       id;
};

// 线程参数
class KernalServerBase;
struct KernalWorkerThreadArg {
	KernalServerBase *pServerBase;
	int arg;
};

class KernalServerBase
{
public:
	KernalServerBase();
	~KernalServerBase();

	int listen( const char *ip, int port );
	int connect( const char *ip, int port, int &sfd, bool addToEpoll = true );

	int listenHttp( const char *ip, int port );
	int connectHttp( const char *ip, int port );

	void init( const char *configPath );
	void uninit();
	void run();

	void epollWroker();
	
	void worker( int arg );
	int getWorkerThreadKey();

    // 检测心跳
    void heartbeatWorker();

	void quit();//退出
	void flush();
	void pushMsg( KernalMessageType type, KernalNetWorkType netType, void *data, unsigned int size, unsigned int id );
	KernalConfig *getConfig()
	{
		return &m_Config;
	};

	void sendMsg( int fd, char *buff, int len );

	virtual void onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size ) = 0;
	virtual void onInit() = 0;
	virtual void onRun() = 0;
	virtual void onExit() = 0;
protected:
	int                         m_threadNum;
	KernalEpoll                 m_Epoll; //EPOLL
	KernalTimer                 m_Timer;
	KernalLog                   m_Log;
private:
	pthread_key_t   	 						   m_WorkerKey;
	std::vector<KernalThread*>                     m_WorkThreads;    //工作线程
	KernalThread                                   m_EpollThread;    //Epoll 线程
	KernalThread                                   m_HeartBeatThread;//心跳 线程
	KernalConfig                                   m_Config;         //配置文件
	KernalCond                                     m_MessageCond;    //条件变量
	KernalSem                                      m_MessageSem;     //信号量
	std::vector<KernalWorkerThreadArg*>            m_WorkThreadArgs; //工作线程参数
	KernalStack<KernalMessage>                     m_Messages;       //网络消息
	bool                                           m_quit;           //是否退出
};

#endif
