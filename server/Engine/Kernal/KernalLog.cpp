
#include "KernalLog.h"
#include <stdarg.h>

#define LOG_BUFF_LINE_SIZE 1024  // 每一行长度

void *KernalLogWorker( void *arg )
{
	KernalLog *pLog = (KernalLog*)arg;
	pLog->logWorker();
	return ((void *)0);
}

KernalLog::KernalLog()
{
	memset( m_pLogFilePath, 0, sizeof( m_pLogFilePath ) );
	memset( m_logPipe, 0, sizeof( m_logPipe ) );
}

KernalLog::~KernalLog()
{

}

void KernalLog::init( char *pLogFilePath )
{
	memccpy( m_pLogFilePath, pLogFilePath, strlen( pLogFilePath ) );
		
    if( pipe( m_logPipe ) )
    {
        return false;
    }
	
	m_LogThread.init(KernalServerBaseWorker, this);
	m_LogThread.detach();
		
	return true;
}

void KernalLog::info( const char* pLogText, ... )
{
	va_list args;
    va_start( args, pLogText );
    char logbuff[ LOG_BUFF_LINE_SIZE ];
	memset( logbuff, 0, sizeof(logbuff) );
	vsprintf( logbuff, pLogText, args );
    va_end( args );
	printf("KernalLog::info: %s \n\r", logbuff);
	
	::write( m_logPipe[1], logbuff, strlen( logbuff ) );
}

void KernalLog::logWorker()
{
	m_logFd = open( m_pLogFilePath, O_RDWR | O_CREAT );
	char logbuff[ LOG_BUFF_LINE_SIZE ];
	int retval = 0;
	int ret =  0;
	while( !m_quit )
	{
		fd_set rset;
		FD_ZERO( &rset );
		FD_SET( m_logPipe[0], &rset );
		retval = ::select( m_logPipe[0] + 1, &rset, NULL, NULL, NULL );
		if( retval > 0 && FD_ISSET( m_logPipe[0], &rset )  )
		{
			memset( logbuff, 0, sizeof(logbuff) );
			ret = ::read( m_logPipe[0], logbuff, sizeof(logbuff) );
			if( ret > 0 )
			{
				::lseek( m_logFd, 0, SEEK_END );
				::write( m_logFd, logbuff, ret );
			}
		}
	}
	
	::close( m_logFd );
}
