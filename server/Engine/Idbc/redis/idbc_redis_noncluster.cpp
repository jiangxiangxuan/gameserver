
#include "dlutil.h"
#include "idbc_redis_noncluster.h"

#define HIREDISLIB "libhiredis_vip.so"

IdbcRedisNonCluster::IdbcRedisNonCluster()
	:m_pContext( NULL ),
	 m_pReply( NULL )
{

}

IdbcRedisNonCluster::~IdbcRedisNonCluster()
{

}

bool IdbcRedisNonCluster::loadLib()
{
	bool result = false;
	m_pLib = DLOPEN(HIREDISLIB);
	if( NULL != m_pLib )
	{
		m_predisConnect_i  = DLSYM( m_pLib, redisConnect_i, "redisConnect" );
		m_predisCommand_i  = DLSYM( m_pLib, redisCommand_i, "redisCommand" );
		m_pfreeReplyObject_i      = DLSYM( m_pLib, freeReplyObject_i,     "freeReplyObject" );
		m_predisFree_i     = DLSYM( m_pLib, redisFree_i,    "redisFree" );

		result = true;
	}
	
	return result;
}

bool IdbcRedisNonCluster::unloadLib()
{
	if( NULL != m_pLib )
	{
		DLCLOSE( m_pLib );
	}
	return true;
}

bool IdbcRedisNonCluster::connect( const char *hosts, int port )
{
	bool result = true;
	m_pContext = m_predisConnect_i(hosts, port);
	if ( NULL == m_pContext || m_pContext->err) 
	{
		printf("connect non cluster: host=%s, port = %d \n", hosts, port);
		result = false;
	}
	return result;
}

bool IdbcRedisNonCluster::close()
{
	freeReply();
    m_predisFree_i( m_pContext );
}

void IdbcRedisNonCluster::freeReply()
{
	if( m_pReply )
	{
    	m_pfreeReplyObject_i( m_pReply );
		m_pReply = NULL;
	}
}

void IdbcRedisNonCluster::setKey( const char *key, const char *value )
{
	freeReply();
    m_pReply = (redisReply*)m_predisCommand_i( m_pContext, "SET %s %s", key, value );
}

const char *IdbcRedisNonCluster::getKey( const char *key )
{
	freeReply();
    m_pReply = (redisReply*)m_predisCommand_i( m_pContext, "GET %s", key );
    return m_pReply->str;
}

void IdbcRedisNonCluster::delKey( const char *key )
{
	freeReply();
    m_pReply = (redisReply*)m_predisCommand_i( m_pContext, "DEL %s", key );
}

void IdbcRedisNonCluster::sethmkey( const char *key, const char *field, const char *value )
{
	freeReply();
	m_pReply = (redisReply*)m_predisCommand_i( m_pContext, "HMSET %s %s %s", key, field, value );
}

const char *IdbcRedisNonCluster::gethmkey( const char *key, const char *field )
{
	freeReply();
	m_pReply = (redisReply*)m_predisCommand_i( m_pContext, "HGET %s %s", key, field );
	return m_pReply->str;
}

void IdbcRedisNonCluster::pushList( const char *key, const char *value )
{
	freeReply();
    m_pReply = (redisReply*)m_predisCommand_i( m_pContext, "LPUSH %s %s", key, value );	
}

void IdbcRedisNonCluster::lrange( std::vector<std::string> &lists, const char *key, int start, int stop )
{
	freeReply();
	lists.clear();
	m_pReply = (redisReply*)m_predisCommand_i( m_pContext,"LRANGE %s %d %d", key, start, stop );
    if (m_pReply->type == REDIS_REPLY_ARRAY) 
    {
        for (int j = 0; j < m_pReply->elements; j++) 
        {
            lists.push_back( m_pReply->element[j]->str );
        }
    }
}

