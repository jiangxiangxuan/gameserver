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

enum KernalMessageType
{
	NETWORK_DATA,  // SOCKET 消息、网络消息（接收数据）
	NETWORK_CLOSE,  // SOCKET 连接关闭
	TIMER_DATA,    // 定时器 消息
};

// 消息处理
class KernalMessage //: public KernalListNode< KernalMessage >
{
public:
	KernalMessage()
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
	KernalMessageType  type;
	void              *data;
	unsigned int       size;
	unsigned int       id;
};

class KernalServerBase
{
public:
	KernalServerBase();
	~KernalServerBase();

	int listen( const char *ip, int port );
	int connect( const char *ip, int port, bool addToEpoll = true );

	int listenHttp( const char *ip, int port );
	int connectHttp( const char *ip, int port );

	void init( const char *configPath );
	void uninit();
	void run();

	void timerWroker();
	void epollWroker();
	void worker();
    // 检测心跳
    void heartbeatWorker();

	void quit();//退出
	void flush();
	void pushMsg( KernalMessageType type, void *data, unsigned int size, unsigned int id );
	KernalConfig *getConfig()
	{
		return &m_Config;
	};

	void sendMsg( int fd, char *buff, int len );

	virtual void onMsg( unsigned int id, KernalMessageType type, const char *data, unsigned int size ) = 0;
	virtual void onProcess() = 0;
	virtual void onRun() = 0;
	virtual void onExit() = 0;
	virtual void oninit() = 0;
	virtual void onuninit() = 0;

	virtual void onWorkerPre();
	virtual void onWorkerEnd();
protected:
	int                         m_threadNum;
	KernalEpoll                 m_Epoll;         //EPOLL
	KernalTimer                 m_Timer;
private:
	KernalQueue<KernalMessage>  m_Messages;       //需处理消息
	KernalStack<KernalThread>   m_WorkThreads;    //工作线程
	KernalThread                m_EpollThread;    //Epoll 线程
	KernalThread                m_TimerThread;    //Timer 线程
	KernalThread                m_HeartBeatThread;//心跳 线程
	KernalConfig                m_Config;         //配置文件
	KernalCond                  m_MessageCond;

	bool                        m_quit;//是否退出
};

#endif