
#ifndef _IDBC_CLUSTER_REDIS_H_
#define _IDBC_CLUSTER_REDIS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "idbc_redis.h"
#include "hiredis/hircluster.h"

typedef redisClusterContext *(*redisClusterConnect_i)(const char *, int);
typedef void *(*redisClusterCommand_i)(redisClusterContext *, const char *, ...);
typedef void (*freeReplyObject_i)(void *);
typedef void (*redisClusterFree_i)(redisClusterContext *);

class IdbcRedisCluster : public IdbcRedis
{
public:
	IdbcRedisCluster();
	~IdbcRedisCluster();

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
	redisClusterConnect_i m_predisClusterConnect_i;
	redisClusterCommand_i m_predisClusterCommand_i;
	freeReplyObject_i     m_pfreeReplyObject_i;
	redisClusterFree_i    m_predisClusterFree_i;	
private:
	void                 *m_pLib;
	redisClusterContext  *m_pClusterContext;
    redisReply           *m_pReply;
};

#endif
