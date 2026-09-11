#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include "lua.h"
#include "lualib.h"
#include "luacode.h"


using BoardType = std::vector<std::vector<uint8_t>>;
BoardType g_board;
static constexpr uint8_t NumberOfColumns = 7;
static constexpr uint8_t NumberOfRows = 6;
static constexpr uint8_t EmptyMarker = 0;
static constexpr const char* EmptyDisplay = "-";
static constexpr const char* Player1Display = "X";
static constexpr char* Player2Display = "O";


/** Regular Board functions **/

void InitBoard(BoardType& board)
{
	// size board and init to all empty markers
	board.resize(NumberOfRows, std::vector<uint8_t>(NumberOfColumns, EmptyMarker));
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

std::string GetBoardDisplayString()
{
	std::string displayString;
	for (int row = 0; row < NumberOfRows; ++row)
	{
		for (int col = 0; col < NumberOfColumns; ++col)
		{
			displayString += GetPieceDisplayString(g_board[row][col]) + " ";
		}
		displayString += "\n";
	}
	displayString += "--------------\n";
	displayString += "1 2 3 4 5 6 7\n";
	displayString += "\n";

	return displayString;
}

bool IsColumnFull(BoardType& board, uint8_t colNumber)
{
	if (colNumber < NumberOfColumns)
	{
		return board[0][colNumber] != EmptyMarker;
	}

	return true; // if column doesn't exist its technically full
}

static bool TryDropPiece(BoardType& board, uint8_t colNumber, uint8_t playerNum)
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

	if (IsColumnFull(board, colNumber))
	{
		std::fprintf(stderr, "column is full %i", colNumber);
		return false;
	}

	// try to find where 'gravity' would take the piece
	bool validMove = false;
	for (int i = NumberOfRows - 1; i >= 0; --i)
	{
		if (board[i][colNumber] == EmptyMarker)
		{
			board[i][colNumber] = playerNum;
			validMove = true;
			break;
		}
	}

	return validMove;
}

// TODO could optimize this by just checking around the most recent play (this does a full board sweep)
static bool CheckWin(BoardType& board, uint8_t player)
{
	// Horizontal check
	for (int row = 0; row < NumberOfRows; row++) {
		for (int col = 0; col <= NumberOfColumns - 4; col++) {
			if (board[row][col] == player &&
				board[row][col + 1] == player &&
				board[row][col + 2] == player &&
				board[row][col + 3] == player) {
				return true;
			}
		}
	}

	// Vertical check
	for (int col = 0; col < NumberOfColumns; col++) {
		for (int row = 0; row <= NumberOfRows - 4; row++) {
			if (board[row][col] == player &&
				board[row + 1][col] == player &&
				board[row + 2][col] == player &&
				board[row + 3][col] == player) {
				return true;
			}
		}
	}

	// Diagonal check (down-right: \ )
	for (int row = 0; row <= NumberOfRows - 4; row++) {
		for (int col = 0; col <= NumberOfColumns - 4; col++) {
			if (board[row][col] == player &&
				board[row + 1][col + 1] == player &&
				board[row + 2][col + 2] == player &&
				board[row + 3][col + 3] == player) {
				return true;
			}
		}
	}

	// Diagonal check (down-left: / )
	for (int row = 0; row <= NumberOfRows - 4; row++) {
		for (int col = 3; col < NumberOfColumns; col++) {
			if (board[row][col] == player &&
				board[row + 1][col - 1] == player &&
				board[row + 2][col - 2] == player &&
				board[row + 3][col - 3] == player) {
				return true;
			}
		}
	}

	return false;
}

bool CheckTie(BoardType& board)
{
	// Check the top row, if all aren't empty, then its a tie 
	// (assuming we did a win check before this)
	for (int col = 0; col < NumberOfColumns; ++col)
	{
		if (board[0][col] == EmptyMarker)
		{
			return false;
		}
	}
	return true;
}



/** Luau Board Function Bindings **/
static int l_getBoard(lua_State* luaState)
{
	lua_newtable(luaState); // outer table (rows)
	for (int row = 0; row < NumberOfRows; ++row)
	{
		lua_newtable(luaState); // inner table: (column)
		for (int col = 0; col < NumberOfColumns; ++col)
		{
			// pushes integer to the stack
			lua_pushinteger(luaState, g_board[row][col]);
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

/**
* Checks if a given move would potentially win
* There is some extra board copying here
* (1 when lua calls GetBoard, 1 when lua is calling this and we copy the board back)
* Could just use the board ref we have here, but I like having an example of getting bigger data back from lua
* And it could give us some more freedom for more advanced AI trying to check future moves
*/
static int l_wouldWin(lua_State* L) {
	// arg 1: board (table of tables)
	// arg 2: column (1-indexed, from Luau)
	// arg 3: player number

	// Copy board passed in from luau
	BoardType tempBoard;
	InitBoard(tempBoard);
	for (int row = 1; row <= NumberOfRows; row++) 
	{
		lua_rawgeti(L, 1, row); // push board[row]
		for (int col = 1; col <= NumberOfColumns; col++) 
		{
			lua_rawgeti(L, -1, col); // push board[row][col]
			tempBoard[row - 1][col - 1] = static_cast<uint8_t>(lua_tointeger(L, -1));
			lua_pop(L, 1); // pop the value
		}
		lua_pop(L, 1); // pop the row table
	}

	int col = static_cast<int>(lua_tointeger(L, 2)) - 1; // convert to 0-indexed
	uint8_t player = static_cast<uint8_t>(lua_tointeger(L, 3));

	// Simulate dropping into tempBoard (gravity: find lowest open row in this column)
	bool dropped = TryDropPiece(tempBoard, col, player);

	if (!dropped) 
	{
		// Column was full - shouldn't happen if caller only passes valid columns
		lua_pushboolean(L, false);
		return 1;
	}

	bool won = CheckWin(tempBoard, player);
	lua_pushboolean(L, won);
	return 1;
}


bool fileExists(const std::string& path)
{
	std::ifstream f(path);
	return f.good();
}

static std::string readFile(const std::string& path)
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
		std::fprintf(stdout, GetBoardDisplayString().c_str());

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

