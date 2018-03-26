
#include "KernalBuffer.h"

KernalBuffer::KernalBuffer( char *buff, int size )
{
	m_Size  = size;
	m_pBuff = ( char * )malloc( size );
	memcpy( m_pBuff, buff, size );
	//m_pBuff[ size ] = '\0';
}

KernalBuffer::~KernalBuffer()
{
	free( m_pBuff );
}
