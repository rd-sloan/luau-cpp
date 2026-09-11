#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

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


class GameState
{
public:
	/** static Functions for luau bindings **/
	static int l_getBoard(lua_State* luaState)
	{
		// Retrieve the GameState * stashed as an upvalue
		GameState* self = static_cast<GameState*>(lua_touserdata(luaState, lua_upvalueindex(1)));
		if (self == nullptr)
		{
			std::fprintf(stderr, "l_getBoard could not find gamestate ptr");
			return 1;
		}

		lua_newtable(luaState); // outer table (rows)
		for (int row = 0; row < NumberOfRows; ++row)
		{
			lua_newtable(luaState); // inner table: (column)
			for (int col = 0; col < NumberOfColumns; ++col)
			{
				// pushes integer to the stack
				lua_pushinteger(luaState, self->m_board[row][col]);
				// actually inserts into table
				// -2 is the index the inner table is at in the stack
				// (-1 is the top, and the integer we just pushed)
				// col + 1 is the index (lua starts indices at 1 instead of 0).
				lua_rawseti(luaState, -2, col + 1);
			}
			lua_rawseti(luaState, -2, row + 1);
		}
		// This return tells luau "Take 1 value off the top of the stack - thats your result"
		// The top value is the outer table, which contains all these values.
		return 1;
	}

	static int l_getEmptyMarker(lua_State* luaState)
	{
		lua_pushinteger(luaState, EmptyMarker);
		return 1;
	}

	// Luau can't read from stdin, so this is a workaround.
	// The human script just calls this function.
	// This could be handled all in C++ by assuming player 1 is always human and getting input, 
	// but by doing it this way we have the option of easily making both players AI scripts if we choose.
	static int l_getHumanMove(lua_State* luaState)
	{
		// TODO: validate this is a valid column number lol
		int col;
		std::cout << "Enter column (1-7): ";
		std::cin >> col;

		lua_pushinteger(luaState, col);
		return 1;
	}


	/** Regular functions **/

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
	}

	std::string GetPieceDisplayString(uint8_t piece)
	{
		if (piece == 0)
		{
			return EmptyDisplay;
		}
		else if (piece == 1)
		{
			return Player1Display;
		}
		else if (piece == 2)
		{
			return Player2Display;
		}
		else
		{
			return "?";
		}
	}

	std::string GetDisplayString()
	{
		std::string displayString;
		for (int row = 0; row < NumberOfRows; ++row)
		{
			for (int col = 0; col < NumberOfColumns; ++col)
			{
				displayString += GetPieceDisplayString(m_board[row][col]) + " ";
			}
			displayString += "\n";
		}
		displayString += "--------------\n";
		displayString += "1 2 3 4 5 6 7\n";
		displayString += "\n";

		return displayString;
	}

	bool IsColumnFull(uint8_t colNumber)
	{
		if (colNumber < NumberOfColumns)
		{
			return m_board[0][colNumber] != EmptyMarker;
		}

		return true; // if column doesn't exist its technically full
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
		for (int i = NumberOfRows - 1; i >= 0; --i)
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

	// TODO could optimize this by just checking around the most recent play (this does a full board sweep)
	bool CheckWin(uint8_t player)
	{
		// Horizontal check
		for (int row = 0; row < NumberOfRows; row++) {
			for (int col = 0; col <= NumberOfColumns - 4; col++) {
				if (m_board[row][col] == player &&
					m_board[row][col + 1] == player &&
					m_board[row][col + 2] == player &&
					m_board[row][col + 3] == player) {
					return true;
				}
			}
		}

		// Vertical check
		for (int col = 0; col < NumberOfColumns; col++) {
			for (int row = 0; row <= NumberOfRows - 4; row++) {
				if (m_board[row][col] == player &&
					m_board[row + 1][col] == player &&
					m_board[row + 2][col] == player &&
					m_board[row + 3][col] == player) {
					return true;
				}
			}
		}

		// Diagonal check (down-right: \ )
		for (int row = 0; row <= NumberOfRows - 4; row++) {
			for (int col = 0; col <= NumberOfColumns - 4; col++) {
				if (m_board[row][col] == player &&
					m_board[row + 1][col + 1] == player &&
					m_board[row + 2][col + 2] == player &&
					m_board[row + 3][col + 3] == player) {
					return true;
				}
			}
		}

		// Diagonal check (down-left: / )
		for (int row = 0; row <= NumberOfRows - 4; row++) {
			for (int col = 3; col < NumberOfColumns; col++) {
				if (m_board[row][col] == player &&
					m_board[row + 1][col - 1] == player &&
					m_board[row + 2][col - 2] == player &&
					m_board[row + 3][col - 3] == player) {
					return true;
				}
			}
		}

		return false;
	}

	bool CheckTie()
	{
		// Check the top row, if all aren't empty, then its a tie 
		// (assuming we did a win check before this)
		for (int col = 0; col < NumberOfColumns; ++col)
		{
			if (m_board[0][col] == EmptyMarker)
			{
				return false;
			}
		}
		return true;
	}

private:
	static constexpr uint8_t NumberOfColumns = 7;
	static constexpr uint8_t NumberOfRows = 6;
	static constexpr uint8_t EmptyMarker = 0;
	static constexpr const char* EmptyDisplay =  "-";
	static constexpr const char* Player1Display = "X";
	static constexpr char* Player2Display = "O";
	uint8_t m_board[NumberOfRows][NumberOfColumns];
};


lua_State* SetupPlayerState(const std::string& scriptName, GameState* gameState)
{
	// Create a Luau VM state
	lua_State* luaState = luaL_newstate();
	luaL_openlibs(luaState); // opens standard library (print, math, string, etc)

	// luau bindings
	// pushes game state pointer as upvalue #1
	lua_pushlightuserdata(luaState, gameState);
	// captures pointer and exposes function binding, the 1 here indicates to get the gamestate from upvalue 1
	lua_pushcclosure(luaState, GameState::l_getBoard, "getBoard", 1);
	lua_setglobal(luaState, "getBoard");
	// If didn't do the class, alternative above would be a global static function
	// Would not need to push user data, and would use pushcfunction instead of pushcclosure
	// another alternative would be to make GameState a singleton and make the Getboard function get the singleton.

	// human move doesn't need the board, so we don't need to push user data or closure, can just push function.
	lua_pushcfunction(luaState, GameState::l_getHumanMove, "getHumanMove");
	lua_setglobal(luaState, "getHumanMove");

	lua_pushcfunction(luaState, GameState::l_getEmptyMarker, "getEmptyMarker");
	lua_setglobal(luaState, "getEmptyMarker");
	


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
uint8_t GetMoveFromScript(lua_State* luaState, GameState* game, int playerNum)
{
	lua_getglobal(luaState, "getMove");					// push function
	lua_pushinteger(luaState, playerNum);				// push arg (player number)
	lua_pcall(luaState, /*nargs*/1, /*nresults*/1, 0);	// run code
	uint8_t col = (uint8_t)lua_tointeger(luaState, -1);			
	lua_pop(luaState, 1);								// pop the return value
	return col;
}

bool fileExists(const std::string& path) 
{
	std::ifstream f(path);
	return f.good();
}


int main(int argc, char** argv)
{
	// Defaults 
	std::string player1Script = std::string(SCRIPTS_DIR) + "/winThenBlock.luau";
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

	// init game state
	GameState gameState;
	gameState.Init();

	// init players
	lua_State* player1L = SetupPlayerState(player1Script, &gameState);
	lua_State* player2L = SetupPlayerState(player2Script, &gameState);

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
		std::fprintf(stdout, "Player %i (%s) enter your move:\n", playerNum, gameState.GetPieceDisplayString(playerNum).c_str());
		uint8_t col = GetMoveFromScript(current, &gameState, playerNum);
		std::fprintf(stdout, "Player %i (%s) plays column %i\n", playerNum, gameState.GetPieceDisplayString(playerNum).c_str(), col);
		gameState.TryPlayMove(col - 1, playerNum); // -1 because luau uses 1-7 instead of 0-6
		std::fprintf(stdout, gameState.GetDisplayString().c_str());

		// check for win
		if (gameState.CheckWin(playerNum))
		{
			std::fprintf(stdout, "Player %i (%s) wins!!!!!!", playerNum, playerNum == 1 ? player1Script.c_str() : player2Script.c_str());
			endGame = true;		// since we break this isn't really necessary, but just in case.
			break;
		}

		if (gameState.CheckTie())
		{
			std::fprintf(stdout, "It's a tie, you both lose!");
			endGame = true;		// since we break this isn't really necessary, but just in case.
			break;
		}

		player1Turn = !player1Turn;
	}

	
	lua_close(player1L);
	lua_close(player2L);

	return 0;
}

