#ifndef UTIL_H
#define UTIL_H

#include <iostream>

using std::cout;
using std::cin;
using std::endl;

extern "C" {
#include "../../sim/sim.h"
#include "../../sim/types.h"
}


enum Card {
  I, ZERO, SUCC, DBL, GET, PUT, S, K, INC, DEC, ATTACK, HELP, COPY, REVIVE,
  ZOMBIE
};

const char* GetCardName(Card card);
void Opp();
void ToN(int i, int n);
void IToN(int i, int n);
void Wrap(int i, Card c);
void WrapSuccN(int i, int n);
void CallWithSlot(int i, int j);
void CallWithValue(int i, int j);
void MaybePut(int i);

void _(Card c, int i);
void _(int i, Card c);

#endif
