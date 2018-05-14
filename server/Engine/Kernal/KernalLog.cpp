
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
	va_list ap;
    va_start( ap, pLogText );
    char logbuff[1024] = { 0 };
	sprintf( logbuff, pLogText, ap );
    va_end( ap );
	printf("KernalLog::info: %s \n\r", logbuff);
}
