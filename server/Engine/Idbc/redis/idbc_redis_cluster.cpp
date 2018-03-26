
#include "dlutil.h"
#include "idbc_redis_cluster.h"

#define HIREDISLIB "libhiredis_vip.so"

IdbcRedisCluster::IdbcRedisCluster()
	:m_pClusterContext( NULL ),
	 m_pReply( NULL )
{

}

IdbcRedisCluster::~IdbcRedisCluster()
{

}

bool IdbcRedisCluster::loadLib()
{
	bool result = false;
	m_pLib = DLOPEN(HIREDISLIB);
	if( NULL != m_pLib )
	{
		m_predisClusterConnect_i  = DLSYM( m_pLib, redisClusterConnect_i, "redisClusterConnect" );
		m_predisClusterCommand_i  = DLSYM( m_pLib, redisClusterCommand_i, "redisClusterCommand" );
		m_pfreeReplyObject_i      = DLSYM( m_pLib, freeReplyObject_i,     "freeReplyObject" );
		m_predisClusterFree_i     = DLSYM( m_pLib, redisClusterFree_i,    "redisClusterFree" );

		result = true;
	}
	
	return result;
}

bool IdbcRedisCluster::unloadLib()
{
	if( NULL != m_pLib )
	{
		DLCLOSE( m_pLib );
	}
	return true;
}

bool IdbcRedisCluster::connect( const char *hosts, int port )
{
	bool result = true;
	m_pClusterContext = m_predisClusterConnect_i(hosts, HIRCLUSTER_FLAG_NULL);
	if ( NULL == m_pClusterContext || m_pClusterContext->err) 
	{
		result = false;
	}
	return result;
}

bool IdbcRedisCluster::close()
{
	freeReply();
    m_predisClusterFree_i( m_pClusterContext );
}

void IdbcRedisCluster::freeReply()
{
	if( m_pReply )
	{
    	m_pfreeReplyObject_i( m_pReply );
	}
}

void IdbcRedisCluster::setKey( const char *key, const char *value )
{
	freeReply();
    m_pReply = (redisReply*)m_predisClusterCommand_i( m_pClusterContext, "SET %s %s", key, value );
}

const char *IdbcRedisCluster::getKey( const char *key )
{
	freeReply();
    m_pReply = (redisReply*)m_predisClusterCommand_i( m_pClusterContext, "GET %s", key );
    return m_pReply->str;
}

void IdbcRedisCluster::delKey( const char *key )
{
	freeReply();
    m_pReply = (redisReply*)m_predisClusterCommand_i( m_pClusterContext, "DEL %s", key );
}

void IdbcRedisCluster::sethmkey( const char *key, const char *field, const char *value )
{
	freeReply();
	m_pReply = (redisReply*)m_predisClusterCommand_i( m_pClusterContext, "HMSET %s %s %s", key, field, value );
}

const char *IdbcRedisCluster::gethmkey( const char *key, const char *field )
{
	freeReply();
	m_pReply = (redisReply*)m_predisClusterCommand_i( m_pClusterContext, "HGET %s %s", key, field );
	return m_pReply->str;
}

void IdbcRedisCluster::pushList( const char *key, const char *value )
{
	freeReply();
    m_pReply = (redisReply*)m_predisClusterCommand_i( m_pClusterContext, "LPUSH %s %s", key, value );	
}

void IdbcRedisCluster::lrange( std::vector<std::string> &lists, const char *key, int start, int stop )
{
	freeReply();
	lists.clear();
	m_pReply = (redisReply*)m_predisClusterCommand_i( m_pClusterContext,"LRANGE %s %d %d", key, start, stop );
    if (m_pReply->type == REDIS_REPLY_ARRAY) 
    {
        for (int j = 0; j < m_pReply->elements; j++) 
        {
            lists.push_back( m_pReply->element[j]->str );
        }
    }
}

