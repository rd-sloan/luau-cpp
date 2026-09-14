#pragma once
#include "lua.h"

/** Luau Board Function Bindings **/

int l_getBoard(lua_State* luaState);
int l_getEmptyMarker(lua_State* luaState);
int l_getHumanMove(lua_State* luaState);
int l_wouldWin(lua_State* luaState);