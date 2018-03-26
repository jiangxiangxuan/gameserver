
#include "DataBase.h"
#include "dlutil.h"

#define MYSQLLIB "libmysqlclient.so"

DataBase::DataBase()
	:m_pmysql_free_result_i( NULL )
{

}

DataBase::~DataBase()
{

}

bool DataBase::loadLib()
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
		m_pmysql_query_i             = DLSYM( m_pLib, mysql_query_i,             "mysql_query" );
		m_pmysql_store_result_i      = DLSYM( m_pLib, mysql_store_result_i,      "mysql_store_result" );
		m_pmysql_fetch_field_i       = DLSYM( m_pLib, mysql_fetch_field_i,       "mysql_fetch_field" );
		m_pmysql_num_fields_i        = DLSYM( m_pLib, mysql_num_fields_i,        "mysql_num_fields" );
		m_pmysql_fetch_row_i         = DLSYM( m_pLib, mysql_fetch_row_i,         "mysql_fetch_row" );
		m_pmysql_error_i             = DLSYM( m_pLib, mysql_error_i,             "mysql_error" );
        m_pmysql_free_result_i       = DLSYM( m_pLib, mysql_free_result_i,       "mysql_free_result" );

		result = true;
	}

	return result;
}

bool DataBase::unloadLib()
{
	if( NULL != m_pLib )
	{
		DLCLOSE( m_pLib );
	}
	return true;
}

void DataBase::close()
{
  freeResult();
	m_pmysql_close_i( m_pConn );
}

bool DataBase::connect(const char *host, const int port, const char *user, const char *passwd, const char *db)
{
	bool result = true;
	m_pConn = m_pmysql_init_i(NULL);
	if( !m_pmysql_real_connect_i(m_pConn, host, user, passwd, db, port, NULL, CLIENT_FOUND_ROWS) )
    {
        result = false;
    }
    return result;
}

bool DataBase::query( const char *sql )
{
	//printf("query mysql error=%d,%s\n", m_pmysql_error_i( m_pConn ), sql );
	return 0 == m_pmysql_query_i( m_pConn, sql );
}

bool DataBase::store()
{
    //freeResult();
	m_Result = m_pmysql_store_result_i( m_pConn );
	if( m_Result )
	{
		m_ColNum = m_pmysql_num_fields_i( m_Result );
	}
	else
	{
		m_ColNum = 0;
	}

	return true;
}

void DataBase::freeResult()
{
    if( m_Result )
    {
		m_pmysql_free_result_i( m_Result );
        m_Result = NULL;
	}
}

MYSQL_ROW &DataBase::fetch()
{
	m_Row = m_pmysql_fetch_row_i( m_Result );
	return m_Row;
}
