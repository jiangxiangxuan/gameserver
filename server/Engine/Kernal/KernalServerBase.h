
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
	KernalMessageType  type;    // 消息类型
    KernalNetWorkType  netType; // 如果是网络数据网络连接类型
	void              *data;
	unsigned int       size;
	unsigned int       id;
};

#if 0
// 线程的通信管道
class KernalServerBase;
class KernalCommunicationPipe {
public:
	KernalCommunicationPipe()
	{
		memset( pipefd, 0, sizeof(pipefd) );
	}
public:
	int       		  pipefd[2];
	pthread_t 		  tid;
	KernalServerBase *pServerBase;
};
#endif

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
	
	//void worker(KernalCommunicationPipe *pComPipe);
	void worker();

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
	virtual void onProcess() = 0;
	virtual void onRun() = 0;
	virtual void onExit() = 0;
	virtual void oninit() = 0;
	virtual void onuninit() = 0;

	virtual void onWorkerBegin();
	virtual void onWorkerEnd();
protected:
	int                         m_threadNum;
	KernalEpoll                 m_Epoll;         //EPOLL
	KernalTimer                 m_Timer;
	KernalLog                   m_Log;
private:
	//KernalQueue<KernalMessage>                     m_Messages;       //需处理消息
	//KernalArrayLockFree<KernalMessage*>            m_Messages;       //需处理消息(无锁)
	KernalStack<KernalThread>                      m_WorkThreads;    //工作线程
	KernalThread                                   m_EpollThread;    //Epoll 线程
	KernalThread                                   m_HeartBeatThread;//心跳 线程
	KernalConfig                                   m_Config;         //配置文件
	KernalCond                                     m_MessageCond;    //条件变量
	KernalSem                                      m_MessageSem;     //信号量
	//std::vector<KernalCommunicationPipe*>          m_WorkThreadsPipe;//工作线程管道
	bool                                           m_quit;           //是否退出
};

#endif
