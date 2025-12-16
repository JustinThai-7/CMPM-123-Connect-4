#include "Connect4.h"

Connect4::Connect4() : Game() { _grid = new Grid(BOARD_WIDTH, BOARD_HEIGHT); }

Connect4::~Connect4() { delete _grid; }

//
// Create a yellow or red piece based on player number
//
Bit *Connect4::PieceForPlayer(int playerNumber) {
  Bit *bit = new Bit();
  // Yellow for player 0, Red for player 1
  bit->LoadTextureFromFile(playerNumber == 0 ? "yellow.png" : "red.png");
  bit->setOwner(getPlayerAt(playerNumber));
  bit->setGameTag(playerNumber + 1); // 1 for yellow, 2 for red
  return bit;
}

void Connect4::setUpBoard() {
  setNumberOfPlayers(2);
  _gameOptions.rowX = BOARD_WIDTH;
  _gameOptions.rowY = BOARD_HEIGHT;
  _grid->initializeSquares(80, "square.png");

  // AIPlayer is set by Application.cpp before calling setUpBoard
  // -1 = no AI (2 player), 0 = AI is yellow, 1 = AI is red
  if (gameHasAI() && _gameOptions.AIPlayer >= 0) {
    setAIPlayer(_gameOptions.AIPlayer);
  }
  startGame();
}

//
// Find lowest empty row in a column (-1 if full)
//
int Connect4::getLowestEmptyRow(int column) {
  for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
    ChessSquare *square = _grid->getSquare(column, row);
    if (square && !square->bit()) {
      return row;
    }
  }
  return -1; // Column is full
}

//
// Handle click on any square in a column - drop piece with animation
//
bool Connect4::actionForEmptyHolder(BitHolder &holder) {
  ChessSquare *clickedSquare = static_cast<ChessSquare *>(&holder);
  int column = clickedSquare->getColumn();
  int targetRow = getLowestEmptyRow(column);

  if (targetRow < 0)
    return false; // Column full

  ChessSquare *targetSquare = _grid->getSquare(column, targetRow);
  if (!targetSquare)
    return false;

  Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber());

  // Start position at top of column for animation (extra credit)
  ImVec2 startPos = _grid->getSquare(column, 0)->getPosition();
  startPos.y -= 80; // Start above the board
  bit->setPosition(startPos);
  targetSquare->setBit(bit);

  // Animate falling to final position
  bit->moveTo(targetSquare->getPosition());

  endTurn();
  return true;
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src) {
  return false; // No piece movement in Connect 4
}

bool Connect4::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
  return false; // No piece movement in Connect 4
}

//
// Count consecutive pieces in a direction from a starting position
//
int Connect4::countDirection(int col, int row, int dCol, int dRow,
                             Player *player) {
  int count = 0;
  int c = col + dCol;
  int r = row + dRow;

  while (c >= 0 && c < BOARD_WIDTH && r >= 0 && r < BOARD_HEIGHT) {
    ChessSquare *sq = _grid->getSquare(c, r);
    if (sq && sq->bit() && sq->bit()->getOwner() == player) {
      count++;
      c += dCol;
      r += dRow;
    } else {
      break;
    }
  }
  return count;
}

//
// Check for a win at a specific position
//
bool Connect4::checkWinAt(int col, int row) {
  ChessSquare *sq = _grid->getSquare(col, row);
  if (!sq || !sq->bit())
    return false;

  Player *player = sq->bit()->getOwner();

  // Check all 4 directions (horizontal, vertical, both diagonals)
  // Each direction is checked by counting in both opposing directions
  int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

  for (auto &dir : directions) {
    int count = 1; // Current piece
    count += countDirection(col, row, dir[0], dir[1], player);
    count += countDirection(col, row, -dir[0], -dir[1], player);
    if (count >= 4)
      return true;
  }
  return false;
}

Player *Connect4::checkForWinner() {
  for (int col = 0; col < BOARD_WIDTH; col++) {
    for (int row = 0; row < BOARD_HEIGHT; row++) {
      if (checkWinAt(col, row)) {
        return _grid->getSquare(col, row)->bit()->getOwner();
      }
    }
  }
  return nullptr;
}

bool Connect4::checkForDraw() {
  // Check if all columns are full
  for (int col = 0; col < BOARD_WIDTH; col++) {
    if (getLowestEmptyRow(col) >= 0) {
      return false;
    }
  }
  return true;
}

//
// Clean up all pieces from the board
//
void Connect4::stopGame() {
  _grid->forEachSquare(
      [](ChessSquare *square, int x, int y) { square->destroyBit(); });
}

//
// State string: 42 characters (7 cols x 6 rows), '0'=empty, '1'=yellow, '2'=red
//
std::string Connect4::initialStateString() {
  return std::string(BOARD_WIDTH * BOARD_HEIGHT, '0');
}

std::string Connect4::stateString() {
  std::string s(BOARD_WIDTH * BOARD_HEIGHT, '0');
  _grid->forEachSquare([&](ChessSquare *square, int x, int y) {
    if (square->bit()) {
      s[y * BOARD_WIDTH + x] = '0' + square->bit()->gameTag();
    }
  });
  return s;
}

void Connect4::setStateString(const std::string &s) {
  if (s.length() != BOARD_WIDTH * BOARD_HEIGHT)
    return;

  _grid->forEachSquare([&](ChessSquare *square, int x, int y) {
    square->destroyBit();
    int index = y * BOARD_WIDTH + x;
    int pieceType = s[index] - '0';
    if (pieceType > 0) {
      Bit *bit = PieceForPlayer(pieceType - 1);
      bit->setPosition(square->getPosition());
      square->setBit(bit);
    }
  });
}

//
// AI helper: get lowest empty row from state string
//
int Connect4::getLowestEmptyRowFromState(int column, const std::string &state) {
  for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
    if (state[row * BOARD_WIDTH + column] == '0') {
      return row;
    }
  }
  return -1;
}

bool Connect4::isColumnFull(int column, const std::string &state) {
  return state[column] != '0';
}

//
// Check if any pieces are currently animating
//
bool Connect4::isAnimating() {
  bool animating = false;
  _grid->forEachSquare([&](ChessSquare *square, int x, int y) {
    if (square->bit() && square->bit()->getMoving()) {
      animating = true;
    }
  });
  return animating;
}

//
// Evaluate a window of 4 cells for AI scoring
// Returns score based on piece counts:
// - 4 AI pieces = +1000 (win)
// - 3 AI + 1 empty = +50 (strong threat)
// - 2 AI + 2 empty = +10 (potential)
// - 4 human pieces = -1000 (loss)
// - 3 human + 1 empty = -100 (block urgently - weighted higher to prioritize
// defense)
// - 2 human + 2 empty = -10 (potential threat)
//
int Connect4::evaluateWindow(const std::string &state, int start, int step,
                             char aiChar, char humanChar) {
  int aiCount = 0;
  int humanCount = 0;
  int emptyCount = 0;

  for (int i = 0; i < 4; i++) {
    int idx = start + i * step;
    if (idx < 0 || idx >= (int)state.length())
      return 0;

    char c = state[idx];
    if (c == aiChar)
      aiCount++;
    else if (c == humanChar)
      humanCount++;
    else
      emptyCount++;
  }

  // Mixed windows (both players have pieces) are worthless
  if (aiCount > 0 && humanCount > 0)
    return 0;

  // AI pieces
  if (aiCount == 4)
    return 1000;
  if (aiCount == 3 && emptyCount == 1)
    return 50;
  if (aiCount == 2 && emptyCount == 2)
    return 10;

  // Human pieces - block these!
  if (humanCount == 4)
    return -1000;
  if (humanCount == 3 && emptyCount == 1)
    return -100; // Higher weight for blocking
  if (humanCount == 2 && emptyCount == 2)
    return -10;

  return 0;
}

//
// Evaluate board position for AI
// Returns positive for AI advantage, negative for human advantage
// Evaluates ALL directions including diagonals properly
//
int Connect4::evaluatePosition(const std::string &state) {
  int score = 0;
  // Determine AI and human characters based on which player is AI
  char aiChar = (_gameOptions.AIPlayer == 0) ? '1' : '2';
  char humanChar = (_gameOptions.AIPlayer == 0) ? '2' : '1';

  // Center column bonus - pieces in center are strategically valuable
  for (int row = 0; row < BOARD_HEIGHT; row++) {
    if (state[row * BOARD_WIDTH + 3] == aiChar)
      score += 6;
    else if (state[row * BOARD_WIDTH + 3] == humanChar)
      score -= 6;
  }

  // Horizontal windows
  for (int row = 0; row < BOARD_HEIGHT; row++) {
    for (int col = 0; col <= BOARD_WIDTH - 4; col++) {
      int start = row * BOARD_WIDTH + col;
      score += evaluateWindow(state, start, 1, aiChar, humanChar);
    }
  }

  // Vertical windows
  for (int col = 0; col < BOARD_WIDTH; col++) {
    for (int row = 0; row <= BOARD_HEIGHT - 4; row++) {
      int start = row * BOARD_WIDTH + col;
      score += evaluateWindow(state, start, BOARD_WIDTH, aiChar, humanChar);
    }
  }

  // Diagonal (down-right) windows
  for (int row = 0; row <= BOARD_HEIGHT - 4; row++) {
    for (int col = 0; col <= BOARD_WIDTH - 4; col++) {
      int start = row * BOARD_WIDTH + col;
      score += evaluateWindow(state, start, BOARD_WIDTH + 1, aiChar, humanChar);
    }
  }

  // Diagonal (up-right) windows
  for (int row = 3; row < BOARD_HEIGHT; row++) {
    for (int col = 0; col <= BOARD_WIDTH - 4; col++) {
      int start = row * BOARD_WIDTH + col;
      score +=
          evaluateWindow(state, start, -BOARD_WIDTH + 1, aiChar, humanChar);
    }
  }

  return score;
}

//
// Negamax algorithm with alpha-beta pruning for AI
//
int Connect4::negamax(std::string &state, int depth, int alpha, int beta,
                      int playerColor) {
  int score = evaluatePosition(state);

  // Terminal conditions - someone won
  if (score >= 1000 || score <= -1000) {
    return playerColor * score;
  }

  // Check for draw (no moves available)
  bool hasMove = false;
  for (int col = 0; col < BOARD_WIDTH; col++) {
    if (!isColumnFull(col, state)) {
      hasMove = true;
      break;
    }
  }
  if (!hasMove)
    return 0;

  // Depth limit reached
  if (depth >= 6) {
    return playerColor * score;
  }

  int bestVal = -10000;

  // Try each column, prioritizing center columns
  int order[] = {3, 2, 4, 1, 5, 0, 6};
  for (int i = 0; i < BOARD_WIDTH; i++) {
    int col = order[i];
    int row = getLowestEmptyRowFromState(col, state);
    if (row >= 0) {
      // Make move
      state[row * BOARD_WIDTH + col] = (playerColor == 1) ? '2' : '1';
      int val = -negamax(state, depth + 1, -beta, -alpha, -playerColor);
      // Undo move
      state[row * BOARD_WIDTH + col] = '0';

      bestVal = std::max(bestVal, val);
      alpha = std::max(alpha, val);
      if (alpha >= beta)
        break; // Alpha-beta pruning
    }
  }

  return bestVal;
}

//
// AI move selection using negamax
//
void Connect4::updateAI() {
  // Wait for any animations to complete before AI makes a move
  if (isAnimating()) {
    return;
  }

  std::string state = stateString();
  int bestCol = -1;
  int bestVal = -10000;

  // Prioritize center columns for initial evaluation
  int order[] = {3, 2, 4, 1, 5, 0, 6};
  for (int i = 0; i < BOARD_WIDTH; i++) {
    int col = order[i];
    int row = getLowestEmptyRowFromState(col, state);
    if (row >= 0) {
      // AI character depends on which player AI is
      char aiChar = (_gameOptions.AIPlayer == 0) ? '1' : '2';
      state[row * BOARD_WIDTH + col] = aiChar;
      int val = -negamax(state, 0, -10000, 10000, HUMAN_PLAYER);
      state[row * BOARD_WIDTH + col] = '0';

      if (val > bestVal) {
        bestVal = val;
        bestCol = col;
      }
    }
  }

  if (bestCol >= 0) {
    // Get any square in the chosen column to trigger drop
    ChessSquare *square = _grid->getSquare(bestCol, 0);
    if (square) {
      actionForEmptyHolder(*square);
    }
  }
}
