
#include "KernalMemory.h"

KernalMemoryBlock::KernalMemoryBlock()
	:m_blockNum( 0 )   ,
	 m_blockSize( 0 )  ,
	 m_pData( NULL )   ,
	 m_pEndData( NULL )
{

}

KernalMemoryBlock::~KernalMemoryBlock()
{
	if( m_pData )
	{
		free( m_pData );
	}
}

bool KernalMemoryBlock::init( unsigned int blockNum, unsigned int blockSize )
{
	m_blockNum  = blockNum;
	m_blockSize = blockSize;
	m_pData     = malloc( m_blockNum * m_blockSize );
	m_pEndData  = ( char * )m_pData + m_blockNum * m_blockSize;

	for( int i = 0; i < m_blockNum; ++i )
	{
		m_freeMemoryBlocks.push( i );
	}
}

void *KernalMemoryBlock::alloc( unsigned int size )
{
	void *pData               = NULL;
	unsigned int curFreeIndex = 0;
	if( m_freeMemoryBlocks.empty() )
	{
		goto EXIT;
	}
	curFreeIndex = m_freeMemoryBlocks.top();
	m_freeMemoryBlocks.pop();
	pData = ( char * )m_pData + curFreeIndex * m_blockSize;
EXIT:
	return pData;
}

bool  KernalMemoryBlock::free( void *pBlock )
{
	bool result             = false;
	unsigned int blockIndex = 0;
	if( !(pBlock >= m_pData && pBlock <= m_pEndData) )
	{
		goto EXIT;
	}

	blockIndex = (unsigned int)( (char*)pBlock - (char*)m_pData ) / m_blockSize - 1;
	m_freeMemoryBlocks.push( blockIndex );

	result = true;
EXIT:
	return result;
}

KernalMemoryPool::KernalMemoryPool()
{

}

KernalMemoryPool::~KernalMemoryPool()
{

}

bool KernalMemoryPool::create( unsigned int blockNum, unsigned int blockSize )
{
	return NULL != allocBlock( blockNum, blockSize );
}

void *KernalMemoryPool::allocBlock( unsigned int blockNum, unsigned int blockSize )
{
	KernalMemoryBlock *pKernalMemoryBlock = new KernalMemoryBlock();
	if( !pKernalMemoryBlock->init( blockNum, blockSize ) )
	{
		delete pKernalMemoryBlock;
		pKernalMemoryBlock = NULL;

		goto EXIT;
	}

	m_KernalMemoryBlocks.push_back( pKernalMemoryBlock );
EXIT:
	return pKernalMemoryBlock;
}

void *KernalMemoryPool::alloc(unsigned int size )
{
	void *pData = NULL;

	KernalMemoryBlock *pHead = m_KernalMemoryBlocks.getHead();
	KernalMemoryBlock *pNext = pHead; 
	while( pNext )
	{
		pData = pNext->alloc( size );
		if( pData )
		{
			break;
		}
		pNext = pNext->m_pNext;
	}

	//如果内存池已经分配完，则重新创建一个内存块,并分配内存
	if( !pData )
	{
		 pNext = (KernalMemoryBlock *)allocBlock( pHead->getBlockNum(), pHead->getBlockSize() );
		pData = pNext->alloc( size );
	}
EXIT:
	return pData;	
}

bool KernalMemoryPool::free( void *pBlock )
{
	bool result = false;

	KernalMemoryBlock *pHead = m_KernalMemoryBlocks.getHead();
	KernalMemoryBlock *pNext = pHead; 
	while( pNext )
	{
		if( pNext->free( pBlock ) )
		{
			result = true;
			break;
		}
		pNext = pNext->m_pNext;
	}

EXIT:
	return result;	
}
