
#ifndef _IDBC_DEF_H
#define _IDBC_DEF_H

#include <map>
#include <vector>
#include <string.h>

enum DBType
{
	DBTYPE_INT        = 9  ,
	DBTYPE_STRING     = 254
};

struct DBColumn
{
	DBType  type;
	char   *name;
	void   *data;
	int     len;
};

struct DBRow
{
	std::map<char*, DBColumn*> columns;
};

#endif
