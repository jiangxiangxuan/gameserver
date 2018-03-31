
#include "ServerMsg.h"

void WriteDBServerReqMsg(DBServerReqMsg &value,char *&data)
{
	NWriteInt32(data,  &value.eventid);
	NWriteInt32(data,  &value.sqlType);
	NWriteString(data, value.sqlStr);
}

void ReadDBServerReqMsg(DBServerReqMsg &value,char *data)
{
	NReadInt32(data,  &value.eventid);
	NReadInt32(data,  &value.sqlType);
	NReadString(data, &value.sqlStr);
}

void WriteDBServerAckMsg(DBServerAckMsg &value,char *&data)
{
	NWriteInt32(data,  &value.eventid);
	NWriteInt32(data,  &value.error);
	NWriteInt32(data,  &value.datalen);
	NWriteBit(data,value.data,value.datalen);
}

void ReadDBServerAckMsg(DBServerAckMsg &value,char *data)
{
	NReadInt32(data,  &value.eventid);
	NReadInt32(data,  &value.error);
	NReadInt32(data,  &value.datalen);

	value.data = (char*)malloc(value.datalen);
	NReadBit(data,value.data,value.datalen);
}


void WriteGateWayInternalServerMsg(GateWayInternalServerMsg &value,char *&data)
{
	NWriteInt32(data,&value.clientID);
	NWriteInt32(data,&value.datalen);
	NWriteBit(data,value.data,value.datalen);
	NWriteInt32(data,&value.type);
}

void ReadGateWayInternalServerMsg(GateWayInternalServerMsg &value,char *data)
{
	NReadInt32(data,&value.clientID);
	NReadInt32(data,&value.datalen);

	value.data = (char*)malloc(value.datalen);
	NReadBit(data,value.data,value.datalen);
	NReadInt32(data,&value.type);
}


void WriteCenterRegisterServerInfo(CenterRegisterServerInfo &value,char *&data)
{
	NWriteInt32(data, &value.type);
	NWriteInt32(data, &value.port);
	NWriteString(data, value.ip);
}

void ReadCenterRegisterServerInfo(CenterRegisterServerInfo &value,char *data)
{
	NReadInt32(data, &value.type);
	NReadInt32(data, &value.port);
	NReadString(data, &value.ip);
}


void WriteCenterUpdateServerInfo(CenterUpdateServerInfo &value,char *&data)
{
	NWriteInt32(data, &value.num);
}

void ReadCenterUpdateServerInfo(CenterUpdateServerInfo &value,char *data)
{
	NReadInt32(data, &value.num);
}


void WriteCenterNotifyServerInfo(CenterNotifyServerInfo &value,char *&data)
{
	NWriteInt32(data, &value.type);
	NWriteInt32(data, &value.state);
	NWriteInt32(data, &value.port);
	NWriteString(data, value.ip);
}

void ReadCenterNotifyServerInfo(CenterNotifyServerInfo &value,char *data)
{
	NReadInt32(data, &value.type);
	NReadInt32(data, &value.state);
	NReadInt32(data, &value.port);
	NReadString(data, &value.ip);
}


void WritePlatformGameServerMsg(PlatformGameServerMsg &value,char *&data)
{
	NWriteInt32(data, &value.lineType);
	NWriteInt32(data, &value.serverID);
	NWriteInt32(data, &value.datalen);
	NWriteBit(data,value.data,value.datalen);
}

void ReadPlatformGameServerMsg(PlatformGameServerMsg &value,char *data)
{
	NReadInt32(data,&value.lineType);
	NReadInt32(data,&value.serverID);
	NReadInt32(data,&value.datalen);

	value.data = (char*)malloc(value.datalen);
	NReadBit(data,value.data,value.datalen);
}
