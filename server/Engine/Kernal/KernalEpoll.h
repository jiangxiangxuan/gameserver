
#ifndef _KERNAL_EPOLL_H_
#define _KERNAL_EPOLL_H_

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <string.h>
#include <string>

#include "KernalNetWorkMsgDef.h"
#include "KernalLock.h"

#define HTTP_BUFFER_SIZE  1024  // http 请求数据大小

#define MAX_EVENTS        64
#define MAX_NET_WORK_NUM  65535 //最多接收连接数
#define RECV_BUFFER_SIZE  1024  // 接收数据缓冲区大小

#define HASH_ID(id) (((unsigned)id) % MAX_NET_WORK_NUM)
//#define HASH_ID(id) (((unsigned)id))

const int socket_data         = 0;    // 发送数据
const int socket_connect      = 1;    // 建立连接
const int socket_connect_http = 2;    // 建立HTTP连接
const int socket_listen       = 3;    // 监听连接
const int socket_listen_http  = 4;    // 监听HTTP连接
const int socket_close        = 5;    // 关闭连接

enum KernalNetWorkType
{
    KernalNetWorkType_NO,
    KernalNetWorkType_CONNECTED,
    KernalNetWorkType_CONNECTED_HTTP,
    KernalNetWorkType_LISTEN,
    KernalNetWorkType_LISTEN_HTTP,
};

struct KernalNetWorkBuffer
{
    struct KernalNetWorkBuffer *next;
    void *data;
    int size;

    KernalNetWorkBuffer()
        :next( NULL )
        ,data( NULL )
        ,size( 0 )
    {

    }
    ~KernalNetWorkBuffer()
    {

    }
};

struct KernalNetWorkBufferChain
{
    struct KernalNetWorkBuffer *head;
    struct KernalNetWorkBuffer *tail;

    void appendBuffer( void *data, int size )
    {
        struct KernalNetWorkBuffer *buffer = new KernalNetWorkBuffer();
        buffer->data = data,
        buffer->size = size;
        buffer->next = NULL;
        if( head == NULL)
        {
	    	head = tail = buffer;
	    }
        else
        {
		    tail->next = buffer;
	    	tail = buffer;
	    }
    }
};

// 发送（通过管道）
struct KernalResponseMsg
{
    int   id;
    int   size;
    char *data;

    KernalResponseMsg()
        :id( 0 )
        ,size( 0 )
        ,data( 0 )
    {

    }
    ~KernalResponseMsg()
    {
        if( data )
        {
            //free( data );
        }
    }

};

// 接收
struct KernalRequestMsg
{
    unsigned int       id;
    unsigned int       size;
    char              *data;
    KernalNetWorkType  netType; 
public:
    KernalRequestMsg()
    {
        init();
    }

    void init()
    {
        id   = 0;
        size = 0;
        data = NULL;
    }
};

struct KernalNetWork
{
    unsigned int id;
    unsigned int fd;
    bool isWrite;
    bool isRead;
    char *readBuffers;
    int  readBuffersLen;

    KernalNetWorkType type;
    KernalNetWorkBufferChain buffers;
public:
    KernalNetWork()
        : id( 0 )
        , fd( 0 )
        , isRead( false )
        , isWrite( false )
        , type( KernalNetWorkType_NO )
        , readBuffersLen( 0 )
        , readBuffers( NULL )
    {

    }

    void init()
    {
        clear();
        readBuffers = ( char * )malloc(RECV_BUFFER_SIZE);
        memset( readBuffers, 0, RECV_BUFFER_SIZE );
    }
    void clear()
    {
        id = 0;
        fd = 0;
        isRead = false;
        isWrite = false;
        type = KernalNetWorkType_NO;
        readBuffersLen = 0;
        if( readBuffers )
        {
            free( readBuffers );
            readBuffers = NULL;
        }
    }
};

enum KernalSocketMessageType
{
    KernalSocketMessageType_NO,
    KernalSocketMessageType_SOCKET_DATA,
    KernalSocketMessageType_SOCKET_CONNECT,
    KernalSocketMessageType_SOCKET_CLOSE,
};

// 管道
struct KernalPipe
{
	int pipe[2];
};

class KernalEpoll
{
public:
    KernalEpoll();
    ~KernalEpoll();

public:
    bool create();
    int listen( const char *addr, const int port, bool isHttp = false );
    int connect( const char *addr, const int port, int &sfd, bool isHttp = false, bool addToEpoll = true );
    //int connectSocket( const char *addr, const int port );
	
	// 创建工作线程管道
	KernalPipe *createWorkerPipe( pthread_t tid );
	void releaseWorkerPipes();
	
    // HTTP
    int listenHttp( const char *addr, const int port );
    int connectHttp( const char *addr, const int port );

    // 通过管道发送数据
    bool send( int id, void *data, int size );
    bool sendToPipe( void *data, int size );
    // 发送HTTP数据
    void sendHttpData( int id, const void *data, int size );
    // 处理消息（每次处理一条消息）
    KernalSocketMessageType handleMessage( KernalRequestMsg &result );

    void epollAdd( int id, bool isET = false );
    void epollAdd( int fd, void *data, bool isET = false );
    void epollMod( int fd, int op, void *data, bool isET = false );
    void epollDel( int id );
    void release();

    void close( int id );

    // 检测心跳
    void heartbeat();
private:
	// 检测是否是工作管道
	bool checkIsWorkerPipe( int fd );
	// 获取工作管道(根据当前线程)
	KernalPipe *getWorkerPipe(); 
    void closeSocket( int id );
    int readMsg( int fd, void *data, int size, bool useRead = false, bool readOnce = false );
    // 发送数据到指定的socket
    int sendMsg( int fd, const void *data, int size, bool useWrite = false );
    int readHttpMsg( int fd, void *data, int size );
private:
    int getSocketID();
private:
    int                  m_epollfd;
    //int                  m_ctrlfd[2]; // 0:接受 1:发送

    struct epoll_event   m_events[ MAX_EVENTS ];
    int                  m_eventNum;
    int                  m_eventIndex;
	KernalMutexLocker    m_locker;

    int                  m_SocketID; //当前socket id

    struct KernalNetWork m_NetWorks[ MAX_NET_WORK_NUM ];
	
	std::map< pthread_t, KernalPipe* > m_WorkerPipes;
};

#endif
