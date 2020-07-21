#ifndef __TADPOLE_LUAREGISTER_H__
#define __TADPOLE_LUAREGISTER_H__

#include "src/log.h"
#include "src/luastack.h"
#include "src/iomanager.h"
#include <string>

namespace tadpole{

LuaStack::ptr GetLuaStack();

void RunLua(const std::string & filename , const std::string & main);

int log_info(lua_State * L);
int log_debug(lua_State * L);
int log_warn(lua_State * L);
int log_fatal(lua_State * L);
int log_error(lua_State * L);

int iom_create(lua_State * L);
int iom_schedule(lua_State * L);
int iom_add_timer(lua_State * L);
int iom_sleep(lua_State * L);

}

#endif 
