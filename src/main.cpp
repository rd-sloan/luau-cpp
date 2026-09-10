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


class Board
{
public:
	// todo instead of class functions lets just make these functions that take in a board.
	void Init()
	{

		for (int row = 0; row < NumberOfRows; ++row)
		{
			for (int col = 0; col < NumberOfColumns; ++col)
			{
				m_board[row][col] = EmptyMarker;
			}
		}

		for (int i = 0; i < NumberOfColumns; ++i)
		{
			m_fullColumns[i] = false;
		}
	}

	std::string GetDisplayString()
	{
		std::string displayString;
		for (int row = 0; row < NumberOfRows; ++row)
		{
			for (int col = 0; col < NumberOfColumns; ++col)
			{
				displayString += std::to_string(m_board[row][col]) + " ";
			}
			displayString += "\n";
		}

		return displayString;
	}

	bool IsColumnFull(uint8_t colNumber)
	{
		if (colNumber < NumberOfColumns)
		{
			return m_fullColumns[colNumber];
		}

		return true; // if column doesn't exist its technically fully
	}

	bool TryPlayMove(uint8_t colNumber, uint8_t playerNum)
	{
		if (colNumber >= NumberOfColumns)
		{
			std::fprintf(stderr, "invalid colNum %i", colNumber);
			return false;
		}

		if (playerNum != 1 && playerNum != 2)
		{
			std::fprintf(stderr, "invalid playerNum %i", playerNum);
			return false;
		}

		if (IsColumnFull(colNumber))
		{
			std::fprintf(stderr, "column is full %i", colNumber);
			return false;
		}

		// try to find where 'gravity' would take the piece
		bool validMove = false;
		for (int i = NumberOfRows; i >= 0; --i)
		{
			if (m_board[i][colNumber] == EmptyMarker)
			{
				m_board[i][colNumber] = playerNum;
				validMove = true;
				break;
			}
		}

		return validMove;
	}

private:
	static const uint8_t NumberOfColumns = 7;
	static const uint8_t NumberOfRows = 6;
	static const uint8_t EmptyMarker = 0;
	uint8_t m_board[NumberOfRows][NumberOfColumns];

	//helper to make querying for full columns ez
	bool m_fullColumns[NumberOfColumns];

};



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

	Board board;
	board.Init();
	std::fprintf(stdout, board.GetDisplayString().c_str());
	std::fprintf(stdout, "\nPlay column 3, player 1\n");
	board.TryPlayMove(3, 1);
	std::fprintf(stdout, board.GetDisplayString().c_str());
	std::fprintf(stdout, "\nPlay column 3, player 2\n");
	board.TryPlayMove(3, 2);
	std::fprintf(stdout, board.GetDisplayString().c_str());
	std::fprintf(stdout, "\nPlay column 4, player 1\n");
	board.TryPlayMove(4, 1);
	std::fprintf(stdout, board.GetDisplayString().c_str());
	std::fprintf(stdout, "\nPlay column 2, player 2\n");
	board.TryPlayMove(2, 2);
	std::fprintf(stdout, board.GetDisplayString().c_str());
	std::fprintf(stdout, "\nPlay column 4, player 1\n");
	board.TryPlayMove(4, 1);
	std::fprintf(stdout, board.GetDisplayString().c_str());


	return 0;
}

