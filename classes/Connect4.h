#pragma once
#include "Game.h"

//
// Connect 4 - the classic game of connecting four pieces in a row
//

class Connect4 : public Game {
public:
  Connect4();
  ~Connect4();

  // Required virtual methods from Game base class
  void setUpBoard() override;
  Player *checkForWinner() override;
  bool checkForDraw() override;
  std::string initialStateString() override;
  std::string stateString() override;
  void setStateString(const std::string &s) override;
  bool actionForEmptyHolder(BitHolder &holder) override;
  bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
  bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
  void stopGame() override;

  // AI methods
  void updateAI() override;
  bool gameHasAI() override { return _gameOptions.AIPlayer >= 0; }
  Grid *getGrid() override { return _grid; }

private:
  // Constants
  static const int BOARD_WIDTH = 7;
  static const int BOARD_HEIGHT = 6;
  static const int YELLOW_PLAYER = 0; // Human by default
  static const int RED_PLAYER = 1;    // AI by default

  // Helper methods
  Bit *PieceForPlayer(int playerNumber);
  int getLowestEmptyRow(int column);
  bool checkWinAt(int col, int row);
  int countDirection(int col, int row, int dCol, int dRow, Player *player);
  int negamax(std::string &state, int depth, int alpha, int beta,
              int playerColor);
  int evaluatePosition(const std::string &state);
  int evaluateWindow(const std::string &state, int start, int step, char aiChar,
                     char humanChar);
  bool isColumnFull(int column, const std::string &state);
  int getLowestEmptyRowFromState(int column, const std::string &state);
  bool isAnimating();

  Grid *_grid;
};
