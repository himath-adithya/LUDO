#include "gamelogic.h"
#include "types.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== SECTION: function definitions =====

void initialization() {
  // Initialize colors
  for (short i = 0; i < COLOR_TOTAL; i++) {
    colors[i].colorNum = i;
  }
  colors[0].colorName = "Red";
  colors[1].colorName = "Green";
  colors[2].colorName = "Yellow";
  colors[3].colorName = "Blue";

  // Initialize pieces and blocks
  for (short color = 0; color < COLOR_TOTAL; color++) {
    for (short piece = 0; piece < PIECE_TOTAL_PER_COLOR; piece++) {
      Piece *p = &pieces[color][piece];
      p->color = &colors[color];
      p->pieceNum = piece;
      p->locationType = BASE;
      p->location = 0;
      p->startingCell = 13 * ((color + 2) % COLOR_TOTAL) + 2; // red: 28; green: 41; yellow: 2; blue: 15;
      p->approach = 13 * ((color + 2) % COLOR_TOTAL);         // red: 26; green: 39; yellow: 0; blue: 13;
      p->capturedPieceCount = 0;
      p->approachPassCount = 0;
      p->blockNum = NULL;
      p->name[0] = colors[color].colorName[0];
      p->name[1] = '1' + piece;
      p->movingDirection = CLOCKWISE;
      p->mysteryEffect = 0;
      p->mysteryEffectDuration = 0;
    }

    for (short block = 0; block < MAX_BLOCKS_PER_COLOR; block++) {
      Block *b = &blocks[color][block];
      b->color = &colors[color];
      for (short piece = 0; piece < MAX_PIECES_PER_BLOCK; piece++) {
        b->pieces[piece] = NULL;
      }
      b->blockNum = block;
      b->pieceCount = 0;
      b->movingDirection = CLOCKWISE;
      b->mysteryEffect = 0;
      b->mysteryEffectDuration = 0;
    }
  }

  // Initialize bases
  for (short color = 0; color < COLOR_TOTAL; color++) {
    Base *b = &bases[color];
    for (short piece = 0; piece < PIECE_TOTAL_PER_COLOR; piece++) {
      b->pieces[piece] = &pieces[color][piece];
    }
    b->color = &colors[color];
    b->pieceCount = 4;
  }

  // Initialize standardArea
  for (short cell = 0; cell < MAX_STANDARD_CELLS; cell++) {
    StandardCell *s = &standardArea[cell];
    s->piece = NULL;
    s->block = NULL;
    s->cellIndex = cell;
    s->pieceCount = 0;
  }

  // Initialize homeStraights
  for (short color = 0; color < COLOR_TOTAL; color++) {
    for (short cell = 0; cell < MAX_HOME_STRAIGHT_CELLS; cell++) {
      homeStraights[color][cell].pieceCount = 0;
      homeStraights[color][cell].color = &colors[color];
      homeStraights[color][cell].cellIndex = cell;
      homeStraights[color][cell].piece = NULL;
    }
  }

  // Initialize homes
  for (short color = 0; color < COLOR_TOTAL; color++) {
    homes[color].color = &colors[color];
    homes[color].pieceCount = 0;
    for (short piece = 0; piece < PIECE_TOTAL_PER_COLOR; piece++) {
      homes[color].pieces[piece] = NULL;
    }
  }

  // Initialize mystery cell
  mysteryCell.cell = NULL;

  // Initialize players
  for (short player = 0; player < PLAYER_TOTAL; player++) {
    sprintf(players[player].playerName, "%s player", colors[player].colorName);
    players[player].isWinner = FALSE;
    players[player].color = &colors[player];
    for (short piece = 0; piece < PIECE_TOTAL_PER_PLAYER; piece++) {
      players[player].pieces[piece] = &pieces[player][piece];
    }
  }

  return;
}

void briefEffect(void *element, short elementType) {
  if (elementType == 0 && ((Piece *)element)->blockNum == NULL) {
    Piece *p = (Piece *)element;

    printf("%s's %s is movement-restricted and has rolled three consecutively. Teleporting %s to base.\n",
           p->color->colorName, p->name, p->name);

    if (p->locationType == BASE) {
      printf("Piece is already in base.");
    } else if (p->locationType == STANDARD_AREA) {
      standardArea[p->location].piece = NULL;
      standardArea[p->location].pieceCount = 0;
      rebasePiece(p);
    } else if (p->locationType == HOME_STRAIGHT) {
      homeStraights[p->color->colorNum][p->location].piece = NULL;
      homeStraights[p->color->colorNum][p->location].pieceCount = 0;
      rebasePiece(p);
    } else if (p->locationType == HOME) {
      printf("Piece is already at home. Can't take effect.");
    }
  } else if (elementType == 1 && ((Block *)element)->pieceCount > 0) {
    Block *b = (Block *)element;
    printf("%s's block of %d is movement-restricted and has rolled three "
           "consecutively. Teleporting block to base.\n",
           b->color->colorName, b->pieceCount);
    if (b->pieces[0]->locationType == STANDARD_AREA) {
      standardArea[b->pieces[0]->location].block = NULL;
      standardArea[b->pieces[0]->location].pieceCount = 0;
      for (short piece = 0; piece < b->pieceCount; piece++) {
        rebasePiece(b->pieces[piece]);
      }
      resetBlock(b);
    } else if (b->pieces[0]->locationType == HOME_STRAIGHT) {
      homeStraights[b->color->colorNum][b->pieces[0]->location].block = NULL;
      homeStraights[b->color->colorNum][b->pieces[0]->location].pieceCount = 0;
      for (short piece = 0; piece < b->pieceCount; piece++) {
        rebasePiece(b->pieces[piece]);
      }
      resetBlock(b);
    }
  }
}

void checkBriefEffect(short diceRolledValue) {

  for (short piece = 0; piece < PIECE_TOTAL_PER_COLOR; piece++) {
    Piece *p = &pieces[currentTurnColor][piece];
    if (p->mysteryEffect == 1) {
      if (diceRolledValue == 3) {
        p->whileInBriefConsecutiveThrees++;
      } else {
        p->whileInBriefConsecutiveThrees = 0;
      }
    }

    if (p->mysteryEffect == 1 && p->whileInBriefConsecutiveThrees > 1) {
      briefEffect(p, 0);
    }
  }

  for (short block = 0; block < MAX_BLOCKS_PER_COLOR; block++) {
    Block *b = &blocks[currentTurnColor][block];
    if (b->mysteryEffect == 1) {
      if (diceRolledValue == 3) {
        b->whileInBriefConsecutiveThrees++;
      } else {
        b->whileInBriefConsecutiveThrees = 0;
      }
    }

    if (b->pieceCount != 0 && b->mysteryEffect == 1 && b->whileInBriefConsecutiveThrees > 1) {
      briefEffect(b, 1);
    }
  }
}

short diceRoll() {
  long randNum = rand();
  short diceRolledValue = randNum % DIE_ROLL_MAX + 1;
  printf("%s rolls %d\n", colors[currentTurnColor].colorName, diceRolledValue);

  checkBriefEffect(diceRolledValue);

  return diceRolledValue;
}

void cycleToNextPlayer() {
  currentTurnColor = (currentTurnColor + 1) % COLOR_TOTAL;
  if (players[currentTurnColor].isWinner) {
    cycleToNextPlayer();
  }
}

void firstToRollPlayerSelection() {
  short rolledValues[COLOR_TOTAL] = {0};
  for (short player = 0; player < PLAYER_TOTAL; player++) {
    rolledValues[player] = diceRoll();
    cycleToNextPlayer();
  }

  short maxRolledValue = rolledValues[0];
  short maxRolledValueIndex = 0;

  for (short player = 1; player < PLAYER_TOTAL; player++) {
    if (rolledValues[player] > maxRolledValue) {
      maxRolledValue = rolledValues[player];
      maxRolledValueIndex = player;
    }
  }

  currentTurnColor = maxRolledValueIndex;
  printf("%s player has the highest roll and will begin the game.\n", colors[currentTurnColor].colorName);
  printf("The order of a single round is %s, %s, %s, and %s.\n\n", colors[currentTurnColor].colorName,
         colors[(currentTurnColor + 1) % COLOR_TOTAL].colorName,
         colors[(currentTurnColor + 2) % COLOR_TOTAL].colorName,
         colors[(currentTurnColor + 3) % COLOR_TOTAL].colorName);
  return;
}

short coinToss() {
  long ranNum = rand();
  short coinTossedValue = ranNum % COIN_TOSS_MAX;
  return coinTossedValue;
}

void resetMoves() {
  moveIndex = 0;
  for (short move = 0; move < MOVES_TOTAL; move++) {
    moves[move].base = NULL;
    moves[move].home = NULL;
    moves[move].homeStraightCell = NULL;
    moves[move].moveType = 0;
    moves[move].piece = NULL;
    moves[move].block = NULL;
    moves[move].playerPiece = NULL;
    moves[move].playerBlock = NULL;
    moves[move].enemyPiece = NULL;
    moves[move].enemyBlock = NULL;
    moves[move].startStandardCell = NULL;
    moves[move].endStandardCell = NULL;
    moves[move].isFromBlock = FALSE;
  }
}

void rebasePiece(Piece *piece) {
  piece->locationType = BASE;
  piece->location = 0;
  piece->capturedPieceCount = 0;
  piece->approachPassCount = 0;
  piece->blockNum = NULL;
  piece->movingDirection = CLOCKWISE;
  piece->mysteryEffect = 0;
  piece->mysteryEffectDuration = 0;
  Base *base = &bases[piece->color->colorNum];
  base->pieces[piece->pieceNum] = piece;
  base->pieceCount++;
}

void resetBlock(Block *block) {
  block->movingDirection = CLOCKWISE;
  block->mysteryEffect = 0;
  block->mysteryEffectDuration = 0;
  block->pieceCount = 0;
  for (short piece = 0; piece < MAX_PIECES_PER_BLOCK; piece++) {
    block->pieces[piece] = NULL;
  }
}

char longestMovingPath(short currentPosition, short approach) {
  short cwDistance = 0;
  short ccwDistance = 0;

  while ((currentPosition + cwDistance + 52) % 52 != approach) {
    cwDistance++;
  }
  while ((currentPosition - ccwDistance + 52) % 52 != approach) {
    ccwDistance++;
  }

  if (cwDistance >= ccwDistance) {
    return CLOCKWISE;
  }
  return COUNTER_CLOCKWISE;
}

Check nextCellsInspection(Piece *piece, Block *block, short moveValue) {

  Check check = {FALSE, FALSE, 0, FALSE, 0, 0};
  short nextLocation;

  if (piece != NULL) {
    check.initialMoveValue = moveValue;
    for (short cell = 1; cell <= moveValue; cell++) {

      if (piece->movingDirection == COUNTER_CLOCKWISE) {
        nextLocation = (piece->location - cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      } else {
        nextLocation = (piece->location + cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      }

      StandardCell *nextCell = &standardArea[nextLocation];

      if (nextCell->block != NULL && nextCell->block->color->colorNum != currentTurnColor) {
        check.doesPieceOrBlockMeetBlock = TRUE;
        if (piece->movingDirection == COUNTER_CLOCKWISE) {
          check.distanceToBlock = -cell;
        } else {
          check.distanceToBlock = cell;
        }
        check.canCaptureBlock = FALSE;
        break;
      }
    }

    for (short cell = 0; cell < moveValue; cell++) {

      if (piece->movingDirection == COUNTER_CLOCKWISE) {
        nextLocation = (piece->location - cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      } else {
        nextLocation = (piece->location + cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      }

      if (piece->approach == nextLocation) {
        check.distanceToApproach = cell;
        check.doesPieceOrBlockPassApproach = TRUE;
        break;
      }
    }
  }

  else if (block != NULL) {
    short moveBlockValue = moveValue / block->pieceCount;
    if (moveBlockValue == 0) {
      return check;
    }
    check.initialMoveValue = moveBlockValue;

    for (short cell = 1; cell <= moveBlockValue; cell++) {

      if (block->movingDirection == COUNTER_CLOCKWISE) {
        nextLocation = (block->pieces[0]->location - cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      } else {
        nextLocation = (block->pieces[0]->location + cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      }

      StandardCell *nextCell = &standardArea[nextLocation];

      if (nextCell->block != NULL && nextCell->block->color->colorNum != currentTurnColor) {
        check.doesPieceOrBlockMeetBlock = TRUE;
        if (block->movingDirection == COUNTER_CLOCKWISE) {
          check.distanceToBlock = -cell;
        } else {
          check.distanceToBlock = cell;
        }
        if (block->pieceCount >= nextCell->block->pieceCount) {
          check.canCaptureBlock = TRUE;
        } else {
          check.canCaptureBlock = FALSE;
        }
        break;
      }
    }

    for (short cell = 0; cell < moveBlockValue; cell++) {
      if (block->movingDirection == COUNTER_CLOCKWISE) {
        nextLocation = (block->pieces[0]->location - cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      } else {
        nextLocation = (block->pieces[0]->location + cell + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
      }

      if (block->pieces[0]->approach == nextLocation) {
        check.distanceToApproach = cell;
        check.doesPieceOrBlockPassApproach = TRUE;
        break;
      }
    }
  }

  if (check.doesPieceOrBlockMeetBlock) {
    if (check.distanceToBlock > check.distanceToApproach && check.doesPieceOrBlockPassApproach) {
      check.doesPieceOrBlockPassApproach = TRUE;
    } else {
      check.doesPieceOrBlockPassApproach = FALSE;
    }
  }

  return check;
}

void createBlock(Piece *originPiece, Piece *targetPiece, void *targetCell, short cellLocationType) {
  Piece *op = originPiece;
  Piece *tp = targetPiece;
  Block *b = NULL;
  for (short block = 0; block < MAX_BLOCKS_PER_COLOR; block++) {
    if (blocks[op->color->colorNum][block].pieceCount == 0) {
      b = &blocks[op->color->colorNum][block];
      break;
    }
  }

  switch (cellLocationType) {
    case STANDARD_AREA: {
      StandardCell *stc = (StandardCell *)targetCell;
      op->blockNum = &b->blockNum;
      op->location = tp->location;
      op->locationType = tp->locationType;
      tp->blockNum = &b->blockNum;
      char movingDirection = longestMovingPath(stc->cellIndex, op->approach);
      b->movingDirection = movingDirection;
      b->pieceCount = 2;
      b->pieces[0] = tp;
      b->pieces[1] = op;
      stc->block = b;
      stc->piece = NULL;
      stc->pieceCount = 2;
      break;
    }
    case HOME_STRAIGHT: {
      HomeStraightCell *htc = (HomeStraightCell *)targetCell;
      op->blockNum = &b->blockNum;
      op->location = tp->location;
      op->locationType = tp->locationType;
      tp->blockNum = &b->blockNum;
      b->movingDirection = CLOCKWISE;
      b->pieceCount = 2;
      b->pieces[0] = tp;
      b->pieces[1] = op;
      htc->block = b;
      htc->piece = NULL;
      htc->pieceCount = 2;
      break;
    }
    default:
      break;
  }
}

void expandBlockByPiece(Piece *originPiece, Block *targetBlock, void *targetCell, short cellLocationType) {
  Piece *op = originPiece;
  Block *tb = targetBlock;

  switch (cellLocationType) {
    case STANDARD_AREA: {
      StandardCell *stc = (StandardCell *)targetCell;
      op->blockNum = &tb->blockNum;
      op->location = tb->pieces[0]->location;
      op->locationType = tb->pieces[0]->locationType;
      char movingDirection = longestMovingPath(stc->cellIndex, tb->pieces[0]->approach);
      tb->movingDirection = movingDirection;
      tb->pieces[tb->pieceCount] = op;
      tb->pieceCount++;
      stc->pieceCount++;
      break;
    }
    case HOME_STRAIGHT: {
      HomeStraightCell *htc = (HomeStraightCell *)targetCell;
      op->blockNum = &tb->blockNum;
      op->location = tb->pieces[0]->location;
      op->locationType = tb->pieces[0]->locationType;
      tb->movingDirection = CLOCKWISE;
      tb->pieces[tb->pieceCount] = op;
      tb->pieceCount++;
      htc->pieceCount++;
      break;
    }
    default:
      break;
  }
}

void expandPieceByBlock(Block *originblock, Piece *targetPiece, void *targetCell, short cellLocationType) {
  Block *ob = originblock;
  Piece *tp = targetPiece;

  switch (cellLocationType) {
    case STANDARD_AREA: {
      StandardCell *stc = (StandardCell *)targetCell;
      char movingDirection = longestMovingPath(stc->cellIndex, tp->approach);
      ob->movingDirection = movingDirection;
      for (short piece = 0; piece < ob->pieceCount; piece++) {
        ob->pieces[piece]->location = stc->cellIndex;
        ob->pieces[piece]->locationType = STANDARD_AREA;
      }
      ob->pieces[ob->pieceCount] = tp;
      ob->pieceCount++;
      tp->blockNum = &ob->blockNum;
      stc->block = ob;
      stc->piece = NULL;
      stc->pieceCount = ob->pieceCount;
      break;
    }
    case HOME_STRAIGHT: {
      HomeStraightCell *htc = (HomeStraightCell *)targetCell;
      ob->movingDirection = CLOCKWISE;
      for (short piece = 0; piece < ob->pieceCount; piece++) {
        ob->pieces[piece]->location = htc->cellIndex;
        ob->pieces[piece]->locationType = HOME_STRAIGHT;
      }
      ob->pieces[ob->pieceCount] = tp;
      ob->pieceCount++;
      tp->blockNum = &ob->blockNum;
      htc->block = ob;
      htc->piece = NULL;
      htc->pieceCount = ob->pieceCount;
    }
    default:
      break;
  }
}

void expandBlockByBlock(Block *originBlock, Block *targetBlock, void *targetCell, short cellLocationType) {
  Block *ob = originBlock;
  Block *tb = targetBlock;

  switch (cellLocationType) {
    case STANDARD_AREA: {
      StandardCell *stc = (StandardCell *)targetCell;
      char movingDirection = longestMovingPath(stc->cellIndex, tb->pieces[0]->approach);
      tb->movingDirection = movingDirection;
      for (short piece = 0; piece < ob->pieceCount; piece++) {
        ob->pieces[piece]->blockNum = &tb->blockNum;
        ob->pieces[piece]->location = tb->pieces[0]->location;
        ob->pieces[piece]->locationType = STANDARD_AREA;
        tb->pieces[tb->pieceCount + piece] = ob->pieces[piece];
      }
      tb->pieceCount += ob->pieceCount;
      resetBlock(ob);
      stc->pieceCount = tb->pieceCount;
      break;
    }
    case HOME_STRAIGHT: {
      HomeStraightCell *htc = (HomeStraightCell *)targetCell;
      tb->movingDirection = CLOCKWISE;
      for (short piece = 0; piece < ob->pieceCount; piece++) {
        ob->pieces[piece]->blockNum = &tb->blockNum;
        ob->pieces[piece]->location = tb->pieces[0]->location;
        ob->pieces[piece]->locationType = HOME_STRAIGHT;
        tb->pieces[tb->pieceCount + piece] = ob->pieces[piece];
      }
      tb->pieceCount += ob->pieceCount;
      resetBlock(ob);
      htc->pieceCount = tb->pieceCount;
      break;
    }
    default:
      break;
  }
}

Move blockBreakMoveCalculate(Piece *piece, short moveValue) {
  Move move;
  move.moveType = 0;
  move.startStandardCell = NULL;
  move.endStandardCell = NULL;
  move.homeStraightCell = NULL;
  move.home = NULL;
  move.piece = piece;
  move.enemyPiece = NULL;
  move.check = nextCellsInspection(piece, NULL, moveValue);

  // no move
  if (moveValue == 0) {
    return move;
  };

  short moveLeft = 0;
  if (piece->locationType == STANDARD_AREA) {
    if (move.check.doesPieceOrBlockMeetBlock == TRUE && isEligibleForHomeStraight(piece, NULL)) {

      // standardArea --> home
      if (moveValue - move.check.distanceToApproach == 6) {
        move.moveType = 12;
        move.startStandardCell = &standardArea[piece->location];
        move.home = &homes[piece->color->colorNum];
      } else if (moveValue - move.check.distanceToApproach > 6) {
        moveLeft = 5;
      } else {
        moveLeft = moveValue - move.check.distanceToApproach;
      }

      // standardArea --> homeStraight / emptyCell
      if (homeStraights[piece->color->colorNum][moveLeft - 1].pieceCount == 0) {
        move.moveType = 9;
        move.startStandardCell = &standardArea[piece->location];
        move.homeStraightCell = &homeStraights[piece->color->colorNum][moveLeft - 1];
      }
    } else {
      short nextLocation =
          (piece->location + (1 - 2 * piece->movingDirection) * moveValue + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;

      StandardCell *nextCell = &standardArea[nextLocation];

      // standardArea --> standardArea / emptyCell
      if (nextCell->pieceCount == 0) {
        move.moveType = 5;
        move.startStandardCell = &standardArea[piece->location];
        move.endStandardCell = nextCell;
      }
      // standardArea --> standardArea / enemyPiece
      else if (nextCell->pieceCount == 1 && nextCell->piece->color->colorNum != piece->color->colorNum) {
        move.moveType = 8;
        move.startStandardCell = &standardArea[piece->location];
        move.endStandardCell = nextCell;
        move.enemyPiece = nextCell->piece;
      }
    }
  } else if (piece->locationType == HOME_STRAIGHT) {
    // homeStraight --> home
    if (moveValue == 5 - piece->location) {
      move.moveType = 13;
      move.homeStraightCell = &homeStraights[piece->color->colorNum][piece->location];
      move.home = &homes[piece->color->colorNum];
    }
  }

  return move;
}

short breakBlockExecute(Block *block, short blockSize, short *moveValues) {
  Move breakMoves[blockSize];
  short mV[blockSize];
  char isValid = TRUE;

  for (short i = 0; i < blockSize; i++) {
    mV[i] = moveValues[i];
  }

  for (short piece = 0; piece < blockSize; piece++) {
    breakMoves[piece] = blockBreakMoveCalculate(block->pieces[piece], mV[piece]);
    isValid *= breakMoves[piece].moveType;
  }

  if (isValid) {
    for (short move = 0; move < blockSize; move++) {
      block->pieces[move]->blockNum = NULL;
      moveExecute(&breakMoves[move]);
    }
    return TRUE;
  } else {
    return FALSE;
  }
}

void breakBlock(Block *block, short blockSize) {
  short mV[MAX_PIECES_PER_BLOCK] = {0};
  Block *b = block;
  char locationType = block->pieces[0]->locationType;
  short location = block->pieces[0]->location;
  short color = block->color->colorNum;

  printf("============================\n");
  printf("%s's block of %d containing ", b->color->colorName, b->pieceCount);
  for (short piece = 0; piece < b->pieceCount; piece++) {
    printf("%s,", b->pieces[piece]->name);
  }

  if (block->pieces[0]->locationType == STANDARD_AREA) {
    printf("\b at standard-area cell L%d will be broken.\n", block->pieces[0]->location);
  } else if (block->pieces[0]->locationType == HOME_STRAIGHT) {
    printf("\b at home-straight L%d will be broken.\n", block->pieces[0]->location);
  }

  for (mV[0] = 0; mV[0] < MOVEMENT_TOTAL; mV[0]++) {
    for (mV[1] = 0; mV[1] < MOVEMENT_TOTAL; mV[1]++) {
      if (mV[1] == mV[0]) {
        continue;
      } else if (blockSize == 2 && mV[0] + mV[1] == MOVEMENT_TOTAL) {
        // block 2 related code
        if (breakBlockExecute(b, 2, mV)) {
          resetBlock(block);
          if (locationType == STANDARD_AREA) {
            standardArea[location].block = NULL;
            standardArea[location].pieceCount = 0;
          } else if (locationType == HOME_STRAIGHT) {
            homeStraights[color][location].block = NULL;
            homeStraights[color][location].pieceCount = 0;
          }
          return;
        }
      } else {
        for (mV[2] = 0; mV[2] < MOVEMENT_TOTAL; mV[2]++) {
          if (mV[2] == mV[0] || mV[2] == mV[1]) {
            continue;
          } else if (blockSize == 3 && mV[0] + mV[1] + mV[2] == MOVEMENT_TOTAL) {
            // block 3 related code
            if (breakBlockExecute(b, 3, mV)) {
              resetBlock(block);
              if (locationType == STANDARD_AREA) {
                standardArea[location].block = NULL;
                standardArea[location].pieceCount = 0;
              } else if (locationType == HOME_STRAIGHT) {
                homeStraights[color][location].block = NULL;
                homeStraights[color][location].pieceCount = 0;
              }
              return;
            }
          } else {
            for (mV[3] = 0; mV[3] < MOVEMENT_TOTAL; mV[3]++) {
              if (mV[3] == mV[0] || mV[3] == mV[1] || mV[3] == mV[2]) {
                continue;
              } else if (blockSize == 4 && mV[0] + mV[1] + mV[2] + mV[3] == MOVEMENT_TOTAL) {
                // block 4 related code
                if (breakBlockExecute(b, 4, mV)) {
                  resetBlock(block);
                  if (locationType == STANDARD_AREA) {
                    standardArea[location].block = NULL;
                    standardArea[location].pieceCount = 0;
                  } else if (locationType == HOME_STRAIGHT) {
                    homeStraights[color][location].block = NULL;
                    homeStraights[color][location].pieceCount = 0;
                  }
                  return;
                }
              }
            }
          }
        }
      }
    }
  }
  printf("Block was failed to be broken.\n");
  return;
}

void blockedMessage(Move *move) {
  Move *m = move;
  StandardCell *blockingCell =
      &standardArea[(m->startStandardCell->cellIndex + m->check.distanceToBlock + MAX_STANDARD_CELLS) %
                    MAX_STANDARD_CELLS];
  Block *enemyBlock = blockingCell->block;
  if (!m->check.doesPieceOrBlockMeetBlock || m->check.canCaptureBlock) {
    return;
  }
  if (m->piece != NULL) {
    if (m->check.doesPieceOrBlockPassApproach == TRUE && isEligibleForHomeStraight(m->piece, NULL)) {
      short hscNum = m->check.initialMoveValue - m->check.distanceToApproach - 1;
      printf("%s is blocked from moving from standard-area L%d to home-straight L%d by %s's block of %d containing ",
             m->piece->name, m->startStandardCell->cellIndex, hscNum, enemyBlock->color->colorName,
             enemyBlock->pieceCount);
      for (short piece = 0; piece < enemyBlock->pieceCount; piece++) {
        printf("%s,", enemyBlock->pieces[piece]->name);
      }
      printf("\b.\n");
    } else {
      short sacNum = (m->startStandardCell->cellIndex + m->check.initialMoveValue -
                      2 * m->piece->movingDirection * m->check.initialMoveValue + MAX_STANDARD_CELLS) %
                     MAX_STANDARD_CELLS;
      printf("%s is blocked from moving from standard-area L%d to L%d by %s's block of %d containing ", m->piece->name,
             m->startStandardCell->cellIndex, sacNum, enemyBlock->color->colorName, enemyBlock->pieceCount);
      for (short piece = 0; piece < enemyBlock->pieceCount; piece++) {
        printf("%s,", enemyBlock->pieces[piece]->name);
      }
      printf("\b.\n");
    }
  } else if (m->block != NULL) {
    if (m->check.doesPieceOrBlockPassApproach == TRUE && isEligibleForHomeStraight(NULL, m->block)) {
      short hscNum = m->check.initialMoveValue - m->check.distanceToApproach - 1;
      printf("%s's block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b was blocked from moving from standard-area L%d to home-straight L%d by %s's block of %d containing ",
             m->startStandardCell->cellIndex, hscNum, enemyBlock->color->colorName, enemyBlock->pieceCount);
      for (short piece = 0; piece < enemyBlock->pieceCount; piece++) {
        printf("%s,", enemyBlock->pieces[piece]->name);
      }
      printf("\b.\n");
    } else {
      short sacNum = (m->startStandardCell->cellIndex + m->check.initialMoveValue -
                      2 * m->block->movingDirection * m->check.initialMoveValue + MAX_STANDARD_CELLS) %
                     MAX_STANDARD_CELLS;
      printf("%s's block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b was blocked from moving from standard-area L%d to L%d by %s's block of %d containing ",
             m->startStandardCell->cellIndex, sacNum, enemyBlock->color->colorName, enemyBlock->pieceCount);
      for (short piece = 0; piece < enemyBlock->pieceCount; piece++) {
        printf("%s,", enemyBlock->pieces[piece]->name);
      }
      printf("\b.\n");
    }
  }
}

void generateMysteryCell() {
  if (colorCycleCount % 16 == 8) {
    char isGenerationSuccessful = FALSE;

    while (!isGenerationSuccessful) {
      short candidateCell = rand() % MAX_STANDARD_CELLS;

      if ((mysteryCell.cell == NULL || candidateCell != mysteryCell.cell->cellIndex) &&
          standardArea[candidateCell].pieceCount == 0) {
        mysteryCell.cell = &standardArea[candidateCell];
        isGenerationSuccessful = TRUE;
        printf("The mystery cell is at L%d for the next 4 rounds.\n", mysteryCell.cell->cellIndex);
      }
    }
  }
}

void *mysteryTeleport(void *element, short elementType, short teleportLocation, short *returnType) {
  StandardCell *originCell = NULL;
  StandardCell *targetCell = &standardArea[teleportLocation];

  if (elementType == PIECE) {
    Piece *piece = (Piece *)element;
    originCell = &standardArea[piece->location];

    // empty piece
    if (targetCell->pieceCount == 0) {
      originCell->piece = NULL;
      originCell->pieceCount = 0;
      targetCell->piece = piece;
      targetCell->pieceCount = 1;
      piece->location = targetCell->cellIndex;
      *returnType = 0;
      return targetCell->piece;
    } else if (targetCell->piece != NULL) {

      // player piece
      if (targetCell->piece->color->colorNum == piece->color->colorNum) {
        Piece *playerPiece = targetCell->piece;
        printf("%s teleported onto %s and created a block of 2\n", piece->name, playerPiece->name);
        originCell->piece = NULL;
        originCell->pieceCount = 0;
        createBlock(piece, playerPiece, targetCell, STANDARD_AREA);
        *returnType = 1;
        return targetCell->block;
      }

      // enemy piece
      else {
        Piece *enemyPiece = targetCell->piece;
        printf("%s teleported onto %s's %s and eliminated it.\n", piece->name, enemyPiece->color->colorName,
               enemyPiece->name);
        originCell->piece = NULL;
        originCell->pieceCount = 0;
        targetCell->piece = piece;
        targetCell->pieceCount = 1;
        piece->capturedPieceCount++;
        piece->location = targetCell->cellIndex;
        rebasePiece(enemyPiece);
        *returnType = 0;
        return targetCell->piece;
      }

    } else if (targetCell->block != NULL) {

      // player block
      if (targetCell->block->color->colorNum == piece->color->colorNum) {
        Block *playerBlock = targetCell->block;
        printf("%s teleported onto %s's block of %d containing ", piece->name, playerBlock->color->colorName,
               playerBlock->pieceCount);
        for (short pieceNum = 0; pieceNum < playerBlock->pieceCount; pieceNum++) {
          printf("%s,", playerBlock->pieces[pieceNum]->name);
        }
        printf("and expanded upon it.\n");
        originCell->piece = NULL;
        originCell->pieceCount = 0;
        expandBlockByPiece(piece, playerBlock, targetCell, STANDARD_AREA);
        *returnType = 1;
        return targetCell->block;
      }

      // enemy block
      else {
        Block *enemyBlock = targetCell->block;
        printf("%s's block of %d is at the teleporting location\n", enemyBlock->color->colorName,
               enemyBlock->pieceCount);
        *returnType = -1;
        return NULL;
      }
    }

  } else if (elementType == BLOCK) {
    Block *block = (Block *)element;
    originCell = &standardArea[block->pieces[0]->location];

    // empty piece
    if (targetCell->pieceCount == 0) {
      originCell->block = NULL;
      originCell->pieceCount = 0;
      targetCell->block = block;
      targetCell->pieceCount = block->pieceCount;
      for (short pieceNum = 0; pieceNum < block->pieceCount; pieceNum++) {
        block->pieces[pieceNum]->location = targetCell->cellIndex;
      }
      *returnType = 1;
      return targetCell->block;
    } else if (targetCell->piece != NULL) {

      // player piece
      if (targetCell->piece->color->colorNum == block->color->colorNum) {
        Piece *playerPiece = targetCell->piece;
        printf("The block landed onto %s and expanded upon it.\n", playerPiece->name);
        originCell->block = NULL;
        originCell->pieceCount = 0;
        expandPieceByBlock(block, playerPiece, targetCell, STANDARD_AREA);
        *returnType = 1;
        return targetCell->block;
      }

      // enemy piece
      else {
        Piece *enemyPiece = targetCell->piece;
        printf("The block landed onto %s's %s and eliminated it.\n", enemyPiece->color->colorName, enemyPiece->name);
        originCell->block = NULL;
        originCell->pieceCount = 0;
        targetCell->block = block;
        targetCell->piece = NULL;
        targetCell->pieceCount = block->pieceCount;
        for (short pieceNum = 0; pieceNum < block->pieceCount; pieceNum++) {
          block->pieces[pieceNum]->capturedPieceCount++;
          block->pieces[pieceNum]->location = targetCell->cellIndex;
        }
        rebasePiece(enemyPiece);
        *returnType = 1;
        return targetCell->block;
      }

    } else if (targetCell->block != NULL) {

      // player block
      if (targetCell->block->color->colorNum == block->color->colorNum) {
        Block *playerBlock = targetCell->block;
        printf("The block landed onto %s's block of %d containing ", playerBlock->color->colorName,
               playerBlock->pieceCount);
        for (short pieceNum = 0; pieceNum < playerBlock->pieceCount; pieceNum++) {
          printf("%s,", playerBlock->pieces[pieceNum]->name);
        }
        printf("and expanded upon it.\n");
        originCell->block = NULL;
        originCell->pieceCount = 0;
        expandBlockByBlock(block, playerBlock, targetCell, STANDARD_AREA);
        *returnType = 1;
        return targetCell->block;
      }
      // enemy block
      else {

        // can eliminate
        if (block->pieceCount >= targetCell->block->pieceCount) {
          Block *enemyBlock = targetCell->block;
          printf("The block landed onto %s's block of %d containing ", enemyBlock->color->colorName,
                 enemyBlock->pieceCount);
          for (short pieceNum = 0; pieceNum < enemyBlock->pieceCount; pieceNum++) {
            printf("%s,", enemyBlock->pieces[pieceNum]->name);
          }
          printf("\b and eliminated it.\n");
          originCell->block = NULL;
          originCell->pieceCount = 0;
          targetCell->block = block;
          targetCell->pieceCount = block->pieceCount;
          for (short pieceNum = 0; pieceNum < block->pieceCount; pieceNum++) {
            block->pieces[pieceNum]->capturedPieceCount++;
            block->pieces[pieceNum]->location = targetCell->cellIndex;
          }
          for (short pieceNum = 0; pieceNum < enemyBlock->pieceCount; pieceNum++) {
            rebasePiece(enemyBlock->pieces[pieceNum]);
          }
          resetBlock(enemyBlock);
          *returnType = 1;
          return targetCell->block;
        }

        // cannot eliminate
        else {
          Block *enemyBlock = targetCell->block;
          printf("The block landed onto %s's block of %d containing ", enemyBlock->color->colorName,
                 enemyBlock->pieceCount);
          for (short pieceNum = 0; pieceNum < enemyBlock->pieceCount; pieceNum++) {
            printf("%s,", enemyBlock->pieces[pieceNum]->name);
          }
          printf("\b and failed to eliminate it.\n");
          *returnType = -1;
          return NULL;
        }
      }
    }
  }
  *returnType = -1;
  return NULL;
}

void executeMysteryEffect() {
  if (mysteryCell.cell == NULL) {
    return;
  }

  Piece *piece = standardArea[mysteryCell.cell->cellIndex].piece;
  Block *block = standardArea[mysteryCell.cell->cellIndex].block;

  short cellEffect = rand() % MYSTERY_EFFECT_TOTAL;
  short teleportLocation;
  short returnType = 0; // 0 - piece, 1 - block
  void *returnElement = NULL;

  if (piece != NULL) {
    switch (cellEffect) {
      // Bhawana
      case 0: {
        printf("%s's %s is teleporting to Bhawana.\n", piece->color->colorName, piece->name);
        teleportLocation = BHAWANA;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          returnElement = mysteryTeleport(piece, PIECE, teleportLocation, &returnType);
        } else {
          returnElement = piece;
          returnType = 0;
        }
        short auraType = coinToss() + 2;

        if (returnType == 0) {
          Piece *returnP = (Piece *)returnElement;
          returnP->mysteryEffect = auraType;
          returnP->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
          switch (auraType) {
            case AURA_ENERGIZED:
              printf("%s feels energized, and movement speed doubles.\n", returnP->name);
              break;
            case AURA_SICKNESS:
              printf("%s feels sick, and movement speed halves.\n", returnP->name);
              break;
          }
        } else if (returnType == 1) {
          Block *returnB = (Block *)returnElement;
          returnB->mysteryEffect = auraType;
          returnB->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
          printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
          for (short piece = 0; piece < returnB->pieceCount; piece++) {
            printf("%s,", returnB->pieces[piece]->name);
          }
          switch (auraType) {
            case AURA_ENERGIZED:
              printf("\b feels energized, and movement speed doubles.\n");
              break;
            case AURA_SICKNESS:
              printf("\b feels sick, and movement speed halves.\n");
              break;
          }
        } else if (returnType == -1) {
          printf("Teleportation failed. No mystery effect was granted.\n");
        }
        break;
      }
      // Kotuwa
      case 1: {
        printf("%s's %s is teleporting to Kotuwa.\n", piece->color->colorName, piece->name);
        teleportLocation = KOTUWA;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          returnElement = mysteryTeleport(piece, PIECE, teleportLocation, &returnType);
        } else {
          returnElement = piece;
          returnType = 0;
        }

        if (returnType == 0) {
          Piece *returnP = (Piece *)returnElement;
          returnP->mysteryEffect = IMMOVABLE;
          returnP->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
          printf("%s attends briefing and cannot move for four rounds.\n", returnP->name);
        } else if (returnType == 1) {
          Block *returnB = (Block *)returnElement;
          returnB->mysteryEffect = IMMOVABLE;
          returnB->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
          printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
          for (short piece = 0; piece < returnB->pieceCount; piece++) {
            printf("%s,", returnB->pieces[piece]->name);
          }
          printf("\b attends briefing and cannot move for four rounds.\n");
        } else if (returnType == -1) {
          printf("Teleportation failed. No mystery effect was granted.\n");
        }
        break;
      }
      // Pita-Kotuwa
      case 2: {
        printf("%s's %s is teleporting to Pita-Kotuwa.\n", piece->color->colorName, piece->name);
        teleportLocation = PITA_KOTUWA;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          returnElement = mysteryTeleport(piece, PIECE, teleportLocation, &returnType);
        } else {
          returnElement = piece;
          returnType = 0;
        }

        if (returnType == 0) {
          Piece *returnP = (Piece *)returnElement;
          if (returnP->movingDirection == CLOCKWISE) {
            returnP->movingDirection = COUNTER_CLOCKWISE;
            printf("%s, which was moving clockwise, has changed to moving counter-clockwise", returnP->name);
          } else if (returnP->movingDirection == COUNTER_CLOCKWISE) {
            returnType = 0;
            teleportLocation = KOTUWA;
            returnElement = mysteryTeleport(returnP, PIECE, teleportLocation, &returnType);
            printf("%s is moving in a counter-clockwise direction. Teleporting to Kotuwa from Pita-Kotuwa.\n",
                   returnP->name);
            if (returnType == 0) {
              Piece *returnP = (Piece *)returnElement;
              returnP->mysteryEffect = IMMOVABLE;
              returnP->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
              printf("%s attends briefing and cannot move for four rounds.\n", returnP->name);
            } else if (returnType == 1) {
              Block *returnB = (Block *)returnElement;
              returnB->mysteryEffect = IMMOVABLE;
              returnB->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
              printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
              for (short piece = 0; piece < returnB->pieceCount; piece++) {
                printf("%s,", returnB->pieces[piece]->name);
              }
              printf("\b attends briefing and cannot move for four rounds.\n");
            } else if (returnType == -1) {
              printf("Teleportation failed. No mystery effect was granted.\n");
            }
          }
        } else if (returnType == 1) {
          Block *returnB = (Block *)returnElement;
          if (returnB->movingDirection == CLOCKWISE) {
            returnB->movingDirection = COUNTER_CLOCKWISE;
            printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
            for (short piece = 0; piece < returnB->pieceCount; piece++) {
              printf("%s,", returnB->pieces[piece]->name);
            }
            printf("\b has changed to moving counter-clockwise.\n");
          } else if (returnB->movingDirection == COUNTER_CLOCKWISE) {
            returnType = 0;
            teleportLocation = KOTUWA;
            returnElement = mysteryTeleport(returnB, BLOCK, teleportLocation, &returnType);
            printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
            for (short piece = 0; piece < returnB->pieceCount; piece++) {
              printf("%s,", returnB->pieces[piece]->name);
            }
            printf("\b is moving in a clockwise direction. Teleporting to Kotuwa from Pita-Kotuwa.\n");
            if (returnType == 1) {
              Block *returnB = (Block *)returnElement;
              returnB->mysteryEffect = IMMOVABLE;
              returnB->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
              printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
              for (short piece = 0; piece < returnB->pieceCount; piece++) {
                printf("%s,", returnB->pieces[piece]->name);
              }
              printf("\b attends briefing and cannot move for four rounds.\n");
            } else if (returnType == -1) {
              printf("Teleportation failed. No mystery effect was granted.\n");
            }
          }
        } else if (returnType == -1) {
          printf("Teleportation failed. No mystery effect was granted.\n");
        }
        break;
      }
      // Base
      case 3: {
        printf("%s's %s is teleporting to Base.\n", piece->color->colorName, piece->name);
        standardArea[piece->location].piece = NULL;
        standardArea[piece->location].pieceCount = 0;
        rebasePiece(piece);
        break;
      }
      // Starting Cell
      case 4: {
        printf("%s's %s is teleporting to Starting Cell (X).\n", piece->color->colorName, piece->name);
        teleportLocation = piece->startingCell;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          mysteryTeleport(piece, PIECE, teleportLocation, &returnType);
        } else {
          returnType = 0;
        }

        if (returnType == -1) {
          printf("Teleportation failed.\n");
        }
        break;
      }
      // Approach
      case 5: {
        printf("%s's %s is teleported to Approach.\n", piece->color->colorName, piece->name);
        teleportLocation = piece->approach;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          mysteryTeleport(piece, PIECE, teleportLocation, &returnType);
        } else {
          returnType = 0;
        }

        if (returnType == -1) {
          printf("Teleportation failed.\n");
        }
        break;
      }
      default:
        break;
    }
  } else if (block != NULL) {
    switch (cellEffect) {
      // Bhawana
      case 0: {
        printf("%s's block of %d containing ", block->color->colorName, block->pieceCount);
        for (short piece = 0; piece < block->pieceCount; piece++) {
          printf("%s,", block->pieces[piece]->name);
        }
        printf("\b is teleporting to Bhawana\n");

        teleportLocation = BHAWANA;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          returnElement = mysteryTeleport(block, BLOCK, teleportLocation, &returnType);
        } else {
          returnElement = block;
          returnType = 1;
        }
        short auraType = coinToss() + 2;

        if (returnType == 1) {
          Block *returnB = (Block *)returnElement;
          returnB->mysteryEffect = auraType;
          returnB->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
          printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
          for (short piece = 0; piece < returnB->pieceCount; piece++) {
            printf("%s,", returnB->pieces[piece]->name);
          }
          switch (auraType) {
            case AURA_ENERGIZED:
              printf("\b feels energized, and movement speed doubles.\n");
              break;
            case AURA_SICKNESS:
              printf("\b feels sick, and movement speed halves.\n");
              break;
          }
        } else if (returnType == -1) {
          printf("Teleportation failed. No mystery effect was granted.\n");
        }
        break;
      }
      // Kotuwa
      case 1: {
        printf("%s's block of %d containing ", block->color->colorName, block->pieceCount);
        for (short piece = 0; piece < block->pieceCount; piece++) {
          printf("%s,", block->pieces[piece]->name);
        }
        printf("\b teleporting to Kotuwa\n");

        teleportLocation = KOTUWA;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          returnElement = mysteryTeleport(block, BLOCK, teleportLocation, &returnType);
        } else {
          returnElement = block;
          returnType = 1;
        }

        if (returnType == 1) {
          Block *returnB = (Block *)returnElement;
          returnB->mysteryEffect = IMMOVABLE;
          returnB->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
          printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
          for (short piece = 0; piece < returnB->pieceCount; piece++) {
            printf("%s,", returnB->pieces[piece]->name);
          }
          printf("\b attends briefing and cannot move for four rounds.\n");
        } else if (returnType == -1) {
          printf("Teleportation failed. No mystery effect was granted.\n");
        }
        break;
      }
      // Pita-Kotuwa
      case 2: {
        printf("%s's block of %d containing ", block->color->colorName, block->pieceCount);
        for (short piece = 0; piece < block->pieceCount; piece++) {
          printf("%s,", block->pieces[piece]->name);
        }
        printf("\b teleporting to Pita-Kotuwa.\n");

        teleportLocation = PITA_KOTUWA;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          returnElement = mysteryTeleport(block, BLOCK, teleportLocation, &returnType);
        } else {
          returnElement = block;
          returnType = 1;
        }

        if (returnType == 1) {
          Block *returnB = (Block *)returnElement;
          if (returnB->movingDirection == CLOCKWISE) {
            returnB->movingDirection = COUNTER_CLOCKWISE;
            printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
            for (short piece = 0; piece < returnB->pieceCount; piece++) {
              printf("%s,", returnB->pieces[piece]->name);
            }
            printf("\b has changed to moving counter-clockwise.\n");
          } else if (returnB->movingDirection == COUNTER_CLOCKWISE) {
            returnType = 0;
            teleportLocation = KOTUWA;
            returnElement = mysteryTeleport(returnB, BLOCK, teleportLocation, &returnType);
            printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
            for (short piece = 0; piece < returnB->pieceCount; piece++) {
              printf("%s,", returnB->pieces[piece]->name);
            }
            printf("\b is moving in a clockwise direction. Teleporting to Kotuwa "
                   "from Pita-Kotuwa.\n");
            if (returnType == 1) {
              Block *returnB = (Block *)returnElement;
              returnB->mysteryEffect = IMMOVABLE;
              returnB->mysteryEffectDuration = MAX_MYSTERY_EFFECT_DURATION;
              printf("%s's block of %d containing ", returnB->color->colorName, returnB->pieceCount);
              for (short piece = 0; piece < returnB->pieceCount; piece++) {
                printf("%s,", returnB->pieces[piece]->name);
              }
              printf("\b attends briefing and cannot move for four rounds.\n");
            } else if (returnType == -1) {
              printf("Teleportation failed. No mystery effect was granted.\n");
            }
          }
        } else if (returnType == -1) {
          printf("Teleportation failed. No mystery effect was granted.\n");
        }
        break;
      }
      // Base
      case 3: {
        printf("%s's block of %d containing ", block->color->colorName, block->pieceCount);
        for (short piece = 0; piece < block->pieceCount; piece++) {
          printf("%s,", block->pieces[piece]->name);
        }
        printf("\b teleported to Base\n");

        standardArea[block->pieces[0]->location].block = NULL;
        standardArea[block->pieces[0]->location].pieceCount = 0;
        for (short piece = 0; piece < block->pieceCount; piece++) {
          rebasePiece(block->pieces[piece]);
        }
        resetBlock(block);
        break;
      }
      // Starting Cell
      case 4: {
        printf("%s's block of %d containing ", block->color->colorName, block->pieceCount);
        for (short piece = 0; piece < block->pieceCount; piece++) {
          printf("%s,", block->pieces[piece]->name);
        }
        printf("\b is teleporting to Starting Cell (X)\n");

        teleportLocation = block->pieces[0]->startingCell;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          mysteryTeleport(block, BLOCK, teleportLocation, &returnType);
        } else {
          returnType = 1;
        }

        if (returnType == -1) {
          printf("Teleportation failed.\n");
        }
        break;
      }
      // Approach
      case 5: {
        printf("%s's block of %d containing ", block->color->colorName, block->pieceCount);
        for (short piece = 0; piece < block->pieceCount; piece++) {
          printf("%s,", block->pieces[piece]->name);
        }
        printf("\b teleported to Approach\n");

        teleportLocation = block->pieces[0]->startingCell;
        if (mysteryCell.cell->cellIndex != teleportLocation) {
          mysteryTeleport(block, BLOCK, teleportLocation, &returnType);
        } else {
          returnType = 1;
        }
        if (returnType == -1) {
          printf("Teleportation failed.\n");
        }
        break;
      }
      default:
        break;
    }
  }
}

void decayMysteryCellEffect() {
  for (short color = 0; color < COLOR_TOTAL; color++) {
    for (short piece = 0; piece < PIECE_TOTAL_PER_COLOR; piece++) {
      Piece *p = &pieces[color][piece];
      if (p->mysteryEffectDuration > 0) {
        p->mysteryEffectDuration--;
      } else if (p->mysteryEffectDuration == 0) {
        p->mysteryEffect = 0;
      }
    }
  }

  for (short color = 0; color < COLOR_TOTAL; color++) {
    for (short block = 0; block < MAX_BLOCKS_PER_COLOR; block++) {
      Block *b = &blocks[color][block];
      if (b->mysteryEffectDuration > 0) {
        b->mysteryEffectDuration--;
      } else if (b->mysteryEffectDuration == 0) {
        b->mysteryEffect = 0;
      }
    }
  }
}

short isEligibleForHomeStraight(Piece *piece, Block *block) {
  if (piece != NULL) {
    if (piece->capturedPieceCount > 0 && piece->movingDirection == COUNTER_CLOCKWISE && piece->approachPassCount > 0) {
      return TRUE;
    } else if (piece->capturedPieceCount > 0 && piece->movingDirection == CLOCKWISE) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else if (block != NULL) {
    for (short pieceNum = 0; pieceNum < MAX_PIECES_PER_BLOCK; pieceNum++) {
      Piece *p = block->pieces[pieceNum];
      if (p == NULL) {
        continue;
      } else if (p->capturedPieceCount == 0) {
        return FALSE;
      } else if (p->movingDirection == COUNTER_CLOCKWISE && p->approachPassCount == 0) {
        return FALSE;
      }
    }

    return TRUE;
  }
  return FALSE;
}

void b_sA_emptyCell(Move *move, Piece *piece) {
  // move piece from base to standardArea
  Move *m = move;
  m->moveType = 1;
  m->piece = piece;
  m->base = &bases[currentTurnColor];
  m->endStandardCell = &standardArea[piece->startingCell];
}

void b_sA_playerPiece(Move *move, Piece *piece) {
  // create block on standardArea
  Move *m = move;
  m->moveType = 2;
  m->piece = piece;
  m->base = &bases[currentTurnColor];
  m->endStandardCell = &standardArea[piece->startingCell];
  m->playerPiece = m->endStandardCell->piece;
}

void b_sA_playerBlock(Move *move, Piece *piece) {
  Move *m = move;
  m->moveType = 3;
  m->piece = piece;
  m->base = &bases[currentTurnColor];
  m->endStandardCell = &standardArea[piece->startingCell];
  m->playerBlock = m->endStandardCell->block;
}

void b_sA_enemyPiece(Move *move, Piece *piece) {
  Move *m = move;
  m->moveType = 4;
  m->piece = piece;
  m->base = &bases[currentTurnColor];
  m->endStandardCell = &standardArea[piece->startingCell];
  m->enemyPiece = standardArea[piece->startingCell].piece;
}

void sA_sA_emptyCell(Move *move, Piece *piece, Block *block, short targetLocation, Check *check) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 5;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->check = *(check);
  } else if (block != NULL) {
    m->moveType = 14;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->check = *(check);
  }
}

void sA_sA_playerPiece(Move *move, Piece *piece, Block *block, short targetLocation, Check *check) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 6;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->playerPiece = standardArea[targetLocation].piece;
    m->check = *(check);
  } else if (block != NULL) {
    m->moveType = 15;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->playerPiece = standardArea[targetLocation].piece;
    m->check = *(check);
  }
}

void sA_sA_playerBlock(Move *move, Piece *piece, Block *block, short targetLocation, Check *check) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 7;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->playerBlock = standardArea[targetLocation].block;
    m->check = *(check);
  } else if (block != NULL) {
    m->moveType = 16;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->playerBlock = standardArea[targetLocation].block;
    m->check = *(check);
  }
}

void sA_sA_enemyPiece(Move *move, Piece *piece, Block *block, short targetLocation, Check *check) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 8;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->enemyPiece = standardArea[targetLocation].piece;
    m->check = *(check);
  } else if (block != NULL) {
    m->moveType = 17;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->endStandardCell = &standardArea[targetLocation];
    m->enemyPiece = standardArea[targetLocation].piece;
    m->check = *(check);
  }
}

void sA_sA_enemyBlock(Move *move, Block *block, short targetLocation, Check *check) {
  Move *m = move;
  m->moveType = 18;
  m->block = block;
  m->startStandardCell = &standardArea[block->pieces[0]->location];
  m->endStandardCell = &standardArea[targetLocation];
  m->enemyBlock = standardArea[targetLocation].block;
  m->check = *(check);
}

void sA_hS_emptyCell(Move *move, Piece *piece, Block *block, short targetLocation) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 9;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->homeStraightCell = &homeStraights[currentTurnColor][targetLocation];
  } else if (block != NULL) {
    m->moveType = 19;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->homeStraightCell = &homeStraights[currentTurnColor][targetLocation];
  }
}

void sA_hS_playerPiece(Move *move, Piece *piece, Block *block, short targetLocation) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 10;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->homeStraightCell = &homeStraights[currentTurnColor][targetLocation];
    m->playerPiece = homeStraights[currentTurnColor][targetLocation].piece;
  } else if (block != NULL) {
    m->moveType = 20;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->homeStraightCell = &homeStraights[currentTurnColor][targetLocation];
    m->playerPiece = homeStraights[currentTurnColor][targetLocation].piece;
  }
  return;
}

void sA_hS_playerBlock(Move *move, Piece *piece, Block *block, short targetLocation) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 11;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->homeStraightCell = &homeStraights[currentTurnColor][targetLocation];
    m->playerBlock = homeStraights[currentTurnColor][targetLocation - 1].block;
  } else if (block != NULL) {
    m->moveType = 21;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->homeStraightCell = &homeStraights[currentTurnColor][targetLocation];
    m->playerBlock = homeStraights[currentTurnColor][targetLocation].block;
  }
}

void sA_H(Move *move, Piece *piece, Block *block) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 12;
    m->piece = piece;
    m->startStandardCell = &standardArea[piece->location];
    m->home = &homes[currentTurnColor];
  } else if (block != NULL) {
    m->moveType = 22;
    m->block = block;
    m->startStandardCell = &standardArea[block->pieces[0]->location];
    m->home = &homes[currentTurnColor];
  }
}

void hS_h(Move *move, Piece *piece, Block *block, char isFromBlock) {
  Move *m = move;
  if (piece != NULL) {
    m->moveType = 13;
    m->piece = piece;
    m->homeStraightCell = &homeStraights[piece->color->colorNum][piece->location];
    m->home = &homes[currentTurnColor];
    m->isFromBlock = isFromBlock;
    if (isFromBlock) {
      m->block = &blocks[m->piece->color->colorNum][*(m->piece->blockNum)];
    }
  } else if (block != NULL) {
    m->moveType = 23;
    m->block = block;
    m->homeStraightCell = &homeStraights[currentTurnColor][block->pieces[0]->location];
    m->home = &homes[currentTurnColor];
  }
}

void calculateMoves() {

  resetMoves();

  // TODO: refine this function to remove all location to location move assigning functions

  // === SECTION: calculate moves for pieces ===
  for (short pieceIndex = 0; pieceIndex < PIECE_TOTAL_PER_COLOR; pieceIndex++) {
    short moveValue = diceRolledValue;
    short moveLeft = 0;

    Piece *piece = &pieces[currentTurnColor][pieceIndex];
    const short locationType = piece->locationType;

    // ignore if the piece is in a block
    if (piece->blockNum != NULL) {
      continue;
    }

    // check for mystery cell effects
    switch (piece->mysteryEffect) {
      case 1:
        continue;
      case 2:
        moveValue *= 2;
        break;
      case 3:
        moveValue /= 2;
        break;
    }

    if (moveValue == 0) {
      continue;
    }

    if (locationType == BASE && diceRolledValue == DIE_ROLL_MAX) {
      // base --> standardArea
      if (standardArea[piece->startingCell].pieceCount == 0) {
        b_sA_emptyCell(&moves[++moveIndex], piece);
      } else if (standardArea[piece->startingCell].piece != NULL) {
        if (standardArea[piece->startingCell].piece->color->colorNum == currentTurnColor) {
          b_sA_playerPiece(&moves[++moveIndex], piece);
        } else if (standardArea[piece->startingCell].piece->color->colorNum != currentTurnColor) {
          b_sA_enemyPiece(&moves[++moveIndex], piece);
        }
      } else if (standardArea[piece->startingCell].block != NULL &&
                 standardArea[piece->startingCell].block->color->colorNum == currentTurnColor) {
        b_sA_playerBlock(&moves[++moveIndex], piece);
      }
    } else if (locationType == STANDARD_AREA) {
      Check check = nextCellsInspection(piece, NULL, moveValue);

      if (check.doesPieceOrBlockPassApproach == TRUE && isEligibleForHomeStraight(piece, NULL)) {
        if (moveValue - check.distanceToApproach == 6) {
          // standardArea --> home
          sA_H(&moves[++moveIndex], piece, NULL);
          continue;
        } else if (moveValue - check.distanceToApproach > 6) {
          moveLeft = 5;
        } else {
          moveLeft = moveValue - check.distanceToApproach;
        }

        // standardArea --> homeStraight
        if (homeStraights[currentTurnColor][moveLeft - 1].pieceCount == 0) {
          // empty cell
          sA_hS_emptyCell(&moves[++moveIndex], piece, NULL, moveLeft - 1);
        } else if (homeStraights[currentTurnColor][moveLeft - 1].pieceCount == 1) {
          // same color piece, create block
          sA_hS_playerPiece(&moves[++moveIndex], piece, NULL, moveLeft - 1);
        } else if (homeStraights[currentTurnColor][moveLeft - 1].pieceCount > 2) {
          // same color block, expand block
          sA_hS_playerBlock(&moves[++moveIndex], piece, NULL, moveLeft - 1);
        }
      }

      // standardArea --> standardArea
      else {
        // moveValue evaluation
        if (check.doesPieceOrBlockMeetBlock == TRUE) {
          moveValue = check.distanceToBlock - 1 + 2 * (short)piece->movingDirection;
          if (moveValue == 0)
            continue;
        } else if (piece->movingDirection == COUNTER_CLOCKWISE) {
          moveValue *= -1;
        }

        short nextLocation = (piece->location + moveValue + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;

        if (standardArea[nextLocation].pieceCount == 0) {
          // empty cell
          sA_sA_emptyCell(&moves[++moveIndex], piece, NULL, nextLocation, &check);
        } else if (standardArea[nextLocation].pieceCount == 1) {
          if (standardArea[nextLocation].piece->color->colorNum == currentTurnColor) {
            // player piece, create block
            sA_sA_playerPiece(&moves[++moveIndex], piece, NULL, nextLocation, &check);
          } else {
            // enemy piece, capture piece
            sA_sA_enemyPiece(&moves[++moveIndex], piece, NULL, nextLocation, &check);
          }
        } else if (standardArea[nextLocation].pieceCount > 1 &&
                   standardArea[nextLocation].block->color->colorNum == currentTurnColor) {
          // same color block, expand block
          sA_sA_playerBlock(&moves[++moveIndex], piece, NULL, nextLocation, &check);
        }
      }
    }

    // homeRow --> home
    else if (locationType == HOME_STRAIGHT) {
      short cellCountToHome = 5 - piece->location;

      if (moveValue == cellCountToHome) {
        hS_h(&moves[++moveIndex], piece, NULL, FALSE);
      }
    };
  }

  // === SECTION: calculate moves for blocks ===
  for (short blockIndex = 0; blockIndex < MAX_BLOCKS_PER_COLOR; blockIndex++) {
    if (blocks[currentTurnColor][blockIndex].pieceCount == 0) {
      continue;
    }

    Block *block = &blocks[currentTurnColor][blockIndex];
    short locationType = block->pieces[0]->locationType;
    short moveValue = diceRolledValue;
    short moveLeft = 0;

    // check for mystery cell effects
    switch (block->mysteryEffect) {
      case 1:
        continue;
      case 2:
        moveValue *= 2;
        break;
      case 3:
        moveValue /= 2;
        break;
    }

    short moveBlockValue = moveValue / block->pieceCount;

    if (moveValue == 0 || moveBlockValue == 0) {
      continue;
    }

    Check check = nextCellsInspection(NULL, block, moveValue);

    if (locationType == STANDARD_AREA) {

      if (isEligibleForHomeStraight(NULL, block) && check.doesPieceOrBlockPassApproach == TRUE) {
        // standardArea --> home
        if (moveBlockValue - check.distanceToApproach == 6) {
          sA_H(&moves[++moveIndex], NULL, block);
        } else if (moveBlockValue - check.distanceToApproach > 6) {
          moveLeft = 5;
        } else {
          moveLeft = moveBlockValue - check.distanceToApproach;
        }

        // standardArea --> homeStraight
        if (homeStraights[currentTurnColor][moveLeft - 1].pieceCount == 0) {
          // empty cell
          sA_hS_emptyCell(&moves[++moveIndex], NULL, block, moveLeft - 1);
        } else if (homeStraights[currentTurnColor][moveLeft - 1].pieceCount == 1) {
          // same color piece, expand block
          sA_hS_playerPiece(&moves[++moveIndex], NULL, block, moveLeft - 1);
        } else if (homeStraights[currentTurnColor][moveLeft - 1].pieceCount > 1) {
          // same color block, expand block
          sA_hS_playerBlock(&moves[++moveIndex], NULL, block, moveLeft - 1);
        }
      }
      // standardArea --> standardArea
      else {
        if (check.doesPieceOrBlockMeetBlock == TRUE) {
          if (check.canCaptureBlock == TRUE) {
            moveBlockValue = check.distanceToBlock;
          } else {
            moveBlockValue = check.distanceToBlock - 1 + 2 * block->movingDirection;
          }

          if (moveBlockValue == 0)
            continue;
        } else if (block->movingDirection == COUNTER_CLOCKWISE) {
          moveBlockValue *= -1;
        }

        short nextLocation = (block->pieces[0]->location + moveBlockValue + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;

        if (standardArea[nextLocation].pieceCount == 0) {
          // empty cell
          sA_sA_emptyCell(&moves[++moveIndex], NULL, block, nextLocation, &check);
        } else if (standardArea[nextLocation].piece != NULL) {
          if (standardArea[nextLocation].piece->color->colorNum == currentTurnColor) {
            // player piece, expand block
            sA_sA_playerPiece(&moves[++moveIndex], NULL, block, nextLocation, &check);
          } else {
            // enemy piece, capture piece
            sA_sA_enemyPiece(&moves[++moveIndex], NULL, block, nextLocation, &check);
          }
        } else if (standardArea[nextLocation].block != NULL) {
          if (standardArea[nextLocation].block->color->colorNum == currentTurnColor) {
            // player block, expand block
            sA_sA_playerBlock(&moves[++moveIndex], NULL, block, nextLocation, &check);
          } else {
            // enemy block, capture
            sA_sA_enemyBlock(&moves[++moveIndex], block, nextLocation, &check);
          }
        }
      }
    }

    // homeStraight --> home
    else if (locationType == HOME_STRAIGHT) {
      short cellCountToHome = 5 - block->pieces[0]->location;

      if (moveBlockValue == cellCountToHome) {
        hS_h(&moves[++moveIndex], NULL, block, TRUE);
      } else if (moveValue == cellCountToHome) {
        hS_h(&moves[++moveIndex], block->pieces[block->pieceCount - 1], NULL, TRUE);
      }
    }
  }
}

short moveSelectionBots() {
  long randNum = rand();
  return randNum % (moveIndex + 1);
}

void moveExecute(Move *move) {
  Move *m = move;
  Block *b = NULL;
  if (move == NULL) {
    short moveChoose = moveSelectionBots();
    m = &moves[moveChoose];
  }
  short moveValue;
  char movingDirection;

  // TODO: this is redundant calculation, try to feed the values from previous function call
  if (m->startStandardCell != NULL && m->endStandardCell != NULL && (m->piece != NULL || m->block != NULL)) {
    if (m->piece != NULL) {
      movingDirection = m->piece->movingDirection;
    } else if (m->block != NULL) {
      movingDirection = m->block->movingDirection;
    }
    if (movingDirection == CLOCKWISE) {
      moveValue =
          (m->endStandardCell->cellIndex - m->startStandardCell->cellIndex + MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
    } else {
      moveValue =
          -(m->endStandardCell->cellIndex - m->startStandardCell->cellIndex - MAX_STANDARD_CELLS) % MAX_STANDARD_CELLS;
    }
  }

  switch (m->moveType) {
    case 0: { // no move
      if (moveIndex == 0) {
        printf("%s has no valid moves. Skipping Turn\n", colors[currentTurnColor].colorName);
      } else {
        printf("%s chooses no moves.\n", colors[currentTurnColor].colorName);
      }
      break;
    }

      // === SECTION: piece related movement ===

    case 1: { // base => standardArea (empty cell)
      gameInProgress = TRUE;
      m->base->pieces[m->piece->pieceNum] = NULL;
      m->base->pieceCount--;
      m->endStandardCell->pieceCount++;
      m->endStandardCell->piece = m->piece;
      m->piece->movingDirection = coinToss();
      m->piece->location = m->piece->startingCell;
      m->piece->locationType = STANDARD_AREA;
      printf("%s player moves piece %s to the starting point.\n", m->piece->color->colorName, m->piece->name);
      printf("%s tosses %s and %s will move in %s direction\n", m->piece->color->colorName,
             coinTossNames[m->piece->movingDirection], m->piece->name, movingDirectionNames[m->piece->movingDirection]);
      printf("%s has %d/4 of pieces on the board and %d/4 pieces on the base.\n", m->piece->color->colorName,
             m->base->pieceCount, 4 - m->base->pieceCount);
      break;
    }
    case 2: { // base => standardArea (player piece, create block)
      createBlock(m->piece, m->playerPiece, m->endStandardCell, STANDARD_AREA);
      b = m->endStandardCell->block;

      m->piece->movingDirection = coinToss();
      m->base->pieceCount--;
      m->base->pieces[m->piece->pieceNum] = NULL;

      printf("%s player moves piece %s to the starting point\n", m->piece->color->colorName, m->piece->name);
      printf("%s landed on %s and created a block of %d.\n", m->piece->name, m->playerPiece->name, b->pieceCount);
      printf("The block has the longest distance from home and will move in the "
             "%s direction.\n",
             movingDirectionNames[b->movingDirection]);
      printf("%s tosses %s and %s will move in %s direction (out of block)\n", m->piece->color->colorName,
             coinTossNames[m->piece->movingDirection], m->piece->name, movingDirectionNames[m->piece->movingDirection]);
      printf("%s has %d/4 of pieces on the board and %d/4 pieces on the base.\n", m->piece->color->colorName,
             m->base->pieceCount, 4 - m->base->pieceCount);
      break;
    }
    case 3: { // base => standardArea (player block, expand block)
      expandBlockByPiece(m->piece, m->endStandardCell->block, m->endStandardCell, STANDARD_AREA);
      b = m->endStandardCell->block;

      m->piece->movingDirection = coinToss();
      m->base->pieces[m->piece->pieceNum] = NULL;
      m->base->pieceCount--;

      printf("%s player moves piece %s to the starting point.\n", m->piece->color->colorName, m->piece->name);
      printf("%s landed on a block of %d containing ", m->piece->name, b->pieceCount - 1);
      for (short piece = 0; piece < b->pieceCount - 1; piece++) {
        printf("%s,", b->pieces[piece]->name);
      }
      printf("\b and expands it.\n");
      printf("The block has longest distance from home and will move in the %s direction.\n",
             movingDirectionNames[b->movingDirection]);
      printf("%s tosses %s and %s will move in %s direction (out of block)\n", m->piece->color->colorName,
             coinTossNames[m->piece->movingDirection], m->piece->name, movingDirectionNames[m->piece->movingDirection]);
      printf("%s has %d/4 of pieces on the board and %d/4 pieces on the base.\n", m->piece->color->colorName,
             m->base->pieceCount, 4 - m->base->pieceCount);
      break;
    }
    case 4: { // base => standardArea (enemy piece, eliminate piece)
      m->base->pieces[m->piece->pieceNum] = NULL;
      m->base->pieceCount--;
      m->endStandardCell->piece = m->piece;
      rebasePiece(m->enemyPiece);
      m->piece->capturedPieceCount++;
      m->piece->location = m->piece->startingCell;
      m->piece->locationType = STANDARD_AREA;
      m->piece->movingDirection = coinToss();
      printf("%s player moves piece %s to the starting point.\n", m->piece->color->colorName, m->piece->name);
      printf("%s landed on %s's %s and captured it.\n", m->piece->name, m->enemyPiece->color->colorName,
             m->enemyPiece->name);
      printf("%s tosses %s and %s will move in %s direction\n", m->piece->color->colorName,
             coinTossNames[m->piece->movingDirection], m->piece->name, movingDirectionNames[m->piece->movingDirection]);
      printf("%s has %d/4 of pieces on the board and %d/4 pieces on the base.\n", m->piece->color->colorName,
             m->base->pieceCount, 4 - m->base->pieceCount);
      printf("The player gets another move since it's captured a piece/block.\n");
      printf("============================\n");
      diceRolledValue = diceRoll();
      moveRound();
      break;
    }
    case 5: { // standardArea => standardArea (empty cell)
      if (m->check.doesPieceOrBlockPassApproach == TRUE) {
        m->piece->approachPassCount++;
      }
      m->endStandardCell->piece = m->piece;
      m->endStandardCell->pieceCount++;
      m->piece->location = m->endStandardCell->cellIndex;
      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;
      blockedMessage(m);
      printf("%s moves %s from standard-area L%d to L%d by %d units in %s direction.\n", m->piece->color->colorName,
             m->piece->name, m->startStandardCell->cellIndex, m->endStandardCell->cellIndex, moveValue,
             movingDirectionNames[m->piece->movingDirection]);
      break;
    }
    case 6: { // standardArea => standardArea (player piece, create block)
      if (m->check.doesPieceOrBlockPassApproach == TRUE) {
        m->piece->approachPassCount++;
      }
      createBlock(m->piece, m->playerPiece, m->endStandardCell, STANDARD_AREA);
      b = m->endStandardCell->block;

      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;
      blockedMessage(m);
      printf("%s moves %s from standard-area L%d to L%d by %d units %s direction.\n", m->piece->color->colorName,
             m->piece->name, m->startStandardCell->cellIndex, m->endStandardCell->cellIndex, moveValue,
             movingDirectionNames[m->piece->movingDirection]);
      printf("%s landed on %s and created a block of %d.\n", m->piece->name, m->playerPiece->name, b->pieceCount);
      printf("The block has the longest distance from home and will move in the %s direction.\n",
             movingDirectionNames[b->movingDirection]);
      break;
    }
    case 7: { // standardArea => standardArea (player block, expand block)
      if (m->check.doesPieceOrBlockPassApproach == TRUE) {
        m->piece->approachPassCount++;
      }
      expandBlockByPiece(m->piece, m->endStandardCell->block, m->endStandardCell, STANDARD_AREA);
      b = m->endStandardCell->block;
      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;

      blockedMessage(m);
      printf("%s moves %s from standard-area L%d to L%d by %d units %s direction.\n", m->piece->color->colorName,
             m->piece->name, m->startStandardCell->cellIndex, m->endStandardCell->cellIndex, moveValue,
             movingDirectionNames[m->piece->movingDirection]);
      printf("%s landed on a block of %d containing ", m->piece->name, b->pieceCount - 1);
      for (short piece = 0; piece < b->pieceCount - 1; piece++) {
        printf("%s,", b->pieces[piece]->name);
      }
      printf("\b and expanded upon it.\n");
      printf("The block has longest distance from home and will move in the %s direction.\n",
             movingDirectionNames[b->movingDirection]);
      break;
    }
    case 8: { // standardArea => standardArea (enemy piece, eliminate piece)
      if (m->check.doesPieceOrBlockPassApproach == TRUE) {
        m->piece->approachPassCount++;
      }
      rebasePiece(m->enemyPiece);
      m->endStandardCell->piece = m->piece;
      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;
      m->piece->capturedPieceCount++;
      m->piece->location = m->endStandardCell->cellIndex;

      blockedMessage(m);
      printf("%s moves %s from standard-area L%d to L%d by %d units %s direction.\n", m->piece->color->colorName,
             m->piece->name, m->startStandardCell->cellIndex, m->endStandardCell->cellIndex, moveValue,
             movingDirectionNames[m->piece->movingDirection]);
      printf("%s landed on %s's %s and captured it.\n", m->piece->name, m->enemyPiece->color->colorName,
             m->enemyPiece->name);
      if (isBreakingDown == FALSE) {
        printf("The player gets another move since it's captured a piece/block.\n");
        printf("============================\n");
        diceRolledValue = diceRoll();
        moveRound();
      }
      break;
    }
    case 9: { // standardArea => homeStraight (empty cell)
      m->homeStraightCell->piece = m->piece;
      m->homeStraightCell->pieceCount++;
      m->piece->location = m->homeStraightCell->cellIndex;
      m->piece->locationType = HOME_STRAIGHT;
      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;
      printf("%s moves %s from standard-area L%d to home-straight L%d.\n", m->piece->color->colorName, m->piece->name,
             m->startStandardCell->cellIndex, m->homeStraightCell->cellIndex);
      break;
    }
    case 10: { // standardArea => homeStraight (player piece, create block)
      createBlock(m->piece, m->playerPiece, m->homeStraightCell, HOME_STRAIGHT);
      b = m->homeStraightCell->block;
      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;
      printf("%s moves %s from standard-area L%d to home-straight L%d.\n", m->piece->color->colorName, m->piece->name,
             m->startStandardCell->cellIndex, m->homeStraightCell->cellIndex);
      printf("%s landed on %s and created a block of %d.\n", m->piece->name, m->playerPiece->name, b->pieceCount);
      printf("The block has the longest distance from home and will move in the %s direction.\n",
             movingDirectionNames[b->movingDirection]);
      break;
    }
    case 11: { // standardArea => homeStraight (player block, expand block)
      expandBlockByPiece(m->piece, m->homeStraightCell->block, m->homeStraightCell, HOME_STRAIGHT);
      b = m->homeStraightCell->block;
      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;
      printf("%s moves %s from standard-area L%d to home-straight L%d.\n", m->piece->color->colorName, m->piece->name,
             m->startStandardCell->cellIndex, m->homeStraightCell->cellIndex);
      printf("%s landed on a block of %d containing ", m->piece->name, b->pieceCount - 1);
      for (short piece = 0; piece < b->pieceCount - 1; piece++) {
        printf("%s,", b->pieces[piece]->name);
      }
      printf("\b and expands it.\n");
      printf("The block has longest distance from home and will move in the %s direction.\n",
             movingDirectionNames[b->movingDirection]);
      break;
    }
    case 12: { // standardArea => home
      m->home->pieces[m->piece->pieceNum] = m->piece;
      m->home->pieceCount++;
      m->piece->location = 0;
      m->piece->locationType = HOME;
      m->startStandardCell->piece = NULL;
      m->startStandardCell->pieceCount--;
      printf("%s moved %s from standard-area L%d to home.\n", m->piece->color->colorName, m->piece->name,
             m->startStandardCell->cellIndex);
      printf("Now there are %d/4 pieces in home.\n", m->home->pieceCount);

      if (m->home->pieceCount == 4) {
        players[currentTurnColor].isWinner = TRUE;
        winners[winnerCount] = &players[currentTurnColor];
        winnerCount++;
        printf("%s wins!!!\n", colors[currentTurnColor].colorName);
      }
      break;
    }
    case 13: { // homeStraight => home
      if (m->isFromBlock == TRUE) {
        if (m->block->pieceCount == 2) {
          m->block->pieces[0]->blockNum = NULL;
          m->homeStraightCell->piece = m->block->pieces[0];
          m->homeStraightCell->block = NULL;
          resetBlock(m->block);
        } else if (m->block->pieceCount > 2) {
          m->block->pieceCount--;
          m->block->pieces[m->block->pieceCount] = NULL;
        }
        m->piece->blockNum = NULL;
      } else {
        m->homeStraightCell->piece = NULL;
      }
      m->homeStraightCell->pieceCount--;
      m->home->pieces[m->piece->pieceNum] = m->piece;
      m->home->pieceCount++;
      m->piece->location = 0;
      m->piece->locationType = HOME;
      if (m->isFromBlock == TRUE) {
        printf("%s detached %s from the block at home-straight L%d and moved it to home.\n", m->piece->color->colorName,
               m->piece->name, m->homeStraightCell->cellIndex);
      } else {
        printf("%s moved %s from homestraight to home.\n", m->piece->color->colorName, m->piece->name);
      }
      printf("Now there are %d/4 pieces in home.\n", m->home->pieceCount);

      if (m->home->pieceCount == 4) {
        players[currentTurnColor].isWinner = TRUE;
        winners[winnerCount] = &players[currentTurnColor];
        winnerCount++;
        printf("%s wins!!!\n", colors[currentTurnColor].colorName);
      }
      break;
    }

      // === SECTION: block related movement ===

    case 14: { // standardArea => standardArea (empty cell)
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        if (m->check.doesPieceOrBlockPassApproach == TRUE) {
          m->block->pieces[piece]->approachPassCount++;
        }
        m->block->pieces[piece]->location = m->endStandardCell->cellIndex;
      }
      m->endStandardCell->block = m->block;
      m->endStandardCell->pieceCount = m->block->pieceCount;
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;

      blockedMessage(m);
      printf("%s moved the block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to L%d by %d units %s direction.\n", m->startStandardCell->cellIndex,
             m->endStandardCell->cellIndex, moveValue, movingDirectionNames[m->block->movingDirection]);
      break;
    }
    case 15: { // standardArea => standardArea (player piece, expand block)
      char *targetCellPieceName = m->playerPiece->name;
      expandPieceByBlock(m->block, m->playerPiece, m->endStandardCell, STANDARD_AREA);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        if (m->check.doesPieceOrBlockPassApproach == TRUE) {
          m->block->pieces[piece]->approachPassCount++;
        }
      }
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;
      blockedMessage(m);
      printf("%s moved the block of %d containing ", m->block->color->colorName, m->block->pieceCount - 1);
      for (short piece = 0; piece < m->block->pieceCount - 1; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to L%d by %d units in %s direction.\n", m->startStandardCell->cellIndex,
             m->endStandardCell->cellIndex, moveValue, movingDirectionNames[movingDirection]);
      printf("the block landed on %s and the block was expanded.\n", targetCellPieceName);
      printf("the block has the longest distance from home and will move in the %s direction.\n",
             movingDirectionNames[m->block->movingDirection]);
      break;
    }
    case 16: { // standardArea => standardArea (player block, expand block)
      short oldPieceCount = m->block->pieceCount;
      expandBlockByBlock(m->block, m->endStandardCell->block, m->endStandardCell, STANDARD_AREA);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        if (m->check.doesPieceOrBlockPassApproach == TRUE) {
          m->block->pieces[piece]->approachPassCount++;
        }
      }
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;

      blockedMessage(m);
      printf("%s moved the block of %d containing ", m->block->color->colorName, oldPieceCount);
      for (short piece = oldPieceCount; piece < m->playerBlock->pieceCount; piece++) {
        printf("%s,", m->endStandardCell->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to L%d by %d units in %s direction.\n", m->startStandardCell->cellIndex,
             m->endStandardCell->cellIndex, moveValue, movingDirectionNames[movingDirection]);
      printf("The block landed on a %s color block containing ", m->playerBlock->color->colorName);
      for (short piece = 0; piece < oldPieceCount; piece++) {
        printf("%s,", m->playerBlock->pieces[piece]->name);
      }
      printf("\b and the block was expanded.\n");
      printf("The block has the longest distance from home and will move in the %s direction.\n",
             movingDirectionNames[m->block->movingDirection]);
      break;
    }
    case 17: { // standardArea => standardArea (enemy piece, eliminate piece)
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        if (m->check.doesPieceOrBlockPassApproach == TRUE) {
          m->block->pieces[piece]->approachPassCount++;
        }
        m->block->pieces[piece]->capturedPieceCount++;
        m->block->pieces[piece]->location = m->endStandardCell->cellIndex;
      }
      m->endStandardCell->block = m->block;
      m->endStandardCell->piece = NULL;
      m->endStandardCell->pieceCount = m->block->pieceCount;
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;
      rebasePiece(m->enemyPiece);

      blockedMessage(m);
      printf("%s moved the block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->endStandardCell->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to L%d by %d units in %s direction.\n", m->startStandardCell->cellIndex,
             m->endStandardCell->cellIndex, moveValue, movingDirectionNames[m->block->movingDirection]);
      printf("The block landed on %s's %s and captured it.\n", m->enemyPiece->color->colorName, m->enemyPiece->name);
      printf("The player gets another move since it's captured a piece/block.\n");
      printf("============================\n");
      diceRolledValue = diceRoll();
      moveRound();
      break;
    }
    case 18: { // standardArea => standardArea (enemy block, eliminate block)
      Block tempEnemyBlock = *(m->enemyBlock);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        if (m->check.doesPieceOrBlockPassApproach == TRUE) {
          m->block->pieces[piece]->approachPassCount++;
        }
        m->block->pieces[piece]->location = m->endStandardCell->cellIndex;
        m->block->pieces[piece]->capturedPieceCount++;
      }
      m->endStandardCell->block = m->block;
      m->endStandardCell->pieceCount = m->block->pieceCount;
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;
      for (short piece = 0; piece < m->enemyBlock->pieceCount; piece++) {
        rebasePiece(m->enemyBlock->pieces[piece]);
      }
      resetBlock(m->enemyBlock);

      blockedMessage(m);
      printf("%s moved the block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to L%d by %d units %s direction.\n", m->startStandardCell->cellIndex,
             m->endStandardCell->cellIndex, moveValue, movingDirectionNames[m->block->movingDirection]);
      printf("The block landed on %s's block of %d containing ", tempEnemyBlock.color->colorName,
             tempEnemyBlock.pieceCount);
      for (short piece = 0; piece < tempEnemyBlock.pieceCount; piece++) {
        printf("%s,", tempEnemyBlock.pieces[piece]->name);
      }
      printf("\b and captured it.\n");
      printf("The player gets another move since it's captured a piece/block.\n");
      printf("============================\n");
      diceRolledValue = diceRoll();
      moveRound();
      break;
    }
    case 19: { // standardArea => homeStraight (empty cell)
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        m->block->pieces[piece]->location = m->homeStraightCell->cellIndex;
        m->block->pieces[piece]->locationType = HOME_STRAIGHT;
      }
      m->homeStraightCell->block = m->block;
      m->homeStraightCell->pieceCount = m->block->pieceCount;
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;
      printf("%s moved the block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to home-straight L%d.\n", m->startStandardCell->cellIndex,
             m->homeStraightCell->cellIndex);
      break;
    }
    case 20: { // standardArea => homeStraight (player piece, expand block)
      expandPieceByBlock(m->block, m->playerPiece, m->homeStraightCell, HOME_STRAIGHT);
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;
      printf("%s moved the block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to home-straight L%d.\n", m->startStandardCell->cellIndex,
             m->homeStraightCell->cellIndex);
      printf("The block landed on %s and and the block was expanded.\n", m->playerPiece->name);
      printf("the block has the longest distance from home and will move in the %s direction\n",
             movingDirectionNames[m->block->movingDirection]);
      break;
    }
    case 21: { // standardArea => homeStraight (player block, expand block)
      short oldPieceCount = m->block->pieceCount;
      expandBlockByBlock(m->block, m->playerBlock, m->homeStraightCell, HOME_STRAIGHT);
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;
      printf("%s moved the block of %d containing ", m->block->color->colorName, oldPieceCount);
      for (short piece = oldPieceCount; piece < m->playerBlock->pieceCount; piece++) {
        printf("%s,", m->playerBlock->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to home-straight L%d.\n", m->startStandardCell->cellIndex,
             m->homeStraightCell->cellIndex);
      printf("The block landed on a %s color block containing ", m->playerBlock->color->colorName);
      for (short piece = 0; piece < oldPieceCount; piece++) {
        printf("%s,", m->playerBlock->pieces[piece]->name);
      }
      printf("\b and the block was expanded.");
      break;
    }
    case 22: { // standardArea => home
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        m->block->pieces[piece]->location = 0;
        m->block->pieces[piece]->locationType = HOME;
        m->home->pieces[m->block->pieces[piece]->color->colorNum] = m->block->pieces[piece];
      }
      m->home->pieceCount += m->block->pieceCount;
      m->startStandardCell->block = NULL;
      m->startStandardCell->pieceCount = 0;
      printf("%s moves a block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b from standard-area L%d to home.\n", m->startStandardCell->cellIndex);
      printf("Now there are %d/4 pieces in home.\n", m->home->pieceCount);
      resetBlock(m->block);

      if (m->home->pieceCount == 4) {
        players[currentTurnColor].isWinner = TRUE;
        winners[winnerCount] = &players[currentTurnColor];
        winnerCount++;
        printf("%s wins!!!\n", colors[currentTurnColor].colorName);
      }
      break;
    }
    case 23: { // homeStraight => home
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        m->block->pieces[piece]->location = 0;
        m->block->pieces[piece]->locationType = HOME;
        m->home->pieces[m->block->pieces[piece]->pieceNum] = m->block->pieces[piece];
      }
      m->home->pieceCount += m->block->pieceCount;
      m->homeStraightCell->block = NULL;
      m->homeStraightCell->pieceCount = 0;
      printf("%s moves a block of %d containing ", m->block->color->colorName, m->block->pieceCount);
      for (short piece = 0; piece < m->block->pieceCount; piece++) {
        printf("%s,", m->block->pieces[piece]->name);
      }
      printf("\b from home-straight L%d to home.\n", m->homeStraightCell->cellIndex);
      printf("Now there are %d/4 pieces in home.\n", m->home->pieceCount);
      resetBlock(m->block);

      if (m->home->pieceCount == 4) {
        players[currentTurnColor].isWinner = TRUE;
        winners[winnerCount] = &players[currentTurnColor];
        winnerCount++;
        printf("%s wins!!!\n", colors[currentTurnColor].colorName);
      }
      break;
    }
    default:
      break;
  }
}

void moveRound() {
  calculateMoves();
  moveExecute(NULL);
  executeMysteryEffect();
}

void displayRound() {
  printf("============================\n");
  printf("%s player now has %d/4 on pieces on the board and %d/4 pieces on the base.\n",
         colors[currentTurnColor].colorName, 4 - bases[currentTurnColor].pieceCount,
         bases[currentTurnColor].pieceCount);
  printf("============================\n");
  printf("Count: %ld\n", count);
  printf("============================\n");
  //printf("Seed: %ld\n", (long int)seed);
  //printf("============================\n");
  printf("Location of pieces: %s\n", colors[currentTurnColor].colorName);
  printf("============================\n");
  for (short piece = 0; piece < PIECE_TOTAL_PER_COLOR; piece++) {
    if (pieces[currentTurnColor][piece].locationType == BASE) {
      printf("%s -> Base\n", pieces[currentTurnColor][piece].name);
    } else if (pieces[currentTurnColor][piece].locationType == STANDARD_AREA) {
      printf("%s -> L%d (SA)\n", pieces[currentTurnColor][piece].name,
             pieces[currentTurnColor][piece].location);
    } else if (pieces[currentTurnColor][piece].locationType == HOME_STRAIGHT) {
      printf("%s -> L%d (HS)\n", pieces[currentTurnColor][piece].name,
             pieces[currentTurnColor][piece].location);
    } else {
      printf("%s -> Home\n", pieces[currentTurnColor][piece].name);
    }
  }
  printf("============================\n");
  if (mysteryCell.cell != NULL) {
    printf("The mystery cell is at L%d.\n", mysteryCell.cell->cellIndex);
    // TODO: add "The mystery cell is at L%d for the next N rounds"
  } else {
    printf("The mystery cell has not been generated yet.\n");
  }
  printf("============================\n");
}

void handleMovesRolls() {
  short consecutiveSixRollCount = 0;
  displayRound();
  generateMysteryCell();

  while (consecutiveSixRollCount < 3) {
    diceRolledValue = diceRoll();

    if (diceRolledValue == DIE_ROLL_MAX) {
      consecutiveSixRollCount++;

      if (consecutiveSixRollCount == 3) {
        if (blocks[currentTurnColor][0].pieceCount || blocks[currentTurnColor][1].pieceCount) {
          Block *block = NULL;
          for (short blockIndex = 0; blockIndex < MAX_BLOCKS_PER_COLOR; blockIndex++) {
            if (blocks[currentTurnColor][blockIndex].pieceCount > 1) {
              block = &blocks[currentTurnColor][blockIndex];
              break;
            }
          }
          printf("Six is rolled for a third consecutive time, a block will be "
                 "broken.\n");
          if (block != NULL) {
            isBreakingDown = TRUE;
            breakBlock(block, block->pieceCount);
            isBreakingDown = FALSE;
          }
        } else {
          printf("Six is rolled for a third consecutive time, roll is ignored\n");
        }

        break;
      } else {
        moveRound();
        printf("Six is rolled for %d consecutive time(s), player will get another roll\n", consecutiveSixRollCount);
        printf("============================\n");
      }
    } else {
      moveRound();
      break;
    }
  }

  printf("\n");
}

void displayWinner() {
  printf("============================\n");
  printf("%s wins at 1st place\n", winners[0]->playerName);
  printf("%s wins at 2nd place\n", winners[1]->playerName);
  printf("%s wins at 3rd place\n", winners[2]->playerName);
  printf("============================\n");
}
