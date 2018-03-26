
#ifndef _KERNAL_BUFFER_H_
#define _KERNAL_BUFFER_H_

#include <stdlib.h>
#include <string.h>
#include "KernalList.h"

class KernalBuffer : public KernalListNode< KernalBuffer >
{
public:
	KernalBuffer( char *buff, int size );
	~KernalBuffer();

	char *getData()
	{
		return m_pBuff;
	};
	int size()
	{
		return m_Size;
	};
private:
	char *m_pBuff;
	int   m_Size;	
};

#endif
