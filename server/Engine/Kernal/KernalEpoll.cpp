
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
    //memset( m_ctrlfd, 0, sizeof( m_ctrlfd ) );
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

    int id =  getSocketID();
    struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
    pNetWork->init();
    pNetWork->type = KernalNetWorkType_CONNECTED;
    pNetWork->fd   = m_ctrlfd[0];
    pNetWork->id   = id;

	epollAdd( id );
	
	int status=pthread_key_create(&m_workerKey,NULL);
	
    return true;
}

int KernalEpoll::listen( const char *addr, const int port, bool isHttp )
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

    int id =  getSocketID();
    if( id > 0 )
    {
        int size      = 0;
		int offset    = 0;
        char _buf[16] = {0};
        char* dataBuf = _buf;
        NWriteInt32(dataBuf, &id);
		if( isHttp )
		{
			NWriteInt32(dataBuf, &socket_listen_http);
		}
		else
		{
			NWriteInt32(dataBuf, &socket_listen);
		}
        NWriteInt32(dataBuf, &size);
        NWriteInt32(dataBuf, &fd);
        dataBuf = _buf;
        sendMsg( m_ctrlfd[1], dataBuf, offset, size + 16, true, true );
    }
    else
    {
        ::close( fd );
    }
    return id;
}

int KernalEpoll::connect( const char *addr, const int port, int &sfd, bool isHttp, bool addToEpoll )
{
	//m_locker.lock();
	int fd = socket( AF_INET, SOCK_STREAM, 0 );

	setnonblocking( fd );
	struct sockaddr_in addrin;
	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons( port );
	inet_pton(AF_INET, addr, (void *)&addrin.sin_addr);
	int ret = ::connect( fd, ( struct sockaddr * )&addrin, sizeof(addrin) );
    int id =  getSocketID();
	sfd = fd;

	if( !( 0 == ret || ( id > 0 && -1 == ret && /*ECONNREFUSED*/EINPROGRESS == errno ) ) )
	{
        ::close( fd );

        id  = -1;
		sfd = 0;
	}
	else if ( addToEpoll && ( 0 == ret || ( id > 0 && -1 == ret && /*ECONNREFUSED*/EINPROGRESS == errno ) ) )
	{
		struct timeval tm = {2, 0};
		fd_set wset, rset;
		FD_ZERO( &wset );
		FD_ZERO( &rset );
		FD_SET( fd, &wset );
		FD_SET( fd, &rset );
		int res = ::select( fd + 1, &rset, &wset, NULL, &tm );
		if( res > 0 && FD_ISSET(fd, &wset)  )
		{
			int error, code;
			socklen_t len;
			len = sizeof(error);
			code = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
			if (code < 0 || error)
			{
				::close( fd );
				id  = -1;
				sfd = 0;
			}
		}
		
		if( id > 0 )
		{
			int size 	  = 0;
			int offset    = 0;
			char _buf[16] = {0};
			char* dataBuf = _buf;
			NWriteInt32(dataBuf, &id);
			if( isHttp )
			{
				NWriteInt32(dataBuf, &socket_connect_http);
			}
			else
			{
				NWriteInt32(dataBuf, &socket_connect);
			}
			NWriteInt32(dataBuf, &size);
			NWriteInt32(dataBuf, &fd);
			dataBuf = _buf;
			sendMsg( m_ctrlfd[1], dataBuf, offset, size + 16, true, true );
		}
	}

	//m_locker.unlock();
    return id;
}

int KernalEpoll::listenHttp( const char *addr, const int port )
{
    int id = listen( addr, port, true );
    return id;
}

int KernalEpoll::connectHttp( const char *addr, const int port )
{
	int sfd = 0;
    int id = connect( addr, port, sfd, true );
    return id;
}

bool KernalEpoll::send( int id, void *data, int size )
{
    if( id < 0 || id > MAX_NET_WORK_NUM )
    {
        return false;
    }
	KernalPipe *pPipe = getWorkerPipe();
	if( !pPipe )
	{
		return false;
	}
	
	int ssize = size + 16;
    char *_buf = ( char * )malloc( ssize );
	memset( _buf, 0, ssize );
    char* dataBuf = _buf;
	int fd 	   	  = 0;
	int offset 	  = 0;
    NWriteInt32(dataBuf, &id);
    NWriteInt32(dataBuf, &socket_data);
    NWriteInt32(dataBuf, &size);
    NWriteInt32(dataBuf, &fd);
	NWriteBit(dataBuf, data, size);
    dataBuf = _buf;
	sendMsg( pPipe->pipe[1], dataBuf, offset, ssize, true, true );
	
	free( _buf );

	return true;
}

bool KernalEpoll::sendToPipe( void *data, int size )
{
	KernalPipe *pPipe = getWorkerPipe();
	if( !pPipe )
	{
		return false;
	}
	
	int offset = 0;
	sendMsg( pPipe->pipe[1], data, offset, size, true, true );
	return true;
}

void KernalEpoll::sendHttpData( int id, const void *data, int size )
{
    char buff[ HTTP_BUFFER_SIZE ];
    memset( buff, 0, HTTP_BUFFER_SIZE );
    sprintf( buff, "GET / HTTP/1.1\r\nAccept: text/html\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length:%d\n\n%s\r\n", size, (char*)data);

    send( id, buff, HTTP_BUFFER_SIZE );
}

int KernalEpoll::readMsg( int fd, void *data, int &readOffset, bool useRead, bool readAll )
{
    int ret 	 = 0;
    int readSize = 0;
	int size = RECV_BUFFER_SIZE - readOffset;
    do
    {
        if( useRead )
        {
            ret = ::read( fd, (char*)data + readOffset, size - readOffset );
        }
        else
        {
            ret = ::recv( fd, (char*)data + readOffset, size - readOffset, 0 );
        }
		
		if( ret == -1 && errno == EWOULDBLOCK )
		{
            break;
		}
		
        if(  ret == -1 && ( errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) )
        {
			if( readAll )
			{
				continue;				
			}
			break;
        }

        if( ret <= 0 )
        {
            break;
        }

        readOffset += ret;
		readSize += ret;
		if( !readAll )
		{
			break;				
		}
    }while( readOffset < size );

	if( ret > 0 && readSize > 0 )
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
		
		if( ret == -1 && errno == EWOULDBLOCK )
		{
			break;
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

int KernalEpoll::sendMsg( int fd, const void *data, int &offSet, int size, bool useWrite, bool sendAll )
{
    int ret = 0;
    do
    {
        if( useWrite )
        {
            ret = ::write( fd, (char*)data + offSet, size - offSet );
        }
        else
        {
            ret = ::send( fd, (char*)data + offSet, size - offSet, 0 );
        }

        if(  ret == -1 && ( errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN ) )
        {
			if( sendAll )
			{
				continue;				
			}
			break;
        }

        if( ret <= 0 )
        {
            break;
        }
		
        if( ret > 0 )
        {
            offSet += ret;
        }
		
		if( !sendAll )
		{
			break;
		}
    }while( offSet < size );

    return ret;
}

// TODO: 待优化
KernalSocketMessageType KernalEpoll::handleMessage( KernalRequestMsg &result, int expire )
{
    result.init();
    if( m_eventNum == m_eventIndex )
    {
        m_eventNum = epoll_wait( m_epollfd, m_events, MAX_EVENTS, expire );
        m_eventIndex = 0;
		if( m_eventNum <= 0)
    	{
	    	m_eventNum = 0;
	    	return KernalSocketMessageType_NO;
		}

        for ( int i = 0; i < m_eventNum; i++ )
        {
            struct epoll_event *pEvent = &m_events[ i ];
            if( !pEvent->data.ptr )
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

    if( checkIsPipe( pNetWork->fd ) && pNetWork->isRead )
    {
        KernalSocketMessageType msgType = KernalSocketMessageType_NO;
        if( pNetWork->readBuffersLen < 16 )
        {
            int ret = readMsg( pNetWork->fd, pNetWork->readBuffers, pNetWork->readBuffersLen, true, false );
        }
        else
        {
            int size = *( (int*)(pNetWork->readBuffers + 8) );
            if( size > pNetWork->readBuffersLen - 16 )
            {
                int ret = readMsg( pNetWork->fd, pNetWork->readBuffers, pNetWork->readBuffersLen, true, false );
            }
        }

        if( pNetWork->readBuffersLen >= 16 )
        {
	    	int fd = *( (int*)(pNetWork->readBuffers + 12) );
            int size = *( (int*)(pNetWork->readBuffers + 8) );
            int type = *( (int*)(pNetWork->readBuffers + 4) );
            int id = *( (int*)(pNetWork->readBuffers) );
			result.id = id;

            if( pNetWork->readBuffersLen - 16 >= size )
            {
                struct KernalNetWork *pNet = &m_NetWorks[ HASH_ID( id ) ];
				
                if( type == socket_close )  // 关闭连接
                {
                    if( KernalNetWorkType_NO != pNet->type )
                    {
                        closeSocket( id );
                        msgType = KernalSocketMessageType_SOCKET_CLOSE;
                    }
                }
                else if( type == socket_connect || type == socket_connect_http )
                {
                    pNet->init();

                    pNet->type = KernalNetWorkType_CONNECTED;
					if( type == socket_connect_http )
					{
						pNet->type = KernalNetWorkType_CONNECTED_HTTP;
					}
                    pNet->fd   = fd;
                    pNet->id   = id;

                    setnonblocking( fd );
                    epollAdd( id );
					
					if( type == socket_connect )
					{
						msgType = KernalSocketMessageType_SOCKET_CONNECT;
					}
                }
                else if( type == socket_listen || type == socket_listen_http )
                {
                    pNet->type = KernalNetWorkType_LISTEN;
					if( type == socket_listen_http )
					{
						pNet->type = KernalNetWorkType_LISTEN_HTTP;
					}
                    pNet->fd = fd;
                    pNet->id = id;

                    setnonblocking( fd );
                    epollAdd( id );
                }
                else if( pNet->id == id )  // 发送数据
                {
                    if( KernalNetWorkType_CONNECTED == pNet->type || KernalNetWorkType_CONNECTED_HTTP == pNet->type )
                    {
						if( KernalNetWorkType_CONNECTED == pNet->type )
						{
							size += 4;
						}
                        void *buffer = malloc( size );
                        memset( buffer, 0, size );
						if( KernalNetWorkType_CONNECTED == pNet->type )
						{
							int datalen = size - 4;
							memcpy( buffer, &datalen, 4);
							memcpy( (char*)buffer + 4, pNetWork->readBuffers + 16, datalen );							
						}
						else
						{
							memcpy( buffer, pNetWork->readBuffers + 16, size );
						}
                        pNet->buffers.appendBuffer( buffer, size );
                        epollMod( pNet->fd, EPOLLOUT, pNet );
                        pNet->isWrite = true;
						
						if( KernalNetWorkType_CONNECTED == pNet->type )
						{
							size -= 4;
						}
                    }
                }
				else if( pNet->id != id ) // 连接已经关闭
				{
                    msgType = KernalSocketMessageType_SOCKET_CLOSE;
				}

                pNetWork->readBuffersLen -= size + 16;
                memmove( pNetWork->readBuffers, pNetWork->readBuffers + size + 16, pNetWork->readBuffersLen );
            }
        }

        if( pNetWork->readBuffersLen < 16 )
        {
            pNetWork->isRead = false;
        }
        else
        {
            int size = *( (int*)(pNetWork->readBuffers + 8) );
            if( size > 0 && size >= pNetWork->readBuffersLen - 16 )
            {
                pNetWork->isRead = false;
            }
        }
		
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
        int id = getSocketID();

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

    if( pNetWork->isRead ) // 接收数据
    {
        pNetWork->isRead = false;
        msgType = KernalSocketMessageType_NO;
        result.id = pNetWork->id;
        result.netType = pNetWork->type;

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
			ret = readMsg( pNetWork->fd, pNetWork->readBuffers, pNetWork->readBuffersLen, false, false );
			
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

        if( 0 == ret )
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

    if( pNetWork->isWrite ) //发送数据
    {
        pNetWork->isWrite = false;
        result.id = pNetWork->id;

        while( pNetWork->buffers.head )
        {
            msgType = KernalSocketMessageType_NO;
            struct KernalNetWorkBuffer *tmp = pNetWork->buffers.head;
            int ret = 0;
		
			ret = sendMsg( pNetWork->fd, tmp->data, tmp->offset, tmp->size );

			if( tmp->offset >= tmp->size )
			{
				pNetWork->buffers.head = tmp->next;
				free(tmp->data);
				free(tmp);
			}

            if( 0 == ret )
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
			
			break;
        }
		
		// 如果未发送完
		if( pNetWork->buffers.head )
		{
			epollMod( pNetWork->fd, EPOLLOUT, pNetWork );
		}
		else
		{
			epollMod( pNetWork->fd, 0, pNetWork );			
		}
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
        if( !checkIsPipe( pNetWork->fd ) && pNetWork->type != KernalNetWorkType_NO )
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
	KernalPipe *pPipe = getWorkerPipe();
	if( !pPipe )
	{
		return;
	}
	
    int size = 0;
	int offset = 0;
    char _buf[16] = {0};
    char* dataBuf = _buf;
	int fd = 0;
    NWriteInt32(dataBuf, &id);
    NWriteInt32(dataBuf, &socket_close);
    NWriteInt32(dataBuf, &size);
    NWriteInt32(dataBuf, &fd);
    dataBuf = _buf;
	sendMsg( pPipe->pipe[1], dataBuf, offset, size + 16, true, true );
}

int KernalEpoll::getSocketID()
{
	m_locker.lock();
    for( int i = 0; i < MAX_NET_WORK_NUM; ++i )
    {
        int id = ++m_SocketID;
        if( id < 0 )
        {
            id = 0;
        }

        struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
        if( KernalNetWorkType_NO == pNetWork->type )
        {
			m_locker.unlock();
            return id;
        }
        else
        {
            --i;
        }
    }
	m_locker.unlock();
    return -1;
}

// 创建工作线程管道
KernalPipe *KernalEpoll::createWorkerPipe( pthread_t tid )
{
	KernalPipe *pPipe = new KernalPipe();
	pPipe->tid = tid; 
	if( ::socketpair( AF_UNIX, SOCK_STREAM, 0, pPipe->pipe ) )
    {
		delete pPipe;
		pPipe = NULL;
    }
	
	if( pPipe )
	{
		int id =  getSocketID();
		struct KernalNetWork *pNetWork = &m_NetWorks[ HASH_ID( id ) ];
		pNetWork->init();
		pNetWork->type = KernalNetWorkType_CONNECTED;
		pNetWork->fd   = pPipe->pipe[0];
		pNetWork->id   = id;
		epollAdd( id );
		
		m_WorkerPipes.insert( std::pair< pthread_t, KernalPipe* >( tid, pPipe ) ); 
	}
	return pPipe;
}

KernalPipe *KernalEpoll::getWorkerPipe()
{
	int *workerarg = (int*)pthread_getspecific( m_workerKey );
	auto iter = m_WorkerPipes.find( *workerarg );
	
	if( iter != m_WorkerPipes.end() )
	{
		return iter->second;
	}
	return NULL;
}

bool KernalEpoll::checkIsPipe( int fd )
{
	if( fd == m_ctrlfd[0] )
	{
		return true;
	}
	for( auto iter = m_WorkerPipes.begin(); iter != m_WorkerPipes.end(); ++iter )
	{
		if( iter->second->pipe[0] == fd )
		{
			return true;
		}
	}
	return false;
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
        if( pNetWork->id != 0 && pNetWork->fd != 0 && !checkIsPipe( pNetWork->fd ) && pNetWork->type == KernalNetWorkType_CONNECTED )
        {
            int ret = ::send( pNetWork->fd, buff, 4, 0 );
            if( ret <= 0 )
            {
                this->close( pNetWork->id );
            }
        }
    }
}
