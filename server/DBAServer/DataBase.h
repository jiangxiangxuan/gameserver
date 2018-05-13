
#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <stdio.h>
#include <stdlib.h>
#include "mysql/mysql.h"

#define DATABASE_BUFF_SIZE 65535

typedef MYSQL        STDCALL *(*mysql_init_i)(MYSQL *);
typedef MYSQL        STDCALL *(*mysql_real_connect_i)(MYSQL *, const char *, const char *, const char *, const char *, unsigned int , const char *, unsigned long );
typedef MYSQL_STMT   STDCALL *(*mysql_stmt_init_i)(MYSQL *);
typedef int          STDCALL (*mysql_stmt_prepare_i)(MYSQL_STMT *, const char *, unsigned long );
typedef my_bool      STDCALL (*mysql_stmt_bind_param_i)(MYSQL_STMT *, MYSQL_BIND *);
typedef my_bool      STDCALL (*mysql_stmt_bind_result_i)(MYSQL_STMT *, MYSQL_BIND *);
typedef int          STDCALL (*mysql_stmt_execute_i)(MYSQL_STMT *);
typedef int          STDCALL (*mysql_stmt_store_result_i)(MYSQL_STMT *);
typedef int          STDCALL (*mysql_stmt_fetch_i)(MYSQL_STMT *);
typedef my_bool      STDCALL (*mysql_stmt_close_i)(MYSQL_STMT *);
typedef my_bool      STDCALL (*mysql_close_i)(MYSQL *);
typedef int          STDCALL (*mysql_query_i)(MYSQL *, const char *);
typedef MYSQL_RES   *STDCALL (*mysql_store_result_i)(MYSQL *);
typedef MYSQL_FIELD *STDCALL (*mysql_fetch_field_i)(MYSQL_RES *);
typedef unsigned int STDCALL (*mysql_num_fields_i)(MYSQL_RES *);
typedef MYSQL_ROW	 STDCALL (*mysql_fetch_row_i)(MYSQL_RES *result);
typedef const char * STDCALL (*mysql_error_i)(MYSQL *);
typedef void         STDCALL (*mysql_free_result_i)(MYSQL_RES *result);

class DataBase
{
public:
	DataBase();
	~DataBase();

	bool loadLib();
	bool unloadLib();
	void close();

	bool connect(const char *host, const int port, const char *user, const char *passwd, const char *db);
	bool query( const char *sql );
	bool store();
    void freeResult();
	MYSQL_ROW &fetch();
	int getColNum()
	{
		return m_ColNum;
	};
	char *getDataBuff()
	{
		return m_DataBuff;
	}
public:
	char m_DataBuff[DATABASE_BUFF_SIZE];
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
	mysql_query_i             m_pmysql_query_i;
	mysql_store_result_i      m_pmysql_store_result_i;
	mysql_fetch_field_i       m_pmysql_fetch_field_i;
	mysql_num_fields_i        m_pmysql_num_fields_i;
	mysql_fetch_row_i         m_pmysql_fetch_row_i;
	mysql_error_i             m_pmysql_error_i;
    mysql_free_result_i       m_pmysql_free_result_i;
private:
	void      *m_pLib;
	MYSQL     *m_pConn;
	MYSQL_ROW  m_Row;
	MYSQL_RES *m_Result;
	int        m_ColNum;
};

#endif
