
#ifndef _IDBC_H
#define _IDBC_H

#include "idbcdef.h"
#include "dlutil.h"

struct StmtInfo
{
    void                  *pStmt;
    char                  *pSql;
    std::vector<DBColumn*> columns;
};

class Idbc
{
public:
	virtual bool loadLib() = 0;
	virtual bool unloadLib() = 0;
	/*
	*创建数据库连接conn
	*/
	virtual bool connect(const char *host, const int port, const char *user, const char *passwd, const char *db) = 0;
	virtual bool connect(const char *dbPath) = 0;//连接sqlite
	/*
	*预处理,创建STMT句柄
	*/
	virtual bool prepare(StmtInfo **pStmtInfo, const char *query) = 0;
	/*
	*绑定数据
	*/
	virtual bool bindresult(StmtInfo *pStmtInfo) = 0;
	/*
	*绑定参数
	*/
	virtual bool bindparam(StmtInfo *pStmtInfo) = 0;
	/*
	*执行与语句句柄相关的预处理查询
	*/
	virtual void execute(StmtInfo *pStmtInfo) = 0;
	/*
	*缓冲数据
	*/
	virtual void result(StmtInfo *pStmtInfo) = 0;
	/*
	*返回结果集中的下一行
	*/
	virtual bool fetch(StmtInfo *pStmtInfo) = 0;
	/*
	*关闭预处理语句 
	*/
	virtual void closeStmt(StmtInfo *pStmtInfo) = 0;
	/*
	*关闭连接
	*/
	virtual void close() = 0;
};

#endif
