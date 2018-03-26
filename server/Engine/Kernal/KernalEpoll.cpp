
#include "KernalEpoll.h"
#include <sys/time.h>

bool setnonblocking(int sock)
{
	int opts;
	opts = fcntl( sock, F_GETFL );
	if( opts < 0 )
	{
		return false;
	}

	opts = opts | O_NONBLOCK;
	if(fcntl( sock, F_SETFL, opts )<0)
	{
		return false;
	}
	return true;
}

KernalEpoll::KernalEpoll()
    :m_epollfd( -1 )
    ,m_eventNum( 0 )
    ,m_eventIndex( 0 )
    ,m_SocketID( 0 )
{
    memset( m_ctrlfd, 0, sizeof( m_ctrlfd ) );
}

KernalEpoll::~KernalEpoll()
{

}

bool KernalEpoll::create()
{
	m_epollfd = epoll_create( 1024 );
    if( -1 == m_epollfd )
    {
      return false;
    }

    if( pipe( m_ctrlfd ) )
    {
        release();
        return false;
    }

    int id = m_ctrlfd[0]; //getSocketID();
    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
    pNetWork->init();
    pNetWork->type = KernalNetWorkType_CONNECTED;
    pNetWork->fd   = m_ctrlfd[0];
    pNetWork->id   = id;

	epollAdd( id );
    return true;
}

int KernalEpoll::listen( const char *addr, const int port )
{
	int fd = socket( AF_INET, SOCK_STREAM, 0 );

	int flag = 1;
	setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int) );
    flag = 1;
	setsockopt( fd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(int) );

	struct sockaddr_in addrin;
	memset( &addrin, 0, sizeof(addrin) );
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons( port );
	inet_pton( AF_INET, addr, (void *)&addrin.sin_addr );
	int ret = bind( fd, ( struct sockaddr * )&addrin, sizeof(addrin) );
	ret = ::listen( fd, 5 );

    int id = fd; //getSocketID();
    if( id > 0 )
    {
#if 1
        struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
        pNetWork->init();
        pNetWork->type = KernalNetWorkType_LISTEN;
        pNetWork->fd   = fd;
        pNetWork->id   = id;

	    setnonblocking( fd );
        epollAdd( id );
#else
        int size = 0;
        char _buf[BUFF_SIZE] = {0};
        char* dataBuf = _buf;
        WriteInt32(dataBuf, &id);
        WriteInt32(dataBuf, &socket_listen);
        WriteInt32(dataBuf, &size);
        dataBuf = _buf;
        sendMsg( m_ctrlfd[1], dataBuf, size + 12, true );
#endif

    }
    else
    {
        ::close( fd );
    }
    return id;
}

int KernalEpoll::connect( const char *addr, const int port, bool addToEpoll )
{
	int fd = socket( AF_INET, SOCK_STREAM, 0 );

	setnonblocking( fd );
	struct sockaddr_in addrin;
	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons( port );
	inet_pton(AF_INET, addr, (void *)&addrin.sin_addr);
	int ret = ::connect( fd, ( struct sockaddr * )&addrin, sizeof(addrin) );
    int id = fd; //getSocketID();
#if 0
    if( 0 == ret || ( id > 0 && -1 == ret && /*ECONNREFUSED*/EINPROGRESS == errno ) )
    {
#if 0
        struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
        pNetWork->init();

        pNetWork->type = KernalNetWorkType_CONNECTED;
        pNetWork->fd   = fd;
        pNetWork->id   = id;

        epollAdd( id );
#else
        int size = 0;
        char _buf[BUFF_SIZE] = {0};
        char* dataBuf = _buf;
        WriteInt32(dataBuf, &id);
        WriteInt32(dataBuf, &socket_connect);
        WriteInt32(dataBuf, &size);
        dataBuf = _buf;
        sendMsg( m_ctrlfd[1], dataBuf, size + 12, true );
#endif

    }
    else
    {
        ::close( fd );

        id = -1;
    }
#endif
	if( !( 0 == ret || ( id > 0 && -1 == ret && /*ECONNREFUSED*/EINPROGRESS == errno ) ) )
	{
        ::close( fd );

        id = -1;
	}
	else if ( addToEpoll && ( 0 == ret || ( id > 0 && -1 == ret && /*ECONNREFUSED*/EINPROGRESS == errno ) ) )
	{
	    int size = 0;
	    char _buf[12] = {0};
	    char* dataBuf = _buf;
	    WriteInt32(dataBuf, &id);
	    WriteInt32(dataBuf, &socket_connect);
	    WriteInt32(dataBuf, &size);
	    dataBuf = _buf;
	    sendMsg( m_ctrlfd[1], dataBuf, size + 12, true );
	}

    return id;
}

#if 0
int KernalEpoll::connectSocket( const char *addr, const int port )
{
	int fd = socket( AF_INET, SOCK_STREAM, 0 );

	setnonblocking( fd );
	struct sockaddr_in addrin;
	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons( port );
	inet_pton(AF_INET, addr, (void *)&addrin.sin_addr);
	int ret = ::connect( fd, ( struct sockaddr * )&addrin, sizeof(addrin) );
    int id = fd; //getSocketID();
    if( !( 0 == ret || ( id > 0 && -1 == ret && /*ECONNREFUSED*/EINPROGRESS == errno ) ) )
    {
        ::close( fd );

        id = -1;
    }

    return id;
}
#endif

int KernalEpoll::listenHttp( const char *addr, const int port )
{
    int id = listen( addr, port );
    if( id > 0 )
    {
        struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
        pNetWork->type = KernalNetWorkType_LISTEN_HTTP;
    }
    return id;
}

int KernalEpoll::connectHttp( const char *addr, const int port )
{
    int id = connect( addr, port );
    if( id > 0 )
    {
        struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
        pNetWork->type = KernalNetWorkType_CONNECTED_HTTP;
    }
    return id;
}

bool KernalEpoll::send( int id, void *data, int size )
{
    if( id < 0 || id >= MAX_NET_WORK_NUM )
    {
        return false;
    }
    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
    if( KernalNetWorkType_NO == pNetWork->type )
    {
        if( data )
        {
            //free( data );
            //data = NULL;
        }
        return false;
    }

    char *buffer = (char*)malloc( size + 4 );
    memset( buffer, 0, size + 4 );
    char *dataBuf = buffer;
    int ret = 0;
    if( KernalNetWorkType_CONNECTED_HTTP != pNetWork->type )
    {
        WriteInt32(dataBuf, &size);
    }
	WriteBit(dataBuf,data,size);
    ret = sendMsg( pNetWork->fd, buffer, dataBuf - buffer );
    free( buffer );

    if( KernalNetWorkType_CONNECTED_HTTP == pNetWork->type )
    {
        closeSocket( pNetWork->id );
    }

    if( KernalNetWorkType_CONNECTED_HTTP != pNetWork->type && ret <= 0 )
    {
        close( pNetWork->id );
    }

    return ( ret > 0 );
}

bool KernalEpoll::sendToPipe( void *data, int size )
{
	sendMsg( m_ctrlfd[1], data, size, true );
	return true;
}

void KernalEpoll::sendHttpData( int id, const void *data, int size )
{
    char buff[ HTTP_BUFFER_SIZE ];
    memset( buff, 0, HTTP_BUFFER_SIZE );
    sprintf( buff, "GET / HTTP/1.1\r\nAccept: text/html\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length:%d\n\n%s\r\n", size, (char*)data);

    send( id, buff, HTTP_BUFFER_SIZE );
}

int KernalEpoll::readMsg( int fd, void *data, int size, bool useRead, bool readOnce )
{
    int ret = 0;
    int readSize = 0;
    do
    {
        if( useRead )
        {
            ret = ::read( fd, (char*)data + readSize, size - readSize );
        }
        else
        {
            ret = ::recv( fd, (char*)data + readSize, size - readSize, 0 );
        }
        if(  ret == -1 && ( errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) )
        {
            continue;
        }

        if( ret <= 0 )
        {
            break;
        }

        readSize += ret;

        if( readOnce )
        {
            break;
        }
    }while( readSize < size );

    if( ret > 0 )
    {
        ret = readSize;
    }
    return ret;
}

int KernalEpoll::readHttpMsg( int fd, void *data, int size )
{
    int readSize = 0;
    do
    {
        int ret = ::recv( fd, (char*)data + readSize, size - readSize, 0 );
        if(  ret == -1 && ( errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) )
        {
            continue;
        }

        if( ret <= 0 )
        {
            break;
        }

        readSize += ret;

    }while( false );

    if( 0 != memcmp( data, "GET", 3 ) && 0 != memcmp( data, "POST", 4 ) )
    {
				readSize = 0;
    }

    if( readSize > 0 )
    {
        std::string str;
        str.assign( (char*)data, readSize );
        int pos = str.find("?");
        if( pos > 0 )
        {
            int endpos = str.find(" ", pos);
            readSize = endpos - pos - 1;
            std::string sstr = str.substr( pos + 1, readSize );
            memset( data, 0, size );
            memcpy( data, sstr.c_str(), readSize );
        }
    }
    return readSize;
}

int KernalEpoll::sendMsg( int fd, const void *data, int size, bool useWrite )
{
    int ret = 0;
    int sendSize = 0;
    do
    {
        if( useWrite )
        {
            ret = ::write( fd, (char*)data + sendSize, size - sendSize );
        }
        else
        {
            ret = ::send( fd, (char*)data + sendSize, size - sendSize, 0 );
        }

        if(  ret == -1 && ( errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN ) )
        {
            continue;
        }

        if( ret <= 0 )
        {
            break;
        }
        if( ret > 0 )
        {
            sendSize += ret;
        }

    }while( sendSize < size );

    if( ret > 0 )
    {
        ret = size;
    }
    return ret;
}

KernalSocketMessageType KernalEpoll::handleMessage( KernalRequestMsg &result )
{
    result.init();
    if( m_eventNum == m_eventIndex )
    {
        m_eventNum = epoll_wait( m_epollfd, m_events, MAX_EVENTS, -1 );
        m_eventIndex = 0;
		if( m_eventNum <= 0)
    	{
		    m_eventNum = 0;
		    return KernalSocketMessageType_NO;
		}

        for ( int i = 0; i < m_eventNum; i++ )
        {
            struct epoll_event *pEvent = &m_events[ i ];
            if( !pEvent->data.ptr /*|| pEvent->data.fd == m_ctrlfd[0]*/ )
            {
                continue;
            }
            struct KernalNetWork *pNetWork = ( struct KernalNetWork * )(pEvent->data.ptr);
            if( !pNetWork )
            {
                continue;
            }

            unsigned flag = pEvent->events;
            pNetWork->isWrite = (flag & EPOLLOUT) == EPOLLOUT;
            pNetWork->isRead  = (flag & EPOLLIN) == EPOLLIN;

        }
    }

    struct epoll_event *pEvent = &m_events[ m_eventIndex++ ];
    struct KernalNetWork *pNetWork = ( struct KernalNetWork * )(pEvent->data.ptr);
    if( !pNetWork )
    {
        return KernalSocketMessageType_NO;
    }

    if( pEvent->events & EPOLLHUP )
    {
        epollDel( pNetWork->id );
        ::close( pNetWork->fd );
        return KernalSocketMessageType_NO;
    }

    if(  pNetWork->fd == m_ctrlfd[0] && pNetWork->isRead )
    {
        KernalSocketMessageType msgType = KernalSocketMessageType_NO;
        if( pNetWork->readBuffersLen < 12 )
        {
            int ret = readMsg( pNetWork->fd, pNetWork->readBuffers + pNetWork->readBuffersLen, RECV_BUFFER_SIZE - pNetWork->readBuffersLen, true, true );
            if( ret > 0 )
            {
                pNetWork->readBuffersLen += ret;
            }
        }
        else
        {
            int size = *( (int*)(pNetWork->readBuffers + 8) );
            if( size > pNetWork->readBuffersLen - 12 )
            {
                int ret = readMsg( pNetWork->fd, pNetWork->readBuffers + pNetWork->readBuffersLen, RECV_BUFFER_SIZE - pNetWork->readBuffersLen, true, true );
                if( ret > 0 )
                {
                    pNetWork->readBuffersLen += ret;
                }
            }
        }

        if( pNetWork->readBuffersLen >= 12 )
        {
            int size = *( (int*)(pNetWork->readBuffers + 8) );
            int type = *( (int*)(pNetWork->readBuffers + 4) );
            int id = *( (int*)(pNetWork->readBuffers) );
            if( pNetWork->readBuffersLen - 12 >= size )
            {
                struct KernalNetWork *pNet = &m_NetWorks[ HASH_ID( id ) ];
                if( type == socket_close )  // 关闭连接
                {
                    if( KernalNetWorkType_NO != pNet->type )
                    {
                        closeSocket( id );
                        result.id = id;
                        msgType = KernalSocketMessageType_SOCKET_CLOSE;
                    }
                }
                else if( type == socket_connect )
                {
                    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
                    pNetWork->init();

                    pNetWork->type = KernalNetWorkType_CONNECTED;
                    pNetWork->fd   = id;
                    pNetWork->id   = id;

                    setnonblocking( id );
                    epollAdd( id );
                }
                else if( type == socket_listen )
                {
                    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
                    pNetWork->type = KernalNetWorkType_LISTEN;
                    pNetWork->fd   = id;
                    pNetWork->id   = id;

                    setnonblocking( id );
                    epollAdd( id );
                }
                else  // 发送数据
                {
                    if( KernalNetWorkType_CONNECTED == pNet->type || KernalNetWorkType_CONNECTED_HTTP == pNet->type )
                    {
                        void *buffer = malloc( size );
                        memset( buffer, 0, size );
                        memcpy( buffer, pNetWork->readBuffers + 12, size );
                        pNet->buffers.appendBuffer( buffer, size );
                        epollMod( pNet->fd, EPOLLOUT, pNet );
                        pNet->isWrite = true;
                    }
                }

                pNetWork->readBuffersLen -= size + 12;
                memmove( pNetWork->readBuffers, pNetWork->readBuffers + size + 12, pNetWork->readBuffersLen );
            }
        }
#if 0
        int ret = readMsg( pNetWork->fd, pNetWork->readBuffers + pNetWork->readBuffersLen, RECV_BUFFER_SIZE - pNetWork->readBuffersLen, true, true );
        if( ret > 0 )
        {
            pNetWork->readBuffersLen += ret;
        }

        while( true )
        {
            if( pNetWork->readBuffersLen >= 12 )
            {
                int size = *( (int*)(pNetWork->readBuffers + 8) );
                int type = *( (int*)(pNetWork->readBuffers + 4) );
                int id = *( (int*)(pNetWork->readBuffers) );
                if( size <= pNetWork->readBuffersLen - 12 )
                {
                    struct KernalNetWork *pNet = &m_NetWorks[ HASH_ID( id ) ];
                    if( type == socket_close )  // 关闭连接
                    {
                        closeSocket( id );
                    }
                    else  // 发送数据
                    {
                        if( KernalNetWorkType_CONNECTED == pNet->type || KernalNetWorkType_CONNECTED_HTTP == pNet->type )
                        {
                            void *buffer = malloc( size );
                            memset( buffer, 0, size );
                            memcpy( buffer, pNetWork->readBuffers + 12, size );
                            pNet->buffers.appendBuffer( buffer, size );
                            epollMod( pNet->fd, EPOLLOUT, pNet );
                            pNet->isWrite = true;
                        }

                        pNetWork->readBuffersLen -= size + 12;
                        memmove( pNetWork->readBuffers, pNetWork->readBuffers + size + 12, pNetWork->readBuffersLen );
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
#endif

#if 1
        if( pNetWork->readBuffersLen < 12 )
        {
            pNetWork->isRead = false;
        }
        else if( socket_data == msgType )
        {
            int size = *( (int*)(pNetWork->readBuffers + 8) );
            if( size > 0 && size > pNetWork->readBuffersLen - 12 )
            {
                pNetWork->isRead = false;
            }
        }
#endif
        if( pNetWork->isRead )
        {
            --m_eventIndex;
        }
        return msgType;
    }

    if( KernalNetWorkType_LISTEN == pNetWork->type ||  KernalNetWorkType_LISTEN_HTTP == pNetWork->type ) // 有新连接
    {
	    struct sockaddr_in addr;
	    socklen_t addrLen = sizeof( addr );
        int fd = accept( pNetWork->fd, (struct sockaddr *)&addr, &addrLen );
        int id = fd; //getSocketID();

        if( id > 0 )
        {
            struct KernalNetWork *pClientNetWork = &m_NetWorks[ HASH_ID( id ) ];
            pClientNetWork->init();

            pClientNetWork->type = KernalNetWorkType_CONNECTED;
            if( KernalNetWorkType_LISTEN_HTTP == pNetWork->type )
            {
                pClientNetWork->type = KernalNetWorkType_CONNECTED_HTTP;
            }

            pClientNetWork->fd   = fd;
            pClientNetWork->id   = id;

            struct timeval timeout = {3, 0};
            setsockopt( fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval) );

            setnonblocking( fd );
            epollAdd( id );
        }
        else
        {
            ::close( fd );
        }
        return KernalSocketMessageType_NO;
    }
    else if( KernalNetWorkType_NO == pNetWork->type )
    {
        closeSocket( pNetWork->id );
        return KernalSocketMessageType_NO;
    }
    KernalSocketMessageType msgType = KernalSocketMessageType_NO;

    if( pNetWork->isRead /*pEvent->events & EPOLLIN*/ ) // 接收数据
    {
        pNetWork->isRead = false;
        msgType = KernalSocketMessageType_NO;
        result.id = pNetWork->id;

        int ret = -1;
        if( KernalNetWorkType_CONNECTED_HTTP == pNetWork->type )
        {
            ret = readHttpMsg( pNetWork->fd, pNetWork->readBuffers + pNetWork->readBuffersLen, RECV_BUFFER_SIZE - pNetWork->readBuffersLen );

            if( ret > 0 )
            {
                pNetWork->readBuffersLen += ret;

                if( pNetWork->readBuffersLen > 0 )
                {
                    result.size = pNetWork->readBuffersLen;
                    result.data = ( char* )malloc( pNetWork->readBuffersLen );
                    memset( result.data, 0, pNetWork->readBuffersLen );
                    memcpy( result.data, pNetWork->readBuffers, pNetWork->readBuffersLen );

                    memmove( pNetWork->readBuffers, pNetWork->readBuffers + pNetWork->readBuffersLen, pNetWork->readBuffersLen - pNetWork->readBuffersLen );
                    pNetWork->readBuffersLen = 0;

                    msgType = KernalSocketMessageType_SOCKET_DATA;
                }

            }

        }
        else
        {
            if( pNetWork->readBuffersLen > 0 )
            {
                int size = *( (int*)(pNetWork->readBuffers) );
                if( size > pNetWork->readBuffersLen - 4 )
                {
                    ret = readMsg( pNetWork->fd, pNetWork->readBuffers + pNetWork->readBuffersLen, RECV_BUFFER_SIZE - pNetWork->readBuffersLen, false, true );
                    if( ret > 0 )
                    {
                        pNetWork->readBuffersLen += ret;
                    }
                }

                ret = pNetWork->readBuffersLen;
            }
            else
            {
                ret = readMsg( pNetWork->fd, pNetWork->readBuffers + pNetWork->readBuffersLen, RECV_BUFFER_SIZE - pNetWork->readBuffersLen, false, true );
                if( ret > 0 )
                {
                    pNetWork->readBuffersLen += ret;
                }
            }

            if( ret > 0 )
            {
                int size = *( (int*)(pNetWork->readBuffers) );
                if( pNetWork->readBuffersLen >= size + 4 )
                {
                    // 心跳不处理
                    if( size > 0 && !( 4 == size && pNetWork->type == KernalNetWorkType_CONNECTED && *( (int*)(pNetWork->readBuffers + 4 ) ) == 0 ) )
                    {
                        result.size = size;
                        result.data = ( char* )malloc( size );
                        memset( result.data, 0, size );
                        memcpy( result.data, pNetWork->readBuffers + 4, size );
                        msgType = KernalSocketMessageType_SOCKET_DATA;
                    }
                    pNetWork->readBuffersLen -= size + 4;
                    memmove( pNetWork->readBuffers, pNetWork->readBuffers + size + 4, pNetWork->readBuffersLen );

                }

            }

        }
        if( pNetWork->readBuffersLen > 0 )
        {
            pNetWork->isRead = true;
        }

        if( 0 == ret /*&& 0 != errno*/ )
        {
            closeSocket( pNetWork->id );
            msgType = KernalSocketMessageType_SOCKET_CLOSE;
        }
        else if( -1 == ret && errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN )
        {
            closeSocket( pNetWork->id );
            msgType = KernalSocketMessageType_SOCKET_CLOSE;
        }
        if( KernalNetWorkType_NO != pNetWork->type && ( pNetWork->isWrite || pNetWork->isRead ) )
        {
            --m_eventIndex;
        }
        return msgType;
    }

    if( pNetWork->isWrite /*pEvent->events & EPOLLOUT*/ ) //发送数据
    {
        pNetWork->isWrite = false;
        result.id = pNetWork->id;

        while( pNetWork->buffers.head )
        {
            msgType = KernalSocketMessageType_NO;
            struct KernalNetWorkBuffer *tmp = pNetWork->buffers.head;
            int ret = 0;
            if( KernalNetWorkType_CONNECTED_HTTP != pNetWork->type )
            {
                ret = sendMsg( pNetWork->fd, &tmp->size, 4 );
            }
            ret = sendMsg( pNetWork->fd, tmp->data, tmp->size );

            pNetWork->buffers.head = tmp->next;
            free(tmp->data);
            free(tmp);

            if( 0 == ret /*&& 0 != errno*/ )
            {
                closeSocket( pNetWork->id );
                msgType = KernalSocketMessageType_SOCKET_CLOSE;
                break;
            }
            else if( -1 == ret && errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN )
            {
                closeSocket( pNetWork->id );
                msgType = KernalSocketMessageType_SOCKET_CLOSE;
                break;
            }
        }
        epollMod( pNetWork->fd, 0, pNetWork );
        if( KernalNetWorkType_CONNECTED_HTTP == pNetWork->type )
        {
            closeSocket( pNetWork->id );
            msgType = KernalSocketMessageType_SOCKET_CLOSE;
        }
        if( KernalNetWorkType_NO != pNetWork->type && ( pNetWork->isWrite || pNetWork->isRead ) )
        {
            --m_eventIndex;
        }
        return msgType;
    }

    return msgType;
}

void KernalEpoll::epollAdd( int id, bool isET )
{
    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
    epollAdd( pNetWork->fd, pNetWork, isET );
}

void KernalEpoll::epollAdd( int fd, void *data, bool isET )
{
	struct epoll_event event;
	event.data.fd  = fd;
	if( data )
	{
		event.data.ptr = data;
	}
	event.events  = EPOLLIN | ( isET ? EPOLLET : 0 );
	epoll_ctl( m_epollfd, EPOLL_CTL_ADD, fd, &event );

}

void KernalEpoll::epollMod( int fd, int op, void *data, bool isET )
{
	struct epoll_event event;
	event.data.fd  = fd;
	if( data )
	{
		event.data.ptr = data;
	}
	event.events  = EPOLLIN | op | ( isET ? EPOLLET : 0 );
	epoll_ctl( m_epollfd, EPOLL_CTL_MOD, fd, &event );
}

void KernalEpoll::epollDel( int id )
{
    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
	epoll_ctl( m_epollfd, EPOLL_CTL_DEL, pNetWork->fd , NULL );
}

void KernalEpoll::release()
{
    for( int i = 0; i < MAX_NET_WORK_NUM; ++i )
    {
        struct KernalNetWork *pNetWork = &m_NetWorks[ i ];
        if( pNetWork->id != m_ctrlfd[0] && pNetWork->type != KernalNetWorkType_NO )
        {
            closeSocket( pNetWork->id );
        }
    }

    ::close( m_epollfd );
	::close( m_ctrlfd[0] );
	::close( m_ctrlfd[1] );
}

void KernalEpoll::closeSocket( int id )
{
    epollDel( id );
    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
    pNetWork->type = KernalNetWorkType_NO;
    ::close( pNetWork->fd );

    while( pNetWork->buffers.head )
    {
        struct KernalNetWorkBuffer *tmp = pNetWork->buffers.head;
        pNetWork->buffers.head = tmp->next;
        free(tmp->data);
        free(tmp);
    }

    pNetWork->clear();
}

void KernalEpoll::close( int id )
{
    if( id < 0 )
    {
        return;
    }
    int size = 0;
    char _buf[12] = {0};
    char* dataBuf = _buf;
    WriteInt32(dataBuf, &id);
    WriteInt32(dataBuf, &socket_close);
    WriteInt32(dataBuf, &size);
    dataBuf = _buf;
    sendMsg( m_ctrlfd[1], dataBuf, size + 12, true );
}

int KernalEpoll::getSocketID()
{
    for( int i = 0; i < MAX_NET_WORK_NUM; ++i )
    {
        int id = ++m_SocketID;
        //int id = __sync_add_and_fetch(&m_SocketID, 1);
        if( id < 0 )
        {
            id = 0;
            //id = __sync_and_and_fetch(&m_SocketID, 0)
        }

        struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
        if( KernalNetWorkType_NO == pNetWork->type )
        {
            return id;
        }
        else
        {
            --i;
        }
    }
    return -1;
}

struct KernalNetWork *KernalEpoll::getNetWork( int id )
{
    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
    return pNetWork;
}

void KernalEpoll::heartbeat()
{
	return;
    char buff[4];
    memset( buff, 0, 4 );
    int msgType = 0;
    memcpy( buff, &msgType, 4 );

    for( int i = 0; i < MAX_NET_WORK_NUM; ++i )
    {
        struct KernalNetWork *pNetWork = &m_NetWorks[ i ];
        if( pNetWork->id != 0 && pNetWork->fd != 0 && pNetWork->id != m_ctrlfd[0] && pNetWork->type == KernalNetWorkType_CONNECTED )
        {
            int ret = ::send( pNetWork->fd, buff, 4, 0 );
            if( ret <= 0 )
            {
                this->close( pNetWork->id );
            }
        }
    }
}