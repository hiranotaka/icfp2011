// Usage: ./ltg.<architecture> match ./ltg_help_attack ./ltg_do_nothing
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

extern "C" {
#include "../sim/sim.h"
#include "../sim/types.h"
}

struct game* G;
int MY_PLAYER;

using namespace std;

// Use MY_PLAYER or MY_PLAYER^1 as a second argument.
int GetVitality(int i, int player, struct game* g) {
  return g->users[player].slots[i].vitality;
}

void Opp() {
  int application_order;
  cin >> application_order;
  if (application_order == 1) {
    string card_name;
    int slot_number;
    cin >> card_name >> slot_number;
    struct value* v = find_card_value(card_name.c_str());
    apply_cs(v, slot_number, G);
  } else {
    int slot_number;
    string card_name;
    cin >> slot_number >> card_name;
    struct value* v = find_card_value(card_name.c_str());
    apply_sc(slot_number, v, G);
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
void __(T lhs, U rhs, struct game* g);

template<>
void __(Card c, int i, struct game* g) {
  cout << 1 << endl;
  cout << card_names[c] << endl;
  cout << i << endl;
  struct value* v = find_card_value(card_names[c]);
  apply_cs(v, i, g);
}

template<>
void __(int i, Card c, struct game* g) {
  cout << 2 << endl;
  cout << i << endl;
  cout << card_names[c] << endl;
  struct value* v = find_card_value(card_names[c]);
  apply_sc(i, v, g);
}

template<typename T, typename U>
void _(T lhs, U rhs) {
  __(lhs, rhs, G);
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
  if (n & 1) Wrap(i, SUCC);
  if (n / 2) Wrap(i, DBL), WrapSuccN(i, n / 2);
}

// Calls a function at slot i with a value at slot j.
void CallWithSlot(int i, int j) {
  // i: F
  Wrap(i, GET);  // i: S(K(F))(get)
  WrapSuccN(i, j);
  _(i, ZERO);  // i: F(get(succ(succ(...(zero)...))))
}

// Calls a function at slot i with a value j
void CallWithValue(int i, int j) {
  // i: F
  WrapSuccN(i, j);
  _(i, ZERO);  // i: F(succ(succ(...(zero)...)))
}

void DoWork() {
  int mv = -1;  // Maximum vitality.
  int mvi = -1;  // Slot number of maximum vitality.
  for (int i = 0; i < 256; ++i) {
    int v = GetVitality(i, MY_PLAYER, G);
    if (mv < v) {
      mv = v;
      mvi = i;
    }
  }

  assert(mv > 0 && mvi >= 0);

  while (mv < 65535) {
    _(PUT, mvi);  // mvi: I
    IToN(mvi, mvi);  // mvi: mvi
    _(HELP, mvi);  // mvi: help(mvi)

    int sv = -1;  // Second largest vitality.
    int svi = -1;  // Slot number of second largest vitality.
    for (int i = 0; i < 256; ++i) {
      if (i == mvi) continue;
      int v = GetVitality(i, MY_PLAYER, G);
      if (sv < v) {
        sv = v;
        svi = i;
      }
    }

    assert(sv > 0 && svi >= 0);

    CallWithValue(mvi, svi);  // mvi: help(mvi)(svi)
    int move = min(mv - 100, 65535 - sv);  // We leave at least 100 life.
    CallWithValue(mvi, move);  // mvi: I
    mv = GetVitality(svi, MY_PLAYER, G);
    mvi = svi;
  }

  while (mv > 10000) {
    int av = -1;  // Maximum vitality in opponent.
    int avi = -1;  // Number of slot which have maximum vitality in opponent.
    for (int i = 0; i < 256; ++i) {
      int v = GetVitality(i, 1 - MY_PLAYER, G);
      if (v > 0 && av < v) {
        av = v;
        avi = i;
      }
    }

    assert(av > 0 && avi >= 0);

    // av could be increased so we add 100.
    int attack = min(av * 10 / 9 + 100, mv - 10000);

    // attack(mvi)(255 - avi)(av)
    _(PUT, mvi);  // mvi: I
    IToN(mvi, mvi);  // mvi: mvi
    _(ATTACK, mvi);  // mvi: attack(mvi)
    CallWithValue(mvi, 255 - avi);  // mvi: attack(mvi)(255 - avi)

    // We want to simply do CallWithValue(mvi, attack) here, but this takes time
    // and before we become to be able to attack, opponent may move his life to
    // other slot using help. Thus instead of that we take fast approach.
    //CallWithValue(mvi, attack);  // mvi: attack(mvi)(255 - avi)(attack)

    // Fast approach.
    for (int i = 0; i < 256; ++i) {
      if (i == mvi) continue;
      if (GetVitality(i, MY_PLAYER, G) == 0) continue;
      _(PUT, i);
      IToN(i, attack);
      CallWithSlot(mvi, i);
      break;
    }

    mv = GetVitality(mvi, MY_PLAYER, G);
  }
}

void Work() {
  while (1) {
    DoWork();
  }
}

int main(int argc, char** argv) {
  assert(argc == 2);
  G = create_game();
  MY_PLAYER = 0;
  if (argv[1] == string("1")) {
    MY_PLAYER = 1;
    Opp();
  }
  Work();
  return 0;
}
