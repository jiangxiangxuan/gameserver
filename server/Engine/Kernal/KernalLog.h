
#ifndef _KERNAL_LOG_H_
#define _KERNAL_LOG_H_

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <string>

#include "KernalLock.h"

class KernalLog
{
public:
    KernalLog();
    ~KernalLog();
public:
	void info( const char* pLogText, ... );
private:
};

#endif
