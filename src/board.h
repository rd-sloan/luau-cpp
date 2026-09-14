#pragma once
#include <vector>
#include <string>

/** Functions and definitions for a connect 4 board **/

using BoardType = std::vector<std::vector<uint8_t>>;
inline BoardType g_board;

inline constexpr uint8_t NumberOfColumns = 7;
inline constexpr uint8_t NumberOfRows = 6;
inline constexpr uint8_t EmptyMarker = 0;
inline constexpr const char* EmptyDisplay = "-";
inline constexpr const char* Player1Display = "X";
inline constexpr char* Player2Display = "O";

void InitBoard(BoardType& board);
std::string GetPieceDisplayString(uint8_t piece);
std::string GetBoardDisplayString(BoardType& board);
bool IsColumnFull(BoardType& board, uint8_t colNumber);
bool TryDropPiece(BoardType& board, uint8_t colNumber, uint8_t playerNum);
bool CheckWin(BoardType& board, uint8_t player);
bool CheckTie(BoardType& board);