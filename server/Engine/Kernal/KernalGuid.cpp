
#include "KernalGuid.h"
#include "KernalUtil.h"
#include "KernalCrc32.h"

static unsigned long int g_guid = 0;

bool guidInit()
{
	char ip[20];
	char mac[17];
	getLocalIP( ip );
	getLocalMac( mac );
	unsigned int ipr  = Crc32( ip );
	unsigned int macr = Crc32( mac );

	g_guid = ipr ^ macr;
	return true;
}

unsigned int getGuid()
{
	unsigned int curTime = getCurrentTime();
	unsigned int t = curTime;
	printf("getGuid g_guid=%ld  time=%ld  time1=%d  guid = %ld  t=%d\r\n", g_guid, curTime, curTime, g_guid ^ curTime,t);
	return g_guid ^ curTime;
}
