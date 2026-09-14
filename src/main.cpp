#pragma once

#include "board.h"
#include "LuaBindings.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include "lualib.h"
#include "luacode.h"


bool fileExists(const std::string& path)
{
	std::ifstream f(path);
	return f.good();
}

std::string readFile(const std::string& path)
{
	std::ifstream file(path);
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}


lua_State* SetupPlayerState(const std::string& scriptName)
{
	// Create a Luau VM state
	lua_State* luaState = luaL_newstate();
	luaL_openlibs(luaState); // opens standard library (print, math, string, etc)

	// luau bindings

	lua_pushcfunction(luaState, l_getBoard, "getBoard", 1);
	lua_setglobal(luaState, "getBoard");

	lua_pushcfunction(luaState, l_getHumanMove, "getHumanMove");
	lua_setglobal(luaState, "getHumanMove");

	lua_pushcfunction(luaState, l_getEmptyMarker, "getEmptyMarker");
	lua_setglobal(luaState, "getEmptyMarker");
	
	lua_pushcfunction(luaState, l_wouldWin, "wouldWin");
	lua_setglobal(luaState, "wouldWin");

	// compile source code into bytecode
	std::string source = readFile(scriptName);
	size_t bytecodeSize = 0;
	char* bytecode = luau_compile(source.c_str(), source.size(), nullptr, &bytecodeSize);

	// Load compiled bytecode into VM
	int result = luau_load(luaState, "script", bytecode, bytecodeSize, 0);
	free(bytecode);

	if (result != 0)
	{
		std::fprintf(stderr, "failed to load script:%s\n", lua_tostring(luaState, -1));
		lua_close(luaState);
		return nullptr;
	}

	// run the top-level code once - this defines getMove() etc as globals
	if (lua_pcall(luaState, 0, LUA_MULTRET, 0) != LUA_OK) {
		std::fprintf(stderr, "Error running %s: %s\n", scriptName.c_str(), lua_tostring(luaState, -1));
		lua_close(luaState);
		return nullptr;
	}

	return luaState;
}

// todo: can the player num be saved on the lua state?
uint8_t GetMoveFromScript(lua_State* luaState, int playerNum)
{
	lua_getglobal(luaState, "getMove");					// push function
	lua_pushinteger(luaState, playerNum);				// push arg (player number)
	lua_pcall(luaState, /*nargs*/1, /*nresults*/1, 0);	// run code
	uint8_t col = (uint8_t)lua_tointeger(luaState, -1);			
	lua_pop(luaState, 1);								// pop the return value
	return col;
}



int main(int argc, char** argv)
{
	// Defaults 
	std::string player1Script = std::string(SCRIPTS_DIR) + "/winBlockCenterWeight.luau";
	std::string player2Script = std::string(SCRIPTS_DIR) + "/human.luau";

	// Arguments
	if (argc >= 2)
	{
		player1Script = argv[1];
	}
	if (argc >= 3) 
	{
		player2Script = argv[2];
	}

	if (!fileExists(player1Script) || !fileExists(player2Script)) 
	{
		std::fprintf(stderr, "Usage: %s [player1Script] [player2Script]\n", argv[0]);
		std::fprintf(stderr, "Could not find one or both script files.\n");
		return 1;
	}

	// init board
	InitBoard(g_board);

	// init players
	lua_State* player1L = SetupPlayerState(player1Script);
	lua_State* player2L = SetupPlayerState(player2Script);

	if (!player1L || !player2L)
	{
		std::fprintf(stderr, "Missing players, aborting game\n");
		return 1;
	}

	// Intro text
	std::fprintf(stdout, "===================================\n");
	std::fprintf(stdout, "=            CONNECT 4            =\n");
	std::fprintf(stdout, "===================================\n");
	std::fprintf(stdout, "\n");

	// Game loop
	bool endGame = false;
	bool player1Turn = true;
	while (!endGame)
	{
		lua_State* current = player1Turn ? player1L : player2L;
		uint8_t playerNum = player1Turn ? 1 : 2;

		// get and play the player's move
		std::fprintf(stdout, "Player %i (%s) enter your move:\n", playerNum, GetPieceDisplayString(playerNum).c_str());
		uint8_t col = GetMoveFromScript(current, playerNum);
		std::fprintf(stdout, "Player %i (%s) plays column %i\n", playerNum, GetPieceDisplayString(playerNum).c_str(), col);
		TryDropPiece(g_board, col - 1, playerNum); // -1 because luau uses 1-7 instead of 0-6
		std::fprintf(stdout, GetBoardDisplayString(g_board).c_str());

		// check for win
		if (CheckWin(g_board, playerNum))
		{
			std::fprintf(stdout, "Player %i (%s) wins!!!!!!", playerNum, playerNum == 1 ? player1Script.c_str() : player2Script.c_str());
			endGame = true;		// since we break this isn't really necessary, but just in case.
			break;
		}

		if (CheckTie(g_board))
		{
			std::fprintf(stdout, "It's a tie, you both lose!");
			endGame = true;		// since we break this isn't really necessary, but just in case.
			break;
		}

		// swap turns
		player1Turn = !player1Turn;
	}

	
	lua_close(player1L);
	lua_close(player2L);

	return 0;
}

