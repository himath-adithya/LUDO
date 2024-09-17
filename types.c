#include "types.h"
#include <stddef.h>
#include <time.h>

// ===== SECTION: struct =====

Color colors[COLOR_TOTAL];
Piece pieces[COLOR_TOTAL][PIECE_TOTAL_PER_COLOR];
Base bases[COLOR_TOTAL];
StandardCell standardArea[MAX_STANDARD_CELLS];
HomeStraightCell homeStraights[COLOR_TOTAL][MAX_HOME_STRAIGHT_CELLS];
Home homes[COLOR_TOTAL];
MysteryCell mysteryCell;
Player players[PLAYER_TOTAL];
Move moves[MOVES_TOTAL];
Block blocks[COLOR_TOTAL][MAX_BLOCKS_PER_COLOR];
Player *winners[] = {NULL, NULL, NULL};

// ===== SECTION: other variables =====

long count = 1;
time_t seed = 0;
long colorCycleCount = 0;
short diceRolledValue;
short currentTurnColor = 0; /* red: 0 ; green: 1 ; yellow: 2 ; blue: 3 */
short winnerCount = 0;
short moveIndex = 0;
const char *coinTossNames[] = {"heads", "tails"};
const char *movingDirectionNames[] = {"clockwise", "counter-clockwise"};
char gameInProgress = FALSE;
char isBreakingDown = FALSE;
