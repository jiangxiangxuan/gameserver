
#ifndef _IDBC_SQLITE_H
#define _IDBC_SQLITE_H
#include <stdio.h>
#include <stdlib.h>
#include "sqlite/sqlite3.h"
#include "../idbc.h"

class IdbcSqlite : public Idbc
{
public:
	IdbcSqlite();
	~IdbcSqlite();

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
	sqlite3 *m_pDB;
};

#endif
