#include "gamelogic.h"
#include "types.h"
#include <stdlib.h>
// #include <time.h>

int main() {

  seed = time(NULL);
  srand(seed);

  initialization();
  firstToRollPlayerSelection();

  while (winnerCount < 3) {
    handleMovesRolls();
    count++;
    if (gameInProgress) {
      colorCycleCount++;
    }
    decayMysteryCellEffect();
    cycleToNextPlayer();
  }

  displayWinner();

  return 0;
}
