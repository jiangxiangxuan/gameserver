
#ifndef _IDBC_MYSQL_H
#define _IDBC_MYSQL_H
#include <stdio.h>
#include <stdlib.h>
#include "mysql/mysql.h"
#include "../idbc.h"

typedef MYSQL      STDCALL *(*mysql_init_i)(MYSQL *);
typedef MYSQL      STDCALL *(*mysql_real_connect_i)(MYSQL *, const char *, const char *, const char *, const char *, unsigned int , const char *, unsigned long );
typedef MYSQL_STMT STDCALL *(*mysql_stmt_init_i)(MYSQL *);
typedef int        STDCALL (*mysql_stmt_prepare_i)(MYSQL_STMT *, const char *, unsigned long );
typedef my_bool    STDCALL (*mysql_stmt_bind_param_i)(MYSQL_STMT *, MYSQL_BIND *);
typedef my_bool    STDCALL (*mysql_stmt_bind_result_i)(MYSQL_STMT *, MYSQL_BIND *);
typedef int        STDCALL (*mysql_stmt_execute_i)(MYSQL_STMT *);
typedef int        STDCALL (*mysql_stmt_store_result_i)(MYSQL_STMT *);
typedef int        STDCALL (*mysql_stmt_fetch_i)(MYSQL_STMT *);
typedef my_bool    STDCALL (*mysql_stmt_close_i)(MYSQL_STMT *);
typedef my_bool    STDCALL (*mysql_close_i)(MYSQL *);

class IdbcMysql : public Idbc
{
public:
	IdbcMysql();
	~IdbcMysql();

	virtual bool loadLib();
	virtual bool unloadLib();
	virtual bool connect(const char *host, const int port, const char *user, const char *passwd, const char *db);
	virtual bool connect(const char *dbPath);
	virtual bool prepare(StmtInfo **pStmtInfo, const char *query);
	virtual bool bindresult(StmtInfo *pStmtInfo);
	virtual bool bindparam(StmtInfo *pStmtInfo);
	virtual void execute(StmtInfo *pStmtInfo);
	virtual void result(StmtInfo *pStmtInfo);
	virtual bool fetch(StmtInfo *pStmtInfo);
	virtual void closeStmt(StmtInfo *pStmtInfo);
	virtual void close();
private:
	mysql_init_i              m_pmysql_init_i;
	mysql_real_connect_i      m_pmysql_real_connect_i;
	mysql_stmt_init_i         m_pmysql_stmt_init_i;
	mysql_stmt_prepare_i      m_pmysql_stmt_prepare_i;
	mysql_stmt_bind_param_i   m_pmysql_stmt_bind_param_i;
	mysql_stmt_bind_result_i  m_pmysql_stmt_bind_result_i;
	mysql_stmt_execute_i      m_pmysql_stmt_execute_i;
	mysql_stmt_store_result_i m_pmysql_stmt_store_result_i;
	mysql_stmt_fetch_i        m_pmysql_stmt_fetch_i;
	mysql_stmt_close_i        m_pmysql_stmt_close_i;
	mysql_close_i             m_pmysql_close_i;
private:
	void  *m_pLib;
	MYSQL *m_pConn;
};

#endif
