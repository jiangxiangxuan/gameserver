
#include "LuaKernalLock.h"
#include "KernalLock.h"

static int lua_CreateKernalMutexLocker(lua_State* L)
{
	KernalMutexLocker *pData = (KernalMutexLocker*)lua_newuserdata( L, sizeof(KernalMutexLocker) );
    luaL_getmetatable(L, "Locker");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_DestoryKernalMutexLocker(lua_State* L)
{
	KernalMutexLocker *pT = (KernalMutexLocker*)luaL_checkudata( L, 1, "Locker" );
    //delete *(KernalMutexLocker**)lua_topointer(L, 1);
    return 0;
}

static int lua_CallLock(lua_State* L)
{
	KernalMutexLocker *pT = (KernalMutexLocker*)luaL_checkudata( L, 1, "Locker" );
    lua_pushinteger(L, pT->lock());
    return 1;
}

static int lua_Callunlock(lua_State* L)
{
	KernalMutexLocker *pT = (KernalMutexLocker*)luaL_checkudata( L, 1, "Locker" );
    lua_pushinteger(L, pT->unlock());
    return 1;
}

static int lua_KernalMutexLockerindex(lua_State* L)
{
	KernalMutexLocker *pT = (KernalMutexLocker*)luaL_checkudata( L, 1, "Locker" );

    if (strcmp(lua_tostring(L, 2), "lock") == 0)
    {
        lua_pushcfunction(L, lua_CallLock);
    }
    else if (strcmp(lua_tostring(L, 2), "unlock") == 0)
    {
        lua_pushcfunction(L, lua_Callunlock);
    }
    else
    {
        return 0;
    }
    return 1;
}

bool loadKernalMutexLocker(lua_State* L)
{
    lua_pushcfunction(L, lua_CreateKernalMutexLocker);
    lua_setglobal(L, "Locker");

    lua_pushcfunction(L, lua_DestoryKernalMutexLocker);
    lua_setglobal(L, "DestoryLocker");

    luaL_newmetatable(L, "Locker");

    lua_pushstring(L, "__gc"); 
    lua_pushcfunction(L, lua_DestoryKernalMutexLocker);
    lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_KernalMutexLockerindex);
    lua_settable(L, -3);
}
