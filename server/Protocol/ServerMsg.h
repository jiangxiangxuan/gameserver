
//服务内部消息处理
#ifndef _SERVER_MSG_H_
#define _SERVER_MSG_H_

#include "Kernal/KernalNetWorkMsgDef.h"

//const int32 MSG_CheckKeepalived               = 0;    //检测心跳
const int32 MSG_GateWayInternalServerMsg      = 1;    //网关和内部服务器之间通信协议
const int32 MSG_PlatformGameServerMsg         = 2;    //大厅和游戏服务器之间通信协议
const int32 MSG_DBServerReqMsg                = 3;    //和数据服务器之间通信协议
const int32 MSG_DBServerAckMsg                = 4;    //和数据服务器之间通信协议
const int32 MSG_CenterRegisterServerInfo      = 5;    //内部服务器注册到中心服务器通信协议
const int32 MSG_CenterUpdateServerInfo        = 6;    //内部服务器更新中心服务器通信协议
const int32 MSG_CenterNotifyServerInfo        = 7;    //中心服务器通知相关服务器信息通信协议

enum DBSqlType
{
	DBSqlType_Select,
	DBSqlType_Update,
	DBSqlType_Insert,
	DBSqlType_Delete
};

//和数据服务器之间 操作数据库
struct DBServerReqMsg
{
	int         eventid;//操作的事件ID， 会在 DBServerAckMsg 中返回
	DBSqlType   sqlType;
	std::string sqlStr;
};
void WriteDBServerReqMsg(DBServerReqMsg &value,char *&data);
void ReadDBServerReqMsg(DBServerReqMsg &value,char *data);
void onDBServerReqMsg(int session, DBServerReqMsg &value);

//和数据服务器之间 操作数据库 返回相应信息
struct DBServerAckMsg
{
	int         eventid;//操作的事件ID
	int         error;
	int         datalen;
	char       *data;
};
void WriteDBServerAckMsg(DBServerAckMsg &value,char *&data);
void ReadDBServerAckMsg(DBServerAckMsg &value,char *data);
void onDBServerAckMsg(int session, DBServerAckMsg &value);

// 网关和游戏服务器之间数据类型
enum MessageType
{
	MESSAGE_NO,
	MESSAGE_DATA,         // 数据
	MESSAGE_CONNECTCLOSE  // 连接关闭
};

//网关和内部服务器
struct GateWayInternalServerMsg
{
	int   UID;
	int   clientID;
	int   datalen;
	char *data;
	MessageType type;
	void initData( char *buff, int size )
	{
		data = (char*)malloc( size );
		memset( data, 0, size );
		memcpy( data, buff, size );
		datalen = size;
	}
	GateWayInternalServerMsg()
		:UID( 0 )
		,clientID( 0 )
		,data( NULL )
		,type( MESSAGE_NO )
	{

	}
	~GateWayInternalServerMsg()
	{
		if( data )
		{
			free(data);
			data = NULL;
		}
	}
};
void WriteGateWayInternalServerMsg(GateWayInternalServerMsg &value,char *&data);
void ReadGateWayInternalServerMsg(GateWayInternalServerMsg &value,char *data);
void onGateWayInternalServerMsg(int session, GateWayInternalServerMsg &value);

enum ServerType
{
	SERVER_GATEWAY,  // 网关
	SERVER_PLATFORM, // 大厅
	SERVER_DBA,      // 数据服
	SERVER_GAME,     // 游戏
};
//向中心服务器注册
struct CenterRegisterServerInfo
{
	ServerType  type;
	int         port;
	std::string ip;
};
void WriteCenterRegisterServerInfo(CenterRegisterServerInfo &value,char *&data);
void ReadCenterRegisterServerInfo(CenterRegisterServerInfo &value,char *data);
void onCenterRegisterServerInfo(int session, CenterRegisterServerInfo &value);

//更新中心服务器中相关服务器信息
struct CenterUpdateServerInfo
{
	int num;
};
void WriteCenterUpdateServerInfo(CenterUpdateServerInfo &value,char *&data);
void ReadCenterUpdateServerInfo(CenterUpdateServerInfo &value,char *data);
void onCenterUpdateServerInfo(int session, CenterUpdateServerInfo &value);

// 服务器状态
enum ServerStateType
{
	SERVERSTATE_RUN,    // 正在运行
	SERVERSTATE_CLOSE,  // 服务器已经关闭
};
// 中心服务器通知网关相关游戏服的信息
struct CenterNotifyServerInfo
{
	ServerType      type;
	ServerStateType state;
	int             port;
	std::string     ip;
};
void WriteCenterNotifyServerInfo(CenterNotifyServerInfo &value,char *&data);
void ReadCenterNotifyServerInfo(CenterNotifyServerInfo &value,char *data);
void onCenterNotifyServerInfo(int session, CenterNotifyServerInfo &value);

enum PlatformGameLineType
{
	LINETYPE_PLATFORMTOGAME,    // 大厅到游戏服
	LINETYPE_GAMETOPLATFORM     // 游戏服到大厅
};
//大厅和游戏服之间的通信信息
struct PlatformGameServerMsg
{
	PlatformGameLineType lineType;
	int   serverID;
	int   datalen;
	char *data;
	void initData( char *buff, int size )
	{
		data = (char*)malloc( size );
		memset( data, 0, size );
		memcpy( data, buff, size );
		datalen = size;
	}
};
void WritePlatformGameServerMsg(PlatformGameServerMsg &value,char *&data);
void ReadPlatformGameServerMsg(PlatformGameServerMsg &value,char *data);
void onPlatformGameServerMsg(int session, PlatformGameServerMsg &value);
#endif
