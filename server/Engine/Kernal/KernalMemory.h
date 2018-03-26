
/******************************************************************* 
 *  Copyright(c)2016-2016 Company Name 
 *  All rights reserved. 
 *   
 *  文件名称: KernalMemory.h
 *  简要描述: 内存池管理
 *   
 *  创建日期: 2016-10-28
 *  作者: Jephy
 *  说明: 
 ******************************************************************/

#ifndef _KERNAL_MEMORY_H_
#define _KERNAL_MEMORY_H_

#include <stdlib.h>
#include <string.h>
#include <stack>
#include "KernalList.h"

class KernalMemoryBlock : public KernalListNode<KernalMemoryBlock>
{
public:
	KernalMemoryBlock();
	~KernalMemoryBlock();

	bool  init( unsigned int blockNum, unsigned int blockSize );
	void *alloc( unsigned int size );
	bool  free( void *pBlock );

	unsigned int getFreeBlockCount()
	{
		return m_freeMemoryBlocks.size();
	};
	unsigned int getBlockNum()
	{
		return m_blockNum;
	};
	unsigned int getBlockSize()
	{
		return m_blockSize;
	};
private:
	std::stack< unsigned int > m_freeMemoryBlocks;//空闲块列表

	 unsigned int m_blockNum;
	 unsigned int m_blockSize;
	 void        *m_pData;
	 void        *m_pEndData;
};

class KernalMemoryPool
{
public:
	KernalMemoryPool();
	~KernalMemoryPool();

	/**
	 *  功能描述: 创建一个内存池，初始化
	 *  @param blockNum  :内存池块数
	 *  @param blockSize :内存池每块大小
	 *  @return  
	 */
	bool create( unsigned int blockNum, unsigned int blockSize );
	/**
	 *  功能描述: 分配内存块
	 *  @param blockNum  :内存池块数
	 *  @param blockSize :内存池每块大小
	 *  @return  
	 */
	void *allocBlock( unsigned int blockNum, unsigned int blockSize );

	void *alloc(unsigned int size );
	bool  free( void *pBlock );
private:
	KernalList< KernalMemoryBlock > m_KernalMemoryBlocks;
};

#endif
