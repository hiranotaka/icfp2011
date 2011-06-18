// Usage: ./ltg.<architecture> match ./ltg_help_attack ./ltg_do_nothing
#include <cassert>
#include <iostream>
#include <string>

extern "C" {
#include "../sim/sim.h"
#include "../sim/types.h"
}

struct game* G;

using namespace std;

void Opp() {
  int application_order;
  cin >> application_order;
  if (application_order == 1) {
    string card_name;
    int slot_number;
    cin >> card_name >> slot_number;
    struct value* v = find_card_value(card_name.c_str());
    play_left(v, slot_number, G);
  } else {
    int slot_number;
    string card_name;
    cin >> slot_number >> card_name;
    struct value* v = find_card_value(card_name.c_str());
    play_right(slot_number, v, G);
  }
  switch_turn(G);
}

const char* card_names[] = {
  "I", "zero", "succ", "dbl", "get", "put", "S", "K", "inc", "dec", "attack",
  "help", "copy", "revive", "zombie"
};

enum Card {
  I, ZERO, SUCC, DBL, GET, PUT, S, K, INC, DEC, ATTACK, HELP, COPY, REVIVE,
  ZOMBIE
};

#define arraysize(a) (sizeof(a)/sizeof((a)[0]))

template<typename T, typename U>
void __(T lhs, U rhs, struct game* g);

template<>
void __(Card c, int i, struct game* g) {
  cout << 1 << endl;
  cout << card_names[c] << endl;
  cout << i << endl;
  struct value* v = find_card_value(card_names[c]);
  play_left(v, i, g);
}

template<>
void __(int i, Card c, struct game* g) {
  cout << 2 << endl;
  cout << i << endl;
  cout << card_names[c] << endl;
  struct value* v = find_card_value(card_names[c]);
  play_right(i, v, g);
}

template<typename T, typename U>
void _(T lhs, U rhs) {
  __(lhs, rhs, G);
  switch_turn(G);
  Opp();
}

// Assumes that slot i has value zero.
void ToN(int i, int n) {
  if (n != 0) {
    ToN(i, n / 2);
    _(DBL, i);
    if (n & 1) {
      _(SUCC, i);
    }
  }
}

// Assumes that slot i has valeu I.
void IToN(int i, int n) {
  _(i, ZERO);
  ToN(i, n);
}

void Wrap(int i, Card c) {
            // i: F
  _(K, i);  // i: K(F)
  _(S, i);  // i: S(K(F))
  _(i, c);  // i: S(K(F))(c)
}

void WrapSuccN(int i, int n) {
  if (n == 0) return;
  WrapSuccN(i, n / 2);
  Wrap(i, DBL);
  if (n & 1) Wrap(i, SUCC);
}

// Calls a function at slot i with a value at slot j.
void CallWithSlot(int i, int j) {
  // i: F
  Wrap(i, GET);  // i: S(K(F))(get)
  //WrapSuccN(i, j);
  for (int k = 0; k < j; ++k) {
    Wrap(i, SUCC);  // i: S(K(S(K(F))(get)))(succ)
  }
  _(i, ZERO);  // i: F(get(succ(succ(...(zero)...))))
}

// Calls a function at slot i with a value j
void CallWithValue(int i, int j) {
  // i: F
  for (int k = 0; k < j; ++k) {
    Wrap(i, SUCC);  // i: S(K(F))(succ)
  }
  _(i, ZERO);  // i: F(succ(succ(...(zero)...)))
}

void DoWork() {
  // Assumes 0: I
  // _(PUT, 0);
  IToN(0, 3);  // 0: 3
  _(GET, 0);  // 0: 8192

  // Repeat help(0)(1)(8192) and help(1)(0)(8192) to increase life.
  for (int i = 0; i < 68; ++i) {
    // help(0)(1)(8192)
    // Assumes 0: 8192
    IToN(1, 4);  // 1: 4
    _(GET, 1);  // 1: help(0)(1)
    CallWithSlot(1, 0);  // help(0)(1)(8192) -> I

    // help(1)(0)(8192)
    // Assumes 0: 8192
    IToN(1, 5);  // 1: 5
    _(GET, 1);  // 1: help(1)(0)
    CallWithSlot(1, 0);  // help(1)(0)(8192) -> I
  }

  _(PUT, 0);  // 0: I

  static int o = 0;

  // Using life of slot 0 and 1 kill opponent's slots repeatedly.

  // attack(0)(o)(11112)
  for (int i = 0; i < 5; ++i) {
    _(1, ZERO);  // 1: 0
    _(ATTACK, 1);  // 1: attack(0)
    _(0, ZERO);  // 0: 0
    ToN(0, o++);  // 0: o
    CallWithSlot(1, 0);  // 1: attack(0)(o)
    CallWithSlot(1, 2);  // 1: attack(0)(o)(11112) -> I
    _(PUT, 0);  // 0: I
  }

  // attack(1)(o)(11112)
  for (int i = 0; i < 5; ++i) {
    _(1, ZERO);  // 1: 0
    _(SUCC, 1);  // 1: 1
    _(ATTACK, 1);  // 1: attack(1)
    _(0, ZERO);  // 0: 0
    ToN(0, o++);  // 0: o
    CallWithSlot(1, 0);  // 1: attack(1)(o)
    CallWithSlot(1, 2);  // 1: attack(1)(o)(11112) -> I
    _(PUT, 0);  // 0: I
  }
}

void Work() {
  // Precompute functions/values.

  // 2: 11112
  IToN(2, 11112);  // 2: 11112

  // 3: 8192
  IToN(3, 8192);  // 3: 8192

  // 4: help(0)(1)
  _(4, HELP);  // 4: help
  CallWithValue(4, 0);  // 4: help(0)
  CallWithValue(4, 1);  // 4: help(0)(1)

  // 5: help(1)(0)
  _(5, HELP);  // 5: help
  CallWithValue(5, 1);  // 5: help(1)
  CallWithValue(5, 0);  // 5: help(1)(0)

  while (1) {
    DoWork();
  }
}



int main(int argc, char** argv) {
  assert(argc == 2);
  G = create_game();
  if (argv[1] == string("1")) {
    Opp();
  }
  Work();
  return 0;
}
