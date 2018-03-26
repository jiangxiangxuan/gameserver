
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

#define ReadInt8(buf,data)   memcpy(data,buf,1); buf += 1;
#define ReadInt16(buf,data)  memcpy(data,buf,2); buf += 2;
#define ReadInt32(buf,data)  memcpy(data,buf,4); buf += 4;
#define ReadInt64(buf,data)	 memcpy(data,buf,8); buf += 8;
#define ReadString(buf,data) {int len; ReadInt32(buf,&len); char *databuff = new char[len + 1];  memcpy(databuff,buf,len); databuff[len] = '\0'; data.append(databuff); delete databuff; buf += len;}
#define ReadBit(buf,data,len) memcpy(data,buf,len); buf += len;

#define WriteInt8(buf,data)	 memcpy(buf,data,1); buf += 1;
#define WriteInt16(buf,data) memcpy(buf,data,2); buf += 2;
#define WriteInt32(buf,data) memcpy(buf,data,4); buf += 4;
#define WriteInt64(buf,data) memcpy(buf,data,8); buf += 8;
#define WriteString(buf,data){int len = data.length(); WriteInt32(buf,&len); memcpy(buf,data.c_str(),len); buf += len; }
#define WriteBit(buf,data,len) memcpy(buf,data,len); buf += len;

#define DealStart(data) { int msgcmd = 0; ReadInt32(data,&msgcmd);  int error = 0; ReadInt32(data,&error); switch(msgcmd){
#define DealMsg(net, cmd, data) case MSG_##cmd:{ cmd val; Read##cmd(val,data); on##cmd(net,val); break; }
//#define DealMsg(net, cmd, data, userdatas) case MSG_##cmd:{ cmd val; Read##cmd(val,data); on##cmd(net,val, userdatas); break; }
#define DealEnd()	} }

#define MsgSend( net, id, cmd, error, msg ) {                 \
								char _buf[BUFF_SIZE] = {0};   \
								char* data = _buf;            \
								int len = 0;                  \
								WriteInt32(data,&MSG_##cmd);  \
								int err = error;              \
								WriteInt32(data,&err);        \
								Write##cmd(msg,data);         \
								len = data - _buf;            \
								data = _buf;           		  \
								net.send(id,_buf,len);        \
							}

#endif
