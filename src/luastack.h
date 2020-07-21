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
	 * @brief ģ��ƫ�ػ���push nil 
	 */
	template<class T>
	class LuaPush {
		public:
		void operator()(lua_State* L, T val) {
			lua_pushnil(L);
		}
	};

	/**
	 * @brief ģ��ƫ�ػ���push int 
	 */
	template<>
	class LuaPush<int> {
	public:
		void operator()(lua_State * L,int val) {
			lua_pushinteger(L, val);
		}
	};

	/**
	 * @brief ģ��ƫ�ػ���push string 
	 */
	template<>
	class LuaPush<std::string> {
	public:
		void operator()(lua_State* L, const std::string & str) {
			lua_pushstring(L, str.c_str());
		}
	};

	/**
	 * @brief ģ��ƫ�ػ���push bool 
	 */
	template<>
	class LuaPush<bool> {
	public:
		void operator()(lua_State* L, bool b) {
			lua_pushboolean(L,b);
		}
	};

	/**
	 * @brief ģ��ƫ�ػ���push double 
	 */
	template<>
	class LuaPush<double> {
	public:
		void operator()(lua_State* L, double dou) {
			lua_pushnumber(L,dou);
		}
	};

	/**
	 * @brief ��������
	 */
	template <class T>
	class SetArg {
	public:
		void operator()(lua_State* L,int pos,T& val) {
			
		}
	};

	/**
	 * @brief ģ��ƫ�ػ���set string 
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
	 * @brief ģ��ƫ�ػ���set int 
	 */
	template <>
	class SetArg<int>{
	public:
		void operator()(lua_State* L, int pos,int& val) {
			val = luaL_checkinteger(L, pos);
		}
	};

	/**
	 * @brief ģ��ƫ�ػ���set double 
	 */
	template <>
	class SetArg<double> {
	public:
		void operator()(lua_State* L, int pos, double& val) {
			val = luaL_checknumber(L, pos);
		}
	};

	/**
	 * @brief lua ջ
	 */
	class LuaStack {
	public:
		/**
		 * @brief ���Ͷ��壬����ָ��
		 */
		typedef std::shared_ptr<LuaStack> ptr;
	public:

		/**
		 * @brief ���캯��
		 */
		LuaStack();

		/**
		 * @brief ��������
		 */
		~LuaStack();

		/**
		 * @brief ����luaԴ�ļ�
		 */
		int loadFile(const std::string& name);

		/**
		 * @brief ����tuple ,��tuple��Ԫ��ѹ��luaջ��
		 * @param[out] argv ��������
		 * @param[in] L luaջָ��
		 * @param[in] arg ���в���
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
		 * @brief ִ��lua�����еĺ�����
		 * @param[in] func ������
		 * @param[in] args ��������
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
		 * @brief ���ǰһ��luaFunc�ķ���ֵ
		 */
		template <typename... Args>
		int getPreLuaFuncRet(Args&... args){
			return GetCFuncArgs(m_state,args...);
		}

		/**
		 * @brief ���������Ϣ
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

		//���c��������
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

		//����c��������ֵ
		template <class... Args>
		static int SetCFuncRet(lua_State* L,Args... args) {
			int count = 0; 
			ForwardSetRet(L,count,args...);
			return count; 
		}

	public:
		/**
		 * @brief luaջָ��
		 */
		lua_State* m_state = nullptr;

		/** 
		 * @brief ������
		 */
		std::vector<luaL_Reg> m_regs;

		/**
		 * @brief ��������
		 */
		int m_count = 0; 
	};
}

#endif 
