
#ifndef _KERNAL_NETWORK_MSG_DEF_H_
#define _KERNAL_NETWORK_MSG_DEF_H_

//消息相关处理
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <string>
#include <initializer_list>
#include <list>
#include "KernalEpoll.h"

#define BUFF_SIZE 1024*5

typedef signed char    int8;
typedef signed short   int16;
typedef signed int     int32;
typedef long long      int64;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef float          float32;
typedef double         float64;

#define NReadInt8(buf,data)   memcpy(data,buf,1); buf += 1;
#define NReadInt16(buf,data)  memcpy(data,buf,2); buf += 2;
#define NReadInt32(buf,data)  memcpy(data,buf,4); buf += 4;
#define NReadInt64(buf,data)  memcpy(data,buf,8); buf += 8;
#define NReadString(buf,data) {int len; NReadInt32(buf,&len); char *databuff = new char[len + 1];  memcpy(databuff,buf,len); databuff[len] = '\0'; data.append(databuff); delete databuff; buf += len;}
#define NReadBit(buf,data,len) memcpy(data,buf,len); buf += len;

#define NWriteInt8(buf,data)  memcpy(buf,data,1); buf += 1;
#define NWriteInt16(buf,data) memcpy(buf,data,2); buf += 2;
#define NWriteInt32(buf,data) memcpy(buf,data,4); buf += 4;
#define NWriteInt64(buf,data) memcpy(buf,data,8); buf += 8;
#define NWriteString(buf,data){int len = data.length(); NWriteInt32(buf,&len); memcpy(buf,data.c_str(),len); buf += len; }
#define NWriteBit(buf,data,len) memcpy(buf,data,len); buf += len;

#define DealStart(data) { int msgcmd = 0; NReadInt32(data,&msgcmd);  int error = 0; NReadInt32(data,&error); switch(msgcmd){
#define DealMsg(net, cmd, data) case MSG_##cmd:{ cmd val; Read##cmd(val,data); on##cmd(net,val); break; }
//#define DealMsg(net, cmd, data, userdatas) case MSG_##cmd:{ cmd val; Read##cmd(val,data); on##cmd(net,val, userdatas); break; }
#define DealEnd()	} }

#define MsgSend( net, id, cmd, error, msg ) {                 \
								char _buf[BUFF_SIZE] = {0};   \
								char* data = _buf;            \
								int len = 0;                  \
								NWriteInt32(data,&MSG_##cmd);  \
								int err = error;              \
								NWriteInt32(data,&err);        \
								Write##cmd(msg,data);         \
								len = data - _buf;            \
								data = _buf;           		  \
								net.send(id,_buf,len);        \
							}

//处理protobuf消息
#define DealProtobufStart(data){ int msgcmd = 0; NReadInt32(data, &msgcmd); int err = 0; NReadInt32(data, &err); int msglen = 0 ; NReadInt32(data, &msglen); switch(msgcmd) {
#define DealProtobufMsg(session, clientid, cmd, func) case cmd:{ func(session, clientid, data, msglen);break;}
#define DealProtobufEnd() }}

//序列化protobuf消息
#define SerializeObjToArray(obj, data, len) {                       \
								len = obj.ByteSize();               \
								char *databuff = new char[len + 1]; \
								memset(databuff,0,len+1);           \
								obj.SerializeToArray(databuff, len);\
								data = databuff;                    \
							}
//反序列化protobuf消息						
#define ParseFromArrayToObj(obj, data, len) {                 \
								obj.ParseFromArray(data, len);\
							}

#define ProtobufMsgSend( net, id, cmd, err, obj ) {                 \
								int pcmd = cmd;                     \
								int perr = err;                     \
								int len = obj.ByteSize();           \
								char *pdata = new char[len];        \
								obj.SerializeToArray(pdata, len);   \
								char *_buff = new char[len+12];      \
								char *databuff = _buff;             \
								NWriteInt32(databuff,&pcmd);        \
								NWriteInt32(databuff,&perr);        \
								NWriteInt32(databuff,&len);         \
								NWriteBit(databuff,pdata,len);      \
								net.send(id,_buff,len+12);           \
								delete []pdata; delete []_buff;     \
							}

//通过网关发送消息给客户端
#define ProtobufMsgSendToClientByGateWay( net, session, clientid, cmd, err, protomsg ) {              \
											int pcmd = cmd;                                           \
											int perr = err;                                           \
											int len1 = protomsg.ByteSize();                           \
											char *pdata = new char[len1];                             \
											protomsg.SerializeToArray(pdata, len1);                   \
											char *_buff = new char[len1+12];                          \
											char *databuff = _buff;                                   \
											NWriteInt32(databuff,&pcmd);                              \
											NWriteInt32(databuff,&perr);                              \
											NWriteInt32(databuff,&len1);                              \
											NWriteBit(databuff,pdata,len1);                           \
											GateWayInternalServerMsg msg;                             \
											msg.clientID = clientid;                                  \
											msg.type     = MESSAGE_DATA;                              \
											msg.initData( _buff, len1+12 );                           \
											MsgSend( net, session, GateWayInternalServerMsg, 0, msg );\
											delete []pdata; delete []_buff;                           \
										}
							
#endif
