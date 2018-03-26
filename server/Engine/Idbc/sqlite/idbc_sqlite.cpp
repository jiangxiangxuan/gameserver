
#include "idbc_sqlite.h"

IdbcSqlite::IdbcSqlite()
{

}

IdbcSqlite::~IdbcSqlite()
{

}

bool IdbcSqlite::loadLib()
{
	return true;
}

bool IdbcSqlite::unloadLib()
{
	return true;
}

bool IdbcSqlite::connect(const char *host, const int port, const char *user, const char *passwd, const char *db)
{
	return false;
}

bool IdbcSqlite::connect(const char *dbPath)
{
	int result;
	result = sqlite3_open( dbPath, &m_pDB ); 
	return ( SQLITE_OK == result );
}

bool IdbcSqlite::prepare(StmtInfo **pStmtInfo, const char *query)
{
	bool result = false;

	sqlite3_stmt *pStmt = NULL;
    sqlite3_prepare( m_pDB, query, strlen( query ), &pStmt, 0 );
	{
		result = true;
	}

	*pStmtInfo = new StmtInfo();
    (*pStmtInfo)->pStmt = pStmt;
	return result;
}

bool IdbcSqlite::bindresult(StmtInfo *pStmtInfo)
{
	return true;
}

bool IdbcSqlite::bindparam(StmtInfo *pStmtInfo)
{
	return true;
}

void IdbcSqlite::execute(StmtInfo *pStmtInfo)
{
	//char  *errMsg = NULL; 
	//sqlite3_exec( m_pDB, pStmtInfo->pSql, 0, 0, &errMsg);
}

void IdbcSqlite::result(StmtInfo *pStmtInfo)
{

}

bool IdbcSqlite::fetch(StmtInfo *pStmtInfo)
{
	bool result = false;
	sqlite3_stmt *pStmt = ( sqlite3_stmt* )pStmtInfo->pStmt;
	if( SQLITE_DONE !=sqlite3_step( pStmt ) )
	{
		for( int i = 0; i < pStmtInfo->columns.size(); ++i )
    	{
    		DBColumn *column = pStmtInfo->columns[i];
    		memset(column->data,'\0', column->len );
    		switch( column->type )
    		{
    			case DBTYPE_INT:
    			{
    				sprintf((char*)column->data, "%d", sqlite3_column_int( pStmt,i ) );
    				break;
    			}
    			case DBTYPE_STRING:
    			{
    				sprintf((char*)column->data, "%s", sqlite3_column_text( pStmt,i ) );
    				break;
    			}
    			default:
    			break;
    		}
    	}
		result = true;
	}

	return result;
}

void IdbcSqlite::closeStmt(StmtInfo *pStmtInfo)
{
	sqlite3_stmt *pStmt = ( sqlite3_stmt* )pStmtInfo->pStmt;
	sqlite3_reset( pStmt );
	sqlite3_finalize( pStmt );
}

void IdbcSqlite::close()
{
	sqlite3_close( m_pDB );
}
