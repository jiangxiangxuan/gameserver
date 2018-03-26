
#include "KernalHash.h"

int stringToHash( const char *str, int len, int defhash )
{
	int hash = len;
	if( defhash > 0 )
	{
		hash = defhash;
	}
	for( int i = 0; i < len; ++i )
	{
		hash = (hash<<4)^(hash>>28)^str[i];
	}

	return hash;
}
