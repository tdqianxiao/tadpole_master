#include "src/luaregister.h"

int main (){
	std::string file = "lua/main.lua";
	std::string main = "main";
	tadpole::RunLua(file,main);
	return 0; 
}
