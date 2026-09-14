#include "board.h"

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

std::string GetBoardDisplayString(BoardType& board)
{
	std::string displayString;
	for (int row = 0; row < NumberOfRows; ++row)
	{
		for (int col = 0; col < NumberOfColumns; ++col)
		{
			displayString += GetPieceDisplayString(board[row][col]) + " ";
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

bool TryDropPiece(BoardType& board, uint8_t colNumber, uint8_t playerNum)
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
bool CheckWin(BoardType& board, uint8_t player)
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