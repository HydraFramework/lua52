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

#define STATEDATA_TO_CO(L) ((StateData *)((unsigned char *)L - LUAI_EXTRASPACE))->co
#define STATEDATA_TO_ISPAGE(L) ((StateData *)((unsigned char *)L - LUAI_EXTRASPACE))->isPage

#ifndef GLOBAL_LOCK
#define STATEDATA_TOLOCK(L) ((StateData *)((unsigned char *)GetMainState(L) - LUAI_EXTRASPACE))->lock
#else
pthread_mutex_t lock;
int lockInited = 0;
#define STATEDATA_TOLOCK(L) lock
#endif

#define STATEDATA_TO_COUNT(L) ((StateData *)((unsigned char *)GetMainState(L) - LUAI_EXTRASPACE))->count
#define STATEDATA_TO_COUNT_PAGE(L) ((StateData *)((unsigned char *)GetPageState(L) - LUAI_EXTRASPACE))->count

lua_State* GetMainState(lua_State *L){
    return G(L)->mainthread;
}

lua_State* GetPageState(lua_State *L){
    if (STATEDATA_TO_ISPAGE(L)) {
        return STATEDATA_TO_CO(L);
    } else {
        return GetMainState(L);
    }
}

void LockMainState(lua_State *L){
    pthread_mutex_lock(&STATEDATA_TOLOCK(L));
}

void UnLockMainState(lua_State *L){
    pthread_mutex_unlock(&STATEDATA_TOLOCK(L));
}

void MarkAsPage(lua_State *L){
    STATEDATA_TO_ISPAGE(L) = 1;
    STATEDATA_TO_CO(L) = L;
}

void LuaLockInitial(lua_State * L){
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
    
    STATEDATA_TO_COUNT(L) = 0;
    STATEDATA_TO_CO(L) = L;
    STATEDATA_TO_ISPAGE(L) = 0;
}

void LuaLockInitialThread(lua_State * L, lua_State * co){
    //copy page thread reference to the child.
    if (STATEDATA_TO_ISPAGE(L)) {
        STATEDATA_TO_ISPAGE(co) = 1;
        STATEDATA_TO_CO(co) = STATEDATA_TO_CO(L);
    } else {
        STATEDATA_TO_ISPAGE(co) = 0;
        STATEDATA_TO_CO(co) = L;
        ((StateData *)((unsigned char *)co - LUAI_EXTRASPACE))->count = 0;
    }
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
    if (!STATEDATA_TO_ISPAGE(L)) {
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
    } else {
#ifdef DEBUG
//        lua_getfield(L, LUA_REGISTRYINDEX, "dataPath");
//        if (lua_type(L, -1) == LUA_TSTRING) {
//            LOGD("0x%08lX(0x%08lX)\t%s incrRef %d -> %d\n", (long)L, (long)GetPageState(L), lua_tostring(L, -1), STATEDATA_TO_COUNT_PAGE(L), STATEDATA_TO_COUNT_PAGE(L) + 1);
//        }
//        lua_pop(L, 1);
#endif
        
        STATEDATA_TO_COUNT(L)++;
        STATEDATA_TO_COUNT_PAGE(L)++;
    }
    lua_unlock(L);
}

void decrRef(lua_State *L){
    lua_State *ROOT = GetMainState(L);
    lua_lock(ROOT);
    
    if (!STATEDATA_TO_ISPAGE(L)) {
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
    } else {
#ifdef DEBUG
//        lua_getfield(L, LUA_REGISTRYINDEX, "dataPath");
//        if (lua_type(L, -1) == LUA_TSTRING) {
//            LOGD("0x%08lX(0x%08lX)\t%s decrRef %d -> %d\n", (long)L, (long)GetPageState(L), lua_tostring(L, -1), STATEDATA_TO_COUNT_PAGE(L), STATEDATA_TO_COUNT_PAGE(L) - 1);
//        }
//        lua_pop(L, 1);
#endif
        
        STATEDATA_TO_COUNT(L)--;
        STATEDATA_TO_COUNT_PAGE(L)--;
    }
    
    lua_unlock(ROOT);
    
    if (STATEDATA_TO_COUNT_PAGE(L) <= 0) {
        lua_pushthread(L);
        lua_xmove(L, ROOT, 1);
        
        lua_pushnil(ROOT);
        lua_rawset(ROOT, LUA_REGISTRYINDEX);
        
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