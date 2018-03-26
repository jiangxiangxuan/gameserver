
#ifndef _KERNAL_MAP_H_
#define _KERNAL_MAP_H_

#include <stdio.h>
#include <map>
#include "KernalLock.h"

template< typename K, typename V >
class KernalMap
{
public:
	KernalMap();
	virtual ~KernalMap();

	void insert( K key, V value );
	void erase( K key );
	V find( K key );
	int size()
	{
		return m_maps.size();
	}
	typename std::map<K,V>::iterator begin()
	{
		return m_maps.begin();
	}
	typename std::map<K,V>::iterator end()
	{
		return m_maps.end();
	}
private:
	std::map<K,V>        m_maps;
	KernalMutexLocker    m_locker;
};

template< typename K, typename V >
KernalMap< K, V >::KernalMap()
{

}

template< typename K, typename V >
KernalMap< K, V >::~KernalMap()
{

}

template< typename K, typename V >
void KernalMap< K, V >::insert( K key, V value )
{
	m_locker.lock();

	m_maps.insert( std::make_pair(key, value) );

	m_locker.unlock();
}

template< typename K, typename V >
void KernalMap< K, V >::erase( K key )
{
	m_locker.lock();

	auto iter = m_maps.find( key );
	if( iter != m_maps.end() )
	{
		m_maps.erase( iter );
	}

	m_locker.unlock();
}

template< typename K, typename V >
V KernalMap< K, V >::find( K key )
{
	V v = NULL;
	auto iter = m_maps.find( key );
	if( iter != m_maps.end() )
	{
		v = (*iter).second;
	}
	return v;
}

#endif
