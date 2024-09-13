#ifndef TYPES_H
#define TYPES_H

#include <time.h>

// ===== SECTION: constants =====

#define COLOR_TOTAL 4
#define PLAYER_TOTAL 4
#define MOVES_TOTAL 8
#define PIECE_TOTAL_PER_COLOR 4
#define PIECE_TOTAL_PER_PLAYER 4
#define MYSTERY_EFFECT_TOTAL 6
#define MAX_STANDARD_CELLS 52
#define MAX_HOME_STRAIGHT_CELLS 5
#define MAX_PIECES_PER_CELL 4
#define MAX_PIECES_PER_BLOCK 4
#define MAX_BLOCKS_PER_COLOR 2
#define MAX_MYSTERY_EFFECT_DURATION 16

#define RED 0
#define GREEN 1
#define YELLOW 2
#define BLUE 3

#define BASE 0
#define STANDARD_AREA 1
#define HOME_STRAIGHT 2
#define HOME 3

#define HEADS 0
#define TAILS 1

#define DIE_ROLL_MAX 6
#define COIN_TOSS_MAX 2

#define CLOCKWISE 0
#define COUNTER_CLOCKWISE 1

#define NO_EFFECT 0
#define IMMOVABLE 1
#define AURA_ENERGIZED 2
#define AURA_SICKNESS 3

#define BHAWANA 9
#define KOTUWA 27
#define PITA_KOTUWA 46

#define TRUE 1
#define FALSE 0

#define MOVEMENT_TOTAL 6

#define PIECE 0
#define BLOCK 1

// ===== SECTION: structs =====

typedef struct {
  char *colorName;
  short colorNum;
} Color;

typedef struct {
  Color *color;
  short pieceNum;
  short locationType;
  short location;
  short startingCell;
  short approach;
  short capturedPieceCount;
  short approachPassCount;
  short *blockNum;
  char name[3];
  char movingDirection;
  char mysteryEffect; // 0 - None, 1 - Immovable, 2 - Aura Energized, 3 - Aura Sickness
  char mysteryEffectDuration;
  short whileInBriefConsecutiveThrees;
} Piece;

typedef struct {
  Color *color;
  Piece *pieces[MAX_PIECES_PER_BLOCK];
  short blockNum;
  short pieceCount; // if >0 then block is active
  char movingDirection; // 0:clockwise; 1:counterClockwise
  char mysteryEffect; // 0 - None, 1 - Immovable 2 - Aura Energized, 3 - Aura Sickness
  char mysteryEffectDuration; // max 4
  short whileInBriefConsecutiveThrees;
} Block;

typedef struct {
  Piece *pieces[PIECE_TOTAL_PER_PLAYER];
  Color *color;
  short pieceCount;
} Base;

typedef struct {
  Piece *piece;
  Block *block;
  short cellIndex;
  short pieceCount;
} StandardCell;

typedef struct {
  Color *color;
  Piece *piece;
  Block *block;
  short cellIndex;
  short pieceCount;
} HomeStraightCell;

typedef struct {
  Color *color;
  Piece *pieces[MAX_PIECES_PER_CELL];
  short pieceCount;
} Home;

typedef struct {
  StandardCell *cell;
} MysteryCell;

typedef struct {
// TODO: remove player struct
  Color *color;
  Piece *pieces[PIECE_TOTAL_PER_PLAYER];
  char playerName[15];
  char isWinner;
} Player;

typedef struct {
  char doesPieceOrBlockMeetBlock;
  char canCaptureBlock;
  short distanceToBlock;
  char doesPieceOrBlockPassApproach;
  short distanceToApproach;
  short initialMoveValue;
} Check;

typedef struct {
  Piece *piece;
  Block *block;
  Base *base;
  StandardCell *startStandardCell;
  StandardCell *endStandardCell;
  HomeStraightCell *homeStraightCell;
  Home *home;
  // properties of pieces in the endStandardCell
  Piece *playerPiece; 
  Block *playerBlock;
  Piece *enemyPiece;
  Block *enemyBlock;
  //
  Check check;
  char moveType;
  char isFromBlock;
} Move;
  // TODO: MOVE can be revised to use less members
  // void* element;
  // void *location;
  // char locationType;
  // void* destElement;
  // void *destLocation;
  // char destLocationType;

extern Color colors[COLOR_TOTAL];
extern Piece pieces[COLOR_TOTAL][PIECE_TOTAL_PER_COLOR];
extern Base bases[COLOR_TOTAL];
extern StandardCell standardArea[MAX_STANDARD_CELLS];
extern HomeStraightCell homeStraights[COLOR_TOTAL][MAX_HOME_STRAIGHT_CELLS];
extern Home homes[COLOR_TOTAL];
extern MysteryCell mysteryCell;
extern Player players[PLAYER_TOTAL];
extern Move moves[MOVES_TOTAL];
extern Block blocks[COLOR_TOTAL][MAX_BLOCKS_PER_COLOR];
extern Player *winners[];

// ===== SECTION: other variables =====

extern long count;
extern time_t seed;
extern long colorCycleCount;
extern short diceRolledValue;
extern short currentTurnColor;
extern short winnerCount;
extern short moveIndex;
extern char *coinTossNames[];
extern char *movingDirectionNames[];
extern char gameInProgress;
extern char isBreakingDown;

#endif // TYPES_H
