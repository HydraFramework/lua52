//
//  luauser.h
//  EOSLib2
//
//  Created by Sam on 6/7/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef EOSLib2_luauser_h
#define EOSLib2_luauser_h

#define GLOBAL_LOCK

#undef luai_userstateopen
#define luai_userstateopen(L)       LuaLockInitial(L)

#undef luai_userstatethread
#define luai_userstatethread(L,L1)  LuaLockInitialThread(L,L1)

#undef luai_userstateclose
#define luai_userstateclose(L)      LuaLockFinalState(L)

#undef luai_userstatefree
#define luai_userstatefree(L,L1)      LuaLockFinalThread(L,L1)

#include <pthread.h>

typedef struct StateData StateData;

struct StateData{
#ifndef GLOBAL_LOCK
    pthread_mutex_t lock;
#endif
    volatile int count;
    lua_State *main;
};

#undef LUAI_EXTRASPACE
#define LUAI_EXTRASPACE sizeof(StateData)

#define lua_lock(L)    LockMainState(L)
#define lua_unlock(L)  UnLockMainState(L)

void LuaLockInitial(lua_State * L);
void LuaLockInitialThread(lua_State * L, lua_State * co);
void LuaLockFinalState(lua_State * L);
void LuaLockFinalThread(lua_State * L, lua_State *co);

void incrRef(lua_State *L);
void decrRef(lua_State *L);

lua_State* GetMainState(lua_State *L);

void LockMainState(lua_State *L);
void UnLockMainState(lua_State *L);

void LuaError(lua_State *L);

LUALIB_API int luaL_typerror (lua_State *L, int narg, const char *tname);
#endif
