
#include "ServerMsg.h"

void WriteDBServerReqMsg(DBServerReqMsg &value,char *&data)
{
	WriteInt32(data,  &value.eventid);
	WriteInt32(data,  &value.sqlType);
	WriteString(data, value.sqlStr);
}

void ReadDBServerReqMsg(DBServerReqMsg &value,char *data)
{
	ReadInt32(data,  &value.eventid);
	ReadInt32(data,  &value.sqlType);
	ReadString(data, &value.sqlStr);
}

void WriteDBServerAckMsg(DBServerAckMsg &value,char *&data)
{
	WriteInt32(data,  &value.eventid);
	WriteInt32(data,  &value.error);
	WriteInt32(data,  &value.datalen);
	WriteBit(data,value.data,value.datalen);
}

void ReadDBServerAckMsg(DBServerAckMsg &value,char *data)
{
	ReadInt32(data,  &value.eventid);
	ReadInt32(data,  &value.error);
	ReadInt32(data,  &value.datalen);

	value.data = (char*)malloc(value.datalen);
	ReadBit(data,value.data,value.datalen);
}


void WriteGateWayInternalServerMsg(GateWayInternalServerMsg &value,char *&data)
{
	WriteInt32(data,&value.clientID);
	WriteInt32(data,&value.datalen);
	WriteBit(data,value.data,value.datalen);
	WriteInt32(data,&value.type);
}

void ReadGateWayInternalServerMsg(GateWayInternalServerMsg &value,char *data)
{
	ReadInt32(data,&value.clientID);
	ReadInt32(data,&value.datalen);

	value.data = (char*)malloc(value.datalen);
	ReadBit(data,value.data,value.datalen);
	ReadInt32(data,&value.type);
}


void WriteCenterRegisterServerInfo(CenterRegisterServerInfo &value,char *&data)
{
	WriteInt32(data, &value.type);
	WriteInt32(data, &value.port);
	WriteString(data, value.ip);
}

void ReadCenterRegisterServerInfo(CenterRegisterServerInfo &value,char *data)
{
	ReadInt32(data, &value.type);
	ReadInt32(data, &value.port);
	ReadString(data, &value.ip);
}


void WriteCenterUpdateServerInfo(CenterUpdateServerInfo &value,char *&data)
{
	WriteInt32(data, &value.num);
}

void ReadCenterUpdateServerInfo(CenterUpdateServerInfo &value,char *data)
{
	ReadInt32(data, &value.num);
}


void WriteCenterNotifyServerInfo(CenterNotifyServerInfo &value,char *&data)
{
	WriteInt32(data, &value.type);
	WriteInt32(data, &value.state);
	WriteInt32(data, &value.port);
	WriteString(data, value.ip);
}

void ReadCenterNotifyServerInfo(CenterNotifyServerInfo &value,char *data)
{
	ReadInt32(data, &value.type);
	ReadInt32(data, &value.state);
	ReadInt32(data, &value.port);
	ReadString(data, &value.ip);
}


void WritePlatformGameServerMsg(PlatformGameServerMsg &value,char *&data)
{
	WriteInt32(data, &value.lineType);
	WriteInt32(data, &value.serverID);
	WriteInt32(data, &value.datalen);
	WriteBit(data,value.data,value.datalen);
}

void ReadPlatformGameServerMsg(PlatformGameServerMsg &value,char *data)
{
	ReadInt32(data,&value.lineType);
	ReadInt32(data,&value.serverID);
	ReadInt32(data,&value.datalen);

	value.data = (char*)malloc(value.datalen);
	ReadBit(data,value.data,value.datalen);
}
