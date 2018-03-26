
#ifndef _LUA_KERNAL_LOCKER_H
#define _LUA_KERNAL_LOCKER_H

#include <string.h>
extern "C"{
	#include <lua/lua.hpp>  
}
bool loadKernalMutexLocker(lua_State* L);

#endif
