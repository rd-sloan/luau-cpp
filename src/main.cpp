#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "lua.h"
#include "lualib.h"
#include "luacode.h"

static std::string readFile(const std::string& path)
{
	std::ifstream file(path);
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

int main()
{
	std::string source = readFile(std::string(SCRIPTS_DIR) + "/script.luau");

	// compile source code into bytecode
	size_t bytecodeSize = 0;
	char* bytecode = luau_compile(source.c_str(), source.size(), nullptr, &bytecodeSize);

	// Create a Luau VM state
	lua_State* luaState = luaL_newstate();
	luaL_openlibs(luaState); // opens standard library (print, math, string, etc)

	// Load compiled bytecode into VM
	int result = luau_load(luaState, "script", bytecode, bytecodeSize, 0);
	free(bytecode);

	if (result != 0)
	{
		std::fprintf(stderr, "failed to load script:%s\n", lua_tostring(luaState, -1));
		lua_close(luaState);
		return 1;
	}

	// run the luaa script
	if (lua_pcall(luaState, 0, LUA_MULTRET, 0) != LUA_OK)
	{
		std::fprintf(stderr, "Runtime error: %s\n", lua_tostring(luaState, -1));
		lua_close(luaState);
		return 1;
	}

	lua_close(luaState);
	return 0;
}