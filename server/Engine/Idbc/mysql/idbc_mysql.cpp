
#include "idbc_mysql.h"

#define MYSQLLIB "libmysqlclient.so"

IdbcMysql::IdbcMysql()
{

}

IdbcMysql::~IdbcMysql()
{

}

bool IdbcMysql::loadLib()
{
	bool result = false;
	m_pLib = DLOPEN(MYSQLLIB);
	if( NULL != m_pLib )
	{
		m_pmysql_init_i              = DLSYM( m_pLib, mysql_init_i,              "mysql_init" );
		m_pmysql_real_connect_i      = DLSYM( m_pLib, mysql_real_connect_i,      "mysql_real_connect" );
		m_pmysql_stmt_init_i         = DLSYM( m_pLib, mysql_stmt_init_i,         "mysql_stmt_init" );
		m_pmysql_stmt_prepare_i      = DLSYM( m_pLib, mysql_stmt_prepare_i,      "mysql_stmt_prepare" );
		m_pmysql_stmt_bind_param_i   = DLSYM( m_pLib, mysql_stmt_bind_param_i,   "mysql_stmt_bind_param" );
		m_pmysql_stmt_bind_result_i  = DLSYM( m_pLib, mysql_stmt_bind_result_i,  "mysql_stmt_bind_result" );
		m_pmysql_stmt_execute_i      = DLSYM( m_pLib, mysql_stmt_execute_i,      "mysql_stmt_execute" );
		m_pmysql_stmt_store_result_i = DLSYM( m_pLib, mysql_stmt_store_result_i, "mysql_stmt_store_result" );
		m_pmysql_stmt_fetch_i        = DLSYM( m_pLib, mysql_stmt_fetch_i,        "mysql_stmt_fetch" );
		m_pmysql_stmt_close_i        = DLSYM( m_pLib, mysql_stmt_close_i,        "mysql_stmt_close" );
		m_pmysql_close_i             = DLSYM( m_pLib, mysql_close_i,             "mysql_close" );

		result = true;
	}
	
	return result;
}

bool IdbcMysql::unloadLib()
{
	if( NULL != m_pLib )
	{
		DLCLOSE( m_pLib );
	}
	return true;
}

bool IdbcMysql::connect(const char *host, const int port, const char *user, const char *passwd, const char *db)
{
	bool result = true;
	m_pConn = m_pmysql_init_i(NULL);   
	if( !m_pmysql_real_connect_i(m_pConn, host, user, passwd, db, port, NULL, CLIENT_FOUND_ROWS) )   
    {
        result = false;
    }  
    return result;
}

bool IdbcMysql::connect(const char *dbPath)
{
	return false;
}

bool IdbcMysql::prepare(StmtInfo **pStmtInfo, const char *query)
{
	bool result = true;
	MYSQL_STMT *pStmt = m_pmysql_stmt_init_i( m_pConn ); 
	if(m_pmysql_stmt_prepare_i(pStmt, query, strlen(query)))
	{
		result = false;
	}
	*pStmtInfo = new StmtInfo();
    (*pStmtInfo)->pStmt = pStmt;
	return result;
}

bool IdbcMysql::bindresult(StmtInfo *pStmtInfo)
{
	MYSQL_STMT *pStmt = (MYSQL_STMT*)pStmtInfo->pStmt;

	MYSQL_BIND params[ pStmtInfo->columns.size() ];
    memset(params, 0, sizeof(params));
    for( int i = 0; i < pStmtInfo->columns.size(); ++i )
    {
    	DBColumn *column          = pStmtInfo->columns[i];
    	params[ i ].buffer_type   = (enum_field_types)column->type;
    	params[ i ].buffer        = column->data;
		params[ i ].buffer_length = column->len;
    }
    m_pmysql_stmt_bind_result_i( pStmt, params ); 
	return true;
}

bool IdbcMysql::bindparam(StmtInfo *pStmtInfo)
{
	MYSQL_STMT *pStmt = (MYSQL_STMT*)pStmtInfo->pStmt;

	MYSQL_BIND params[ pStmtInfo->columns.size() ];
    memset(params, 0, sizeof(params));
    for( int i = 0; i < pStmtInfo->columns.size(); ++i )
    {
    	DBColumn *column          = pStmtInfo->columns[i];
    	params[ i ].buffer_type   = (enum_field_types)column->type;
    	params[ i ].buffer        = column->data;
		params[ i ].buffer_length = column->len;
    }
    m_pmysql_stmt_bind_param_i( pStmt, params ); 
	return true;	
}

void IdbcMysql::execute(StmtInfo *pStmtInfo)
{
	MYSQL_STMT *pStmt = (MYSQL_STMT*)pStmtInfo->pStmt;
	m_pmysql_stmt_execute_i( pStmt );
}

void IdbcMysql::result(StmtInfo *pStmtInfo)
{
	MYSQL_STMT *pStmt = (MYSQL_STMT*)pStmtInfo->pStmt;
	m_pmysql_stmt_store_result_i( pStmt );
}

bool IdbcMysql::fetch(StmtInfo *pStmtInfo)
{
	MYSQL_STMT *pStmt = (MYSQL_STMT*)pStmtInfo->pStmt;
	return 0 == m_pmysql_stmt_fetch_i( pStmt );
}

void IdbcMysql::closeStmt(StmtInfo *pStmtInfo)
{
	MYSQL_STMT *pStmt = (MYSQL_STMT*)pStmtInfo->pStmt;
	m_pmysql_stmt_close_i(pStmt);
}

void IdbcMysql::close()
{
	m_pmysql_close_i( m_pConn );
}
