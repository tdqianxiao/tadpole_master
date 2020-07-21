#include "luastack.h"

namespace tadpole {

	LuaStack::LuaStack() {
		m_state = luaL_newstate();
		luaL_openlibs(m_state);
	}

	int LuaStack::loadFile(const std::string& name) {
		int ret = luaL_loadfile(m_state, name.c_str());
		if (ret != 0) {
			LogError(std::cout);
		}
		runLuaFunc("",0);
		return ret; 
	}
	
	

	std::ostream& LuaStack::LogError(std::ostream & os) {
		lua_type(m_state, -1);
		const char* err = lua_tostring(m_state, -1);
		os <<"Error: %s"<<err;
		lua_pop(m_state, 1);
		return os; 
	}

	LuaStack::~LuaStack() {
		lua_close(m_state);
	}

	void LuaStack::luaRegister(const std::string& name,lua_CFunction func) {
		lua_register(m_state, name.c_str(), func);
	}

}
