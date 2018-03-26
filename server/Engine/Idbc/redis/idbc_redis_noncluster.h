
#ifndef _IDBC_NON_CLUSTER_REDIS_H_
#define _IDBC_NON_CLUSTER_REDIS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "idbc_redis.h"
#include "hiredis/hiredis.h"

typedef redisContext *(*redisConnect_i)(const char *, int);
typedef void *(*redisCommand_i)(redisContext *, const char *, ...);
typedef void (*freeReplyObject_i)(void *);
typedef void (*redisFree_i)(redisContext *);

class IdbcRedisNonCluster : public IdbcRedis
{
public:
	IdbcRedisNonCluster();
	~IdbcRedisNonCluster();

	virtual bool loadLib();
	virtual bool unloadLib();

	virtual bool connect(const char *hosts, int port = 0 );
	virtual bool close();

	virtual void setKey( const char *key, const char *value );
	virtual const char *getKey( const char *key );
	virtual void delKey( const char *key );
	
	virtual void sethmkey( const char *key, const char *field, const char *value );
	virtual const char *gethmkey( const char *key, const char *field );

	virtual void pushList( const char *key, const char *value );
	virtual void lrange( std::vector<std::string> &lists, const char *key, int start = 0, int stop = -1);

private:
	virtual void freeReply();
private:
	redisConnect_i m_predisConnect_i;
	redisCommand_i m_predisCommand_i;
	freeReplyObject_i     m_pfreeReplyObject_i;
	redisFree_i    m_predisFree_i;	
private:
	void                 *m_pLib;
	redisContext  *m_pContext;
    redisReply           *m_pReply;
};

#endif
