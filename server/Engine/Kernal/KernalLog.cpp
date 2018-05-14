
#include "KernalLog.h"
#include <stdarg.h>

KernalLog::KernalLog()
{
	
}

KernalLog::~KernalLog()
{

}

void KernalLog::info( const char* pLogText, ... )
{
	va_list args;
    va_start( args, pLogText );
    char logbuff[1024];
	memset( logbuff, 0, sizeof(logbuff) );
	vsprintf( logbuff, pLogText, args );
    va_end( args );
	printf("KernalLog::info: %s \n\r", logbuff);
}
