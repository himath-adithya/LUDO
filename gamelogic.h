#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "types.h"

// ===== SECTION: function declarations =====

void initialization();
void briefEffect(void *, short);
void checkBriefEffect(short);
short diceRoll();
short coinToss();
void cycleToNextPlayer();
void firstToRollPlayerSelection();

void resetMoves();
void rebasePiece(Piece*);
void resetBlock(Block*);

Check nextCellsInspection(Piece*, Block*, short);

void createBlock(Piece*, Piece*, void*, short);
void expandBlockByPiece(Piece*, Block*, void*, short);
void expandBlockByBlock(Block*, Block*, void*, short);

Move blockBreakMoveCalculate(Piece* piece, short moveValue);
short breakBlockExecute(Block* block, short blockSize, short *moveValues);
void breakBlock(Block*, short);

void blockedMessage(Move*);

void generateMysteryCell();
void *mysteryTeleport(void *, short, short, short *);
void executeMysteryEffect();
void decayMysteryCellEffect();

char longestMovingPath(short, short);
short isEligibleForHomeStraight(Piece*, Block*);

void b_sA_emptyCell(Move*, Piece*); // 1
void b_sA_playerPiece(Move*, Piece*); // 2
void b_sA_playerBlock(Move*, Piece*); // 3
void b_sA_enemyPiece(Move*, Piece*); // 4

void sA_sA_emptyCell(Move*, Piece*, Block*, short, Check*); // 5, 14
void sA_sA_playerPiece(Move*, Piece*, Block*, short, Check*); // 6, 15
void sA_sA_playerBlock(Move*, Piece*, Block*, short, Check*); // 7, 16
void sA_sA_enemyPiece(Move*, Piece*, Block*, short, Check*); // 8, 17
void sA_sA_enemyBlock(Move*, Block*, short, Check*); // 17

void sA_hS_emptyCell(Move*, Piece*, Block*, short); // 9, 19
void sA_hS_playerPiece(Move*, Piece*, Block*, short); // 10, 20
void sA_hS_playerBlock(Move*, Piece*, Block*, short); // 11, 21

void sA_H(Move*, Piece*, Block*); // 12, 22
void hS_h(Move*, Piece*, Block*, char); // 13, 23

void calculateMoves();
short moveSelectionBots();
void moveExecute(Move *m);
void moveRound();
void handleMovesRolls();
void displayWinner();

#endif // GAMELOGIC_H
