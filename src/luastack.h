#ifndef __TADPOLE_LUASTACK_H__
#define __TADPOLE_LUASTACK_H__

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <string>
#include <iostream>
#include <memory>
#include <tuple>
#include <algorithm>
#include <type_traits>
#include <vector>
#include <functional>

namespace tadpole {
	/**
	 * @brief 模板偏特化，push nil 
	 */
	template<class T>
	class LuaPush {
		public:
		void operator()(lua_State* L, T val) {
			lua_pushnil(L);
		}
	};

	/**
	 * @brief 模板偏特化，push int 
	 */
	template<>
	class LuaPush<int> {
	public:
		void operator()(lua_State * L,int val) {
			lua_pushinteger(L, val);
		}
	};

	/**
	 * @brief 模板偏特化，push string 
	 */
	template<>
	class LuaPush<std::string> {
	public:
		void operator()(lua_State* L, const std::string & str) {
			lua_pushstring(L, str.c_str());
		}
	};

	/**
	 * @brief 模板偏特化，push bool 
	 */
	template<>
	class LuaPush<bool> {
	public:
		void operator()(lua_State* L, bool b) {
			lua_pushboolean(L,b);
		}
	};

	/**
	 * @brief 模板偏特化，push double 
	 */
	template<>
	class LuaPush<double> {
	public:
		void operator()(lua_State* L, double dou) {
			lua_pushnumber(L,dou);
		}
	};

	/**
	 * @brief 不做操作
	 */
	template <class T>
	class SetArg {
	public:
		void operator()(lua_State* L,int pos,T& val) {
			
		}
	};

	/**
	 * @brief 模板偏特化，set string 
	 */
	template <>
	class SetArg<std::string>{
	public:
		void operator()(lua_State* L, int pos, std::string & val) {
			const char * str = luaL_checkstring(L, pos);
			val = std::string(str);
		}
	};
	
	/**
	 * @brief 模板偏特化，set int 
	 */
	template <>
	class SetArg<int>{
	public:
		void operator()(lua_State* L, int pos,int& val) {
			val = luaL_checkinteger(L, pos);
		}
	};

	/**
	 * @brief 模板偏特化，set double 
	 */
	template <>
	class SetArg<double> {
	public:
		void operator()(lua_State* L, int pos, double& val) {
			val = luaL_checknumber(L, pos);
		}
	};

	/**
	 * @brief lua 栈
	 */
	class LuaStack {
	public:
		/**
		 * @brief 类型定义，智能指针
		 */
		typedef std::shared_ptr<LuaStack> ptr;
	public:

		/**
		 * @brief 构造函数
		 */
		LuaStack();

		/**
		 * @brief 析构函数
		 */
		~LuaStack();

		/**
		 * @brief 加载lua源文件
		 */
		int loadFile(const std::string& name);

		/**
		 * @brief 遍历tuple ,将tuple内元素压入lua栈中
		 * @param[out] argv 参数个数
		 * @param[in] L lua栈指针
		 * @param[in] arg 所有参数
		 */
		template <class T, class... Args>
		void forEach(int& argv,lua_State* L,const T& val , Args... args) {
			++argv;
			LuaPush<T>()(L, val);
			forEach(argv, L, args...);
		}

		void forEach(int& argv, lua_State* L) {
			return; 
		}

		/**
		 * @brief 执行lua代码中的函数，
		 * @param[in] func 函数名
		 * @param[in] args 其它参数
		 */
		template <typename... Args>
		int runLuaFunc(const std::string& func,int retCount, Args... args){
			if (!func.empty()) {
				lua_getglobal(m_state, func.c_str());

				if (!lua_isfunction(m_state, -1)) {
					std::cout << "Error func is not fount : "<< func<< std::endl;
					return -1;
				}
			}

			int argv = 0;

			forEach(argv, m_state, args...);

			return lua_pcall(m_state, argv, retCount, 0);
		}
	
		/**
		 * @brief 获得前一个luaFunc的返回值
		 */
		template <typename... Args>
		int getPreLuaFuncRet(Args&... args){
			return GetCFuncArgs(m_state,args...);
		}

		/**
		 * @brief 输出错误信息
		 */
		std::ostream& LogError(std::ostream& os);

		void luaRegister(const std::string& name, lua_CFunction func);

		template <class T>
		static void ForwardSetArg(lua_State* L, int& pos) {
			return;
		}

		template <class T>
		static void ForwardSetArg(lua_State* L, int& pos, T& val) {
			SetArg<T>()(L, pos, val);
			return;
		}

		template <class T,class... Args>
		static void ForwardSetArg(lua_State* L, int &pos,T& val,Args&... args) {
			SetArg<T>()(L, pos, val);
			++pos;
			ForwardSetArg(L, pos, args...);
		}

		//获得c函数参数
		template <class... Args>
		static int GetCFuncArgs(lua_State* L,Args&... args ) {
			int pos = 1; 
			ForwardSetArg(L, pos, args...);
			return pos;
		}

		static void ForwardSetRet(lua_State* L, int& count) {
			return; 
		}

		template <class T>
		static void ForwardSetRet(lua_State* L, int& count, T val) {
			LuaPush<T>()(L, val);
			++count;
		}

		template <class T, class... Args>
		static void ForwardSetRet(lua_State* L, int& count, T val, Args... args) {
			LuaPush<T>()(L, val);
			++count; 
			ForwardSetRet(L,count,args...);
		}

		//设置c函数返回值
		template <class... Args>
		static int SetCFuncRet(lua_State* L,Args... args) {
			int count = 0; 
			ForwardSetRet(L,count,args...);
			return count; 
		}

	public:
		/**
		 * @brief lua栈指针
		 */
		lua_State* m_state = nullptr;

		/** 
		 * @brief 函数表
		 */
		std::vector<luaL_Reg> m_regs;

		/**
		 * @brief 函数数量
		 */
		int m_count = 0; 
	};
}

#endif 
