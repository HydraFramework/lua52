#include <stdio.h>
#include "lua.h"
#include "luauser.h"
#include "lauxlib.h"
#include "lstate.h"

#ifdef __ANDROID__

#include <android/log.h>
#define LOG_TAG "lua.print"
#undef LOG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR , LOG_TAG,__VA_ARGS__)

#else

#define LOGD(...) fprintf(stdout, __VA_ARGS__)
#define LOGE(...) fprintf(stderr, __VA_ARGS__)

#endif

#define STATEDATA_TO_MAIN(L) ((StateData *)((unsigned char *)L - LUAI_EXTRASPACE))->main

#ifndef GLOBAL_LOCK
#define STATEDATA_TOLOCK(L) ((StateData *)((unsigned char *)STATEDATA_TO_MAIN(L) - LUAI_EXTRASPACE))->lock
#else
pthread_mutex_t lock;
int lockInited = 0;
#define STATEDATA_TOLOCK(L) lock
#endif

#define STATEDATA_TO_COUNT(L) ((StateData *)((unsigned char *)STATEDATA_TO_MAIN(L) - LUAI_EXTRASPACE))->count

lua_State* GetMainState(lua_State *L){
    if (L) {
        return STATEDATA_TO_MAIN(L);
    } else {
        return NULL;
    }
}

void LockMainState(lua_State *L){
    if (L) {
        pthread_mutex_lock(&STATEDATA_TOLOCK(L));
    }
}

void UnLockMainState(lua_State *L){
    if (L) {
        pthread_mutex_unlock(&STATEDATA_TOLOCK(L));
    }
}

void LuaLockInitial(lua_State * L){
    STATEDATA_TO_MAIN(L) = L;
    STATEDATA_TO_COUNT(L) = 0;

#ifdef GLOBAL_LOCK
    if (!lockInited) {
#endif
        pthread_mutexattr_t a;
        pthread_mutexattr_init(&a);
        pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&STATEDATA_TOLOCK(L), &a);
        
        lockInited = 1;
#ifdef GLOBAL_LOCK
    }
#endif
}

void LuaLockInitialThread(lua_State * L, lua_State * co){
    STATEDATA_TO_MAIN(co) = STATEDATA_TO_MAIN(L);
    
#ifdef DEBUG
//    LOGD("initialThread 0x%08lX\n", (long)co);
#endif
}

void LuaLockFinalState(lua_State * L){
#ifndef GLOBAL_LOCK
    pthread_mutex_destroy(&STATEDATA_TOLOCK(L));
#endif
}

void LuaLockFinalThread(lua_State * L, lua_State * co){
#ifdef DEBUG
//    LOGD("final thread 0x%08lX\n", (long)co);
#endif
}

void incrRef(lua_State *L){
    lua_lock(L);
#ifdef DEBUG
//        lua_getfield(L, LUA_REGISTRYINDEX, "dataPath");
//        if (lua_type(L, -1) == LUA_TSTRING) {
//            LOGD("0x%08lX(ROOT)\t%s incrRefRoot %d -> %d\n", (long)L, lua_tostring(L, -1), STATEDATA_TO_COUNT(L), STATEDATA_TO_COUNT(L) + 1);
//        } else {
//            LOGD("0x%08lX(ROOT)\t incrRefRoot %d -> %d\n", (long)L, STATEDATA_TO_COUNT(L), STATEDATA_TO_COUNT(L) + 1);
//        }
//        lua_pop(L, 1);
#endif
        
    STATEDATA_TO_COUNT(L)++;
    lua_unlock(L);
}

void decrRef(lua_State *L){
    lua_State *ROOT = GetMainState(L);
    lua_lock(ROOT);
    
#ifdef DEBUG
//        lua_getfield(L, LUA_REGISTRYINDEX, "dataPath");
//        if (lua_type(L, -1) == LUA_TSTRING) {
//            LOGD("0x%08lX(ROOT)\t%s decrRefRoot %d -> %d\n", (long)L, lua_tostring(L, -1), STATEDATA_TO_COUNT(L), STATEDATA_TO_COUNT(L) - 1);
//        } else {
//            LOGD("0x%08lX(ROOT)\t decrRefRoot %d -> %d\n", (long)L, STATEDATA_TO_COUNT(L), STATEDATA_TO_COUNT(L) - 1);
//        }
//        lua_pop(L, 1);
#endif
        
    STATEDATA_TO_COUNT(L)--;
    
    lua_unlock(ROOT);
    
    if (STATEDATA_TO_COUNT(L) == 1) {
        lua_gc(ROOT, LUA_GCCOLLECT, 0);
    }
    
    if (STATEDATA_TO_COUNT(ROOT) <= 0) {
        lua_close(ROOT);
        printf("dealloc state\n");
    }
}

LUALIB_API int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}