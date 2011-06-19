#include "global.h"
#include "sim_util.h"
#include "util.h"

#include <cassert>
#include <iostream>
#include <string>
#include <queue>

using namespace std;

struct game* G;
bool DEBUG;
int MY_PLAYER;
int OPP_PLAYER;

// return true when revived
bool ReviveIfDeath(int slot) {
  if (GetMyVitality(slot, G) > 0) {
    return false;
  }

  int alive = 0;
  for (int i = 10; i < 256; ++i) {
    if (GetMyVitality(i, G) > 1000) {
      alive = i;
      break;
    }
  }

  _(PUT, alive); // alive : I
  IToN(alive, slot); // alive : slot
  _(REVIVE, alive); // revive slot

  return true;
}

void PrintSlots() {
  for (int i = 0; i < 256; ++i) {
    int v = GetMyVitality(i, G);
    if (v <= 0) {
      cerr << "*";
    } else if (v == 1) {
      cerr << ",";
    } else {
      cerr << ".";
    }
  }
  cerr << endl;
}

void TryRevive() {
  int alive = 0;
  for (int i = 0; i < 256; ++i) {
    if (GetMyVitality(i, G) > 1000) {
      alive = i;
      break;
    }
  }
  int revived = 0;
  for (int i = 0; i < 256; ++i) {
    if (GetMyVitality(i, G) <= 0) {
      if (GetMyVitality(alive, G) <= 0)
	break;

      int value = GetMyValue(alive, G);
      if (value == i - 3) {
	_(SUCC, alive);
	_(SUCC, alive);
	_(SUCC, alive);
      } else if (value == i - 2) {
	_(SUCC, alive);
	_(SUCC, alive);
      } else if (value == i - 1) {
	_(SUCC, alive);
      } else {
	MaybePut(alive);
	IToN(alive, i); // alive : i
      }
      _(REVIVE, alive); // revive i
      cerr << "revive " << i << " alive " << alive << endl;
      PrintSlot(MY_PLAYER, i);
      PrintSlot(MY_PLAYER, alive);

      PrintSlots();

      revived++;
    }
  }
  if (revived == 0){
    _(PUT, 0);
  }
}


void Work() {
  while (1) {
    TryRevive();
  }
}


int main(int argc, char** argv) {
  assert(argc == 2);
  DEBUG = false;
  G = create_game();
  MY_PLAYER = 0;
  OPP_PLAYER = 1;
  if (argv[1] == string("1")) {
    MY_PLAYER = 1;
    OPP_PLAYER = 0;
    Opp();
  }
  Work();
  destroy_game(G);
  return 0;
}
