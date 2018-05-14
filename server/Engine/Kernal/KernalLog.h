
#ifndef _KERNAL_LOG_H_
#define _KERNAL_LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <string>

#include "KernalLock.h"
#include "KernalThread.h"

class KernalLog
{
public:
    KernalLog();
    ~KernalLog();
	
	bool init( char *pLogFilePath );
	void logWorker();
public:
	void info( const char *pLogText, ... );
private:
	KernalThread m_LogThread;
	char 		 m_pLogFilePath[256];
	int  		 m_logFd;
	
	int  		 m_logPipe[2];
	bool 		 m_quit;
};

#endif
