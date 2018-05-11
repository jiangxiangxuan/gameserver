
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

unsigned long int getGuid()
{
	unsigned long int curTime = getCurrentTime();
	printf("getGuid g_guid=%ld  time=%ld  guid = %ld\r\n", g_guid, curTime, g_guid ^ curTime);
	return g_guid ^ curTime;
}
