#include "luaregister.h"

namespace tadpole{
#define LUA_FUNC(XX)\
	XX(log_info)\
	XX(log_error)\
	XX(log_debug)\
	XX(log_warn)\
	XX(log_fatal)\
	XX(iom_create)\
	XX(iom_schedule)\
	XX(iom_add_timer)\
	XX(iom_sleep)
/**
 * @根据日志级别打印不同级别的日志
 */
#define TADPOLE_LOG_LUA_LEVEL(level,name,file,line) \
	tadpole::EventWrap::ptr(new tadpole::EventWrap(name,level,tadpole::LogEvent::ptr(new tadpole::LogEvent(line\
										 ,tadpole::GetThreadId(),tadpole::GetFiberId()\
										 ,time(NULL),tadpole::GetElapse(),file))))->getSS()


/**
 * @brief 分别输出不同级别的日志，用logger输出；
 */
#define TADPOLE_LOG_LUA_DEBUG(logger,file,line) TADPOLE_LOG_LUA_LEVEL(tadpole::LogLevel::DEBUG,logger,file,line)

#define TADPOLE_LOG_LUA_INFO(logger,file,line) TADPOLE_LOG_LUA_LEVEL(tadpole::LogLevel::INFO,logger,file,line)

#define TADPOLE_LOG_LUA_WARN(logger,file,line) TADPOLE_LOG_LUA_LEVEL(tadpole::LogLevel::WARN,logger,file,line)

#define TADPOLE_LOG_LUA_ERROR(logger,file,line) TADPOLE_LOG_LUA_LEVEL(tadpole::LogLevel::ERROR,logger,file,line)

#define TADPOLE_LOG_LUA_FATAL(logger,file,line) TADPOLE_LOG_LUA_LEVEL(tadpole::LogLevel::FATAL,logger,file,line)

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

static LuaStack::ptr g_lua = nullptr; 

/*
 * lua function 
 */
int log_info(lua_State * L){
	std::string str; 
	lua_Debug ar; 
	tadpole::LuaStack::GetCFuncArgs(L,str);
	lua_getstack(L,1,&ar);
	lua_getinfo(L,"Sl",&ar);
	TADPOLE_LOG_LUA_INFO(g_logger,ar.source,ar.currentline)<< str;	
	return 0 ; 
}

int log_debug(lua_State * L){
	std::string str; 
	lua_Debug ar; 
	tadpole::LuaStack::GetCFuncArgs(L,str);
	lua_getstack(L,1,&ar);
	lua_getinfo(L,"Sl",&ar);
	TADPOLE_LOG_LUA_DEBUG(g_logger,ar.source,ar.currentline)<< str;	
	return 0 ; 
}

int log_error(lua_State * L){
	std::string str; 
	lua_Debug ar; 
	tadpole::LuaStack::GetCFuncArgs(L,str);
	lua_getstack(L,1,&ar);
	lua_getinfo(L,"Sl",&ar);
	TADPOLE_LOG_LUA_ERROR(g_logger,ar.source,ar.currentline)<< str;	
	return 0 ; 
}


int log_warn(lua_State * L){
	std::string str; 
	lua_Debug ar; 
	tadpole::LuaStack::GetCFuncArgs(L,str);
	lua_getstack(L,1,&ar);
	lua_getinfo(L,"Sl",&ar);
	TADPOLE_LOG_LUA_WARN(g_logger,ar.source,ar.currentline)<< str;	
	return 0 ; 
}

int log_fatal(lua_State * L){
	std::string str; 
	lua_Debug ar; 
	tadpole::LuaStack::GetCFuncArgs(L,str);
	lua_getstack(L,1,&ar);
	lua_getinfo(L,"Sl",&ar);
	TADPOLE_LOG_LUA_FATAL(g_logger,ar.source,ar.currentline)<< str;	
	return 0 ; 
}

//end 
static std::vector<IOManager::ptr> m_ioms; 

int iom_create(lua_State* L){
	int threadcount = 0 ; 
	bool usecaller = false;
	std::string name = "";
	tadpole::LuaStack::GetCFuncArgs(L,threadcount,usecaller,name);
	IOManager::ptr iom(new IOManager(threadcount,usecaller,name));
	m_ioms.push_back(iom);
	int size = m_ioms.size();
	tadpole::LuaStack::SetCFuncRet(L,size);
	return 1;
}

int iom_schedule(lua_State * L){
	int index = 0 ; 
	std::string funcname = "";
	tadpole::LuaStack::GetCFuncArgs(L,index,funcname);
	auto fun = [funcname](){
		g_lua->runLuaFunc(funcname,0);
	};

	//TADPOLE_LOG_ERROR(g_logger)<<index << " "<< funcname;
	if(index > (int)m_ioms.size()){
		TADPOLE_LOG_ERROR(g_logger)<< "m_ions out of range ";
		return -1 ; 
	}
	m_ioms[index-1]->schedule(fun);
	return 0 ; 
}

int iom_add_timer(lua_State * L){
	int index = 0 ; 
	int ms = 0 ; 
	std::string funcname = "";

	tadpole::LuaStack::GetCFuncArgs(L,index,ms,funcname);
	auto fun = [funcname](){
		g_lua->runLuaFunc(funcname,0);
	};

	//TADPOLE_LOG_ERROR(g_logger)<<index << " "<< funcname;
	if(index > (int)m_ioms.size()){
		TADPOLE_LOG_ERROR(g_logger)<< "m_ions out of range ";
		return -1 ; 
	}
	m_ioms[index-1]->addTimer(ms,fun);
	return 0 ; 
}

int iom_sleep(lua_State * L){
	int s = 0 ; 
	tadpole::LuaStack::GetCFuncArgs(L,s);
	sleep(s);
	return 0; 
}

struct LuaRegisterIniter{
	LuaRegisterIniter(){
		g_lua.reset(new tadpole::LuaStack);
#define XX(name)\
		g_lua->luaRegister(#name,name);
		LUA_FUNC(XX)
#undef XX
	}
};

static LuaRegisterIniter __luaRegIni;  

LuaStack::ptr GetLuaStack(){
	return g_lua; 
}

void RunLua(const std::string & filename , const std::string & main){
	if(!g_lua){
		return ; 
	}
	if(filename.empty()){
		TADPOLE_LOG_INFO(g_logger)<< "Lua filename is empty !";
	}

	g_lua->loadFile(filename);
	g_lua->runLuaFunc(main,0);
}

}
