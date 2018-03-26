
#ifndef _IDBC_REDIS_H_
#define _IDBC_REDIS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

class IdbcRedis
{
public:

	virtual bool loadLib() = 0;
	virtual bool unloadLib() = 0;

	virtual bool connect(const char *hosts, int port = 0 ) = 0;
	virtual bool close() = 0;

 	virtual void setKey( const char *key, const char *value ) = 0;
	virtual const char *getKey( const char *key ) = 0;
	virtual void delKey( const char *key ) = 0;
	
	virtual void sethmkey( const char *key, const char *field, const char *value ) = 0;
	virtual const char *gethmkey( const char *key, const char *field ) = 0;

	virtual void pushList( const char *key, const char *value ) = 0;
	virtual void lrange( std::vector<std::string> &lists, const char *key, int start = 0, int stop = -1) = 0;

	virtual void freeReply() = 0;
};

#endif

