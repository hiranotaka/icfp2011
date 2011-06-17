// Usage: ./ltg.<architecture> match ./ltg_help_attack ./ltg_do_nothing
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

void Opp() {
  int application_order;
  cin >> application_order;
  if (application_order == 1) {
    string card_name;
    int slot_number;
    cin >> card_name >> slot_number;
  } else {
    int slot_number;
    string card_name;
    cin >> slot_number >> card_name;
  }
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
void __(T lhs, U rhs);

template<>
void __(Card c, int i) {
  cout << 1 << endl;
  cout << card_names[static_cast<int>(c)] << endl;
  cout << i << endl;
}

template<>
void __(int i, Card c) {
  cout << 2 << endl;
  cout << i << endl;
  cout << card_names[static_cast<int>(c)] << endl;
}

template<typename T, typename U>
void _(T lhs, U rhs) {
  __(lhs, rhs);
  Opp();
}

void ApplyAtZero(int i) {
               // i: F
  _(K, i);     // i: K(F)
  _(S, i);     // i: S(K(F))
  _(i, GET);   // i: S(K(F))(get)
  _(i, ZERO);  // i: S(K(F))(get)(zero) -> F(get zero)
}

void DoubleN(int i, int n) {
  for (int j = 0; j < n; ++j) {
    _(DBL, i);
  }
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

void Succ(int i, int n) {
  for (int j = 0; j < n; ++j) {
    _(SUCC, i);
  }
}

void DoWork() {
  // Assumes 0: I
  // _(PUT, 0);
  _(0, ZERO);  // 0: 0
  ToN(0, 3);  // 0: 3
  _(GET, 0);  // 0: 8192

  // Repeat help(0)(1)(8192) and help(1)(0)(8192) to increase life.
  for (int i = 0; i < 68; ++i) {
    // help(0)(1)(8192)
    // Assumes 0: 8192
    _(1, ZERO);  // 1: 0
    ToN(1, 4);  // 1: 4
    _(GET, 1);  // 1: help(0)(1)
    ApplyAtZero(1);  // help(0)(1)(8192) -> I

    // help(1)(0)(8192)
    // Assumes 0: 8192
    _(1, ZERO);  // 1: 0
    ToN(1, 5);  // 1: 5
    _(GET, 1);  // 1: help(1)(0)
    ApplyAtZero(1);  // help(1)(0)(8192) -> I
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
    ApplyAtZero(1);  // 1: attack(0)(o)
    _(PUT, 0);  // 0: I
    _(0, ZERO);  // 0: 0
    ToN(0, 2);  // 0: 2
    _(GET, 0);  // 0: 11112
    ApplyAtZero(1);  // 1: attack(0)(o)(11112) -> I
    _(PUT, 0);  // 0: I
  }

  // attack(1)(o)(11112)
  for (int i = 0; i < 5; ++i) {
    _(1, ZERO);  // 1: 0
    _(SUCC, 1);  // 1: 1
    _(ATTACK, 1);  // 1: attack(1)
    _(0, ZERO);  // 0: 0
    ToN(0, o++);  // 0: o
    ApplyAtZero(1);  // 1: attack(1)(o)
    _(PUT, 0);  // 0: I
    _(0, ZERO);  // 0: 0
    ToN(0, 2);  // 0: 2
    _(GET, 0);  // 0: 11112
    ApplyAtZero(1);  // 1: attack(1)(o)(11112) -> I
    _(PUT, 0);  // 0: I
  }
}

void Work() {
  // Precompute functions/values.

  // 2: 11112
  _(2, ZERO);  // 2: 0
  ToN(2, 11112);  // 2: 11112

  // 3: 8192
  _(3, ZERO);  // 3: 0
  ToN(3, 8192);  // 3: 8192

  // 4: help(0)(1)
  _(0, ZERO);  // 0: 0
  _(SUCC, 0);  // 0: 1
  _(4, HELP);  // 4: help
  _(4, ZERO);  // 4: help(0)
  ApplyAtZero(4);  // 4: help(0)(1)
  _(PUT, 0);  // 0: I

  // 5: help(1)(0)
  _(0, ZERO);  // 0: 0
  _(5, ZERO);  // 5: 0
  _(SUCC, 5);  // 5: 1
  _(HELP, 5);  // 5: help(1)
  ApplyAtZero(5);  // 5: help(1)(0)
  _(PUT, 0);  // 0: I

  while (1) {
    DoWork();
  }
}

int main(int argc, char** argv) {
  assert(argc == 2);
  if (argv[1] == string("1")) {
    Opp();
  }
  Work();
  return 0;
}
