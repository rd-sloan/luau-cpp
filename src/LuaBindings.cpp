#include "LuaBindings.h"
#include "board.h"

#include <iostream>

int l_getBoard(lua_State* luaState)
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

int l_getEmptyMarker(lua_State* luaState)
{
	lua_pushinteger(luaState, EmptyMarker);
	return 1;
}

// Luau can't read from stdin, so this is a workaround.
// The human script just calls this function.
// This could be handled all in C++ by assuming player 1 is always human and getting input, 
// but by doing it this way we have the option of easily making both players AI scripts if we choose.
int l_getHumanMove(lua_State* luaState)
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
int l_wouldWin(lua_State* luaState) 
{
	// arg 1: board (table of tables)
	// arg 2: column (1-indexed, from Luau)
	// arg 3: player number

	// Copy board passed in from luau
	BoardType tempBoard;
	InitBoard(tempBoard);
	for (int row = 1; row <= NumberOfRows; row++)
	{
		lua_rawgeti(luaState, 1, row); // push board[row]
		for (int col = 1; col <= NumberOfColumns; col++)
		{
			lua_rawgeti(luaState, -1, col); // push board[row][col]
			tempBoard[row - 1][col - 1] = static_cast<uint8_t>(lua_tointeger(luaState, -1));
			lua_pop(luaState, 1); // pop the value
		}
		lua_pop(luaState, 1); // pop the row table
	}

	int col = static_cast<int>(lua_tointeger(luaState, 2)) - 1; // convert to 0-indexed
	uint8_t player = static_cast<uint8_t>(lua_tointeger(luaState, 3));

	// Simulate dropping into tempBoard (gravity: find lowest open row in this column)
	bool dropped = TryDropPiece(tempBoard, col, player);

	if (!dropped)
	{
		// Column was full - shouldn't happen if caller only passes valid columns
		lua_pushboolean(luaState, false);
		return 1;
	}

	bool won = CheckWin(tempBoard, player);
	lua_pushboolean(luaState, won);
	return 1;
}