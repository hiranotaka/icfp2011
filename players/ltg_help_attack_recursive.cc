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

  static int dead[256] = {};
  for (int i = 0; i < 256; ++i) {
    if (!dead[i] && GetVitality(i, MY_PLAYER, G) == 0) {
      dead[i] = 1;
      throw 0;
    }
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

#define arraysize(a) (int)((sizeof(a)/sizeof((a)[0])))

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

// If the ith slot is not I, call PUT.
void MaybePut(int i) {
  struct value* v = G->users[MY_PLAYER].slots[i].field;
  if (v->type == TYPE_FUNCTION &&
      string("I") == v->u.function.ops->name)
    return;
  _(PUT, i);
}

const int kHelpRange = 54;

void Help(int offset, int help_n) {
//   MaybePut(0);  // 0: I
//   IToN(0, 8192);  // 0: 8192
//   _(K, 0);  // 0: K(8192)
//   MaybePut(1);  // 1: I
//   _(1, HELP);  // 1: HELP
//   _(S, 1);  // 1: S(help)
//   _(1, SUCC);  // 1: S(help)(succ)
//   _(S, 1);  // 1: S(S(help)(succ))
//   CallWithSlot(1, 0);  // 1: S(S(help)(succ))(K(8192))

  // help -> S(revive)(help)
  MaybePut(1);  // 1: I
  _(1, REVIVE);  // 1: revive
  _(S, 1);  // 1: S(revive)
  _(1, HELP);  // 1: S(revive)(help)
  MaybePut(0);  // 0: I
  IToN(0, help_n);  // 0: 8192
  _(K, 0);  // 0: K(8192)
  _(S, 1);  // 1: S(S(revive)(help))
  _(1, SUCC);  // 1: S(S(revive)(help))(succ)
  _(S, 1);  // 1: S(S(S(revive)(help))(succ))
  CallWithSlot(1, 0);  // 1: S(S(S(revive)(help))(succ))(K(8192))

  MaybePut(0);  // 0: I
  _(0, GET);  // 0: get
  _(K, 0);  // 0: K(get)

  _(S, 1);  // 1: S(S(S(help)(succ))(K(8192)))
  CallWithSlot(1, 0);  // 1: S(S(S(help)(succ))(K(8192)))(K(get))

  MaybePut(0);  // 0: I
  _(0, ZERO);  // 0: zero
  _(SUCC, 0);  // 0: 1
  _(K, 0);  // 0: K(1)

  _(S, 1);  // 1: S(S(S(S(help)(succ))(K(8192)))(K(get)))
  CallWithSlot(1, 0);  // 1: S(S(S(S(help)(succ))(K(8192)))(K(get)))(K(1))

  _(S, 1);  // 1: S(S(S(S(S(help)(succ))(K(8192)))(K(get)))(K(1)))
  _(1, SUCC);  // 1: S(S(S(S(S(help)(succ))(K(8192)))(K(get)))(K(1)))(succ)

  // turn 74 here.

  // S(S(K(get))(K(1)))(I)
  MaybePut(2);  // 2: I
  _(2, ZERO);  // 2: zero
  _(SUCC, 2);  // 2: 1
  _(K, 2);  // 2: K(1)
  MaybePut(0);  // 0: I
  _(0, GET);  // 0: get
  _(K, 0);  // 0: K(get)
  _(S, 0);  // 0: S(K(get))
  CallWithSlot(0, 2);  // 0: S(K(get))(K(1))
  _(S, 0);  // 0: S(S(K(get))(K(1)))
  _(0, I);  // 0: S(S(K(get))(K(1)))(I)

  MaybePut(2);  // 2: I
  IToN(2, offset + kHelpRange);  // 2: 67
  MaybePut(3);  // 3: I
  IToN(3, help_n);  // 3: 8192
  MaybePut(6);  // 6: I
  IToN(6, offset);

  //  while (1) {
  for (int i = 0; i < 2; ++i) {
    MaybePut(4);  // 4: I
    _(4, ZERO);  // 4: 0
    _(GET, 4);  // 4: S(K(get))(K(1))

    // help(67)(0)(8192)
    MaybePut(5);  // 5: I
    IToN(5, 2);  // 5: 2
    _(GET, 5);  // 5: 67
    _(HELP, 5);  // 5: help(67)
    //_(4, ZERO);  // 4: help(67)(0)
    CallWithSlot(5, 6);

    // Fire!
    CallWithSlot(4, 6);

    // Help!
    CallWithSlot(5, 3);  // 4: help(67)(0)(8192) -> I
  }
}

const int kAttackRange = 55;

void Attack(int offset) {
  MaybePut(0);  // 0: I
  IToN(0, 11112);  // 0: 11112
  _(K, 0);  // 0: K(11112)
  MaybePut(1);  // 1: I
  _(1, ATTACK);  // 1: attack
  _(S, 1);  // 1: S(attack)
  _(1, I);  // 1: S(attack)(I)
  _(S, 1);  // 1: S(S(attack)(I))
  CallWithSlot(1, 0);  // 1: S(S(attack)(I))(K(11112))

  MaybePut(0);  // 0: I
  _(0, GET);  // 0: get
  _(K, 0);  // 0: K(get)

  _(S, 1);  // 1: S(S(S(attack)(I))(K(11112)))
  CallWithSlot(1, 0);  // 1: S(S(S(attack)(I))(K(11112)))(K(get))

  MaybePut(0);  // 0: I
  IToN(0, 1);  // 0: 1
  _(K, 0);  // 0: K(1)

  _(S, 1);  // 1: S(S(S(S(attack)(I))(K(11112)))(K(get)))
  CallWithSlot(1, 0);  // 1: S(S(S(S(attack)(I))(K(11112)))(K(get)))(K(1))

  _(S, 1);  // 1: S(S(S(S(S(attack)(I))(K(11112)))(K(get)))(K(1)))
  _(1, SUCC);  // 1: S(S(S(S(S(attack)(I))(K(11112)))(K(get)))(K(1)))(succ)

  // S(S(K(get))(K(1)))(I)
  MaybePut(2);  // 2: I
  _(2, ZERO);  // 2: zero
  _(SUCC, 2);  // 2: 1
  _(K, 2);  // 2: K(1)
  MaybePut(0);  // 0: I
  _(0, GET);  // 0: get
  _(K, 0);  // 0: K(get)
  _(S, 0);  // 0: S(K(get))
  CallWithSlot(0, 2);  // 0: S(K(get))(K(1))
  _(S, 0);  // 0: S(S(K(get))(K(1)))
  _(0, I);  // 0: S(S(K(get))(K(1)))(I)

  MaybePut(3);  // 3: I
  IToN(3, offset);  // 3: attack_offsets[i]

  MaybePut(2);  // 2: I
  _(2, ZERO);  // 2: zero
  _(GET, 2);  // 2: S(S(K(get))(K(1)))(I)
  CallWithSlot(2, 3);
}

int CountDead(int l, int r) {
  int dead = 0;
  for (int i = l; i < r; ++i) {
    if (GetVitality(i, MY_PLAYER, G) == 0) {
      ++dead;
    }
  }
  return dead;
}

void DoRevive(int offset) {
  int mv = -1;
  int mvi = -1;
  for (int i = 0; i < 256; ++i) {
    int v = GetVitality(i, MY_PLAYER, G);
    if (mv < v) {
      mv = v;
      mvi = i;
    }
  }

  int sv = -1;
  int svi = -1;
  for (int i = 0; i < 256; ++i) {
    if (i == mvi) continue;
    int v = GetVitality(i, MY_PLAYER, G);
    if (sv < v) {
      sv = v;
      svi = i;
    }
  }

  // We have only one slot. Give up.
  if (sv == -1) while (1) _(PUT, 1);

  int tv = -1;
  int tvi = -1;
  for (int i = 0; i < 256; ++i) {
    if (i == mvi || i == svi) continue;
    int v = GetVitality(i, MY_PLAYER, G);
    if (tv < v) {
      tv = v;
      tvi = i;
    }
  }

  // We have only two slot. Give up.
  if (tv == -1) while (1) _(PUT, 1);

  // Increasing order infinite loop.
  // S (S (S (F) (K (get) ) ) (K (zero) ) ) (succ)

  MaybePut(svi);
  _(svi, GET);
  _(K, svi);  // svi: K(get)

  MaybePut(mvi);
  _(mvi, REVIVE);
  //_(mvi, DEC);
  _(S, mvi);
  CallWithSlot(mvi, svi);
  _(S, mvi);  // mvi: S(S(revive)(K(get)))

  MaybePut(svi);
  IToN(svi, mvi);
  _(K, svi);  // svi: K(mvi)

  CallWithSlot(mvi, svi);  // mvi: S(S(revive)(K(get)))(K(mvi))

  _(S, mvi);  // mvi: S(S(S(revive)(K(get)))(K(mvi)))
  _(mvi, SUCC);  // mvi: S(S(S(revive)(K(get)))(K(mvi)))(succ)

  // Try to make svi: S(S(K(get))(K(mvi)))(I)
  MaybePut(tvi);  // tvi: I
  IToN(tvi, mvi);  // tvi: mvi
  _(K, tvi);  // tvi: K(mvi)

  MaybePut(svi);  // svi: I
  _(svi, GET);  // svi: get
  _(K, svi);  // svi: K(get)
  _(S, svi);  // svi: S(K(get))
  CallWithSlot(svi, tvi);  // svi: S(K(get))(K(mvi))
  _(S, svi);  // svi: S(S(K(get))(K(mvi)))
  _(svi, I);  // svi: S(S(K(get))(K(mvi)))(I)

  CallWithValue(svi, offset);
  //sleep(1);
  _(mvi, ZERO);
}

void Revive() {
  int before_dead = CountDead(0, 256);
  if (before_dead == 0) return;
  if (CountDead(0, 110) > 0) DoRevive(0);
  if (CountDead(110, 210) > 0) DoRevive(110);
  if (CountDead(149, 256) > 0) DoRevive(149);
  cerr << before_dead << " " << CountDead(0, 256) << endl;
  //sleep(1);
  //sleep(1);
}

void DoWork() {
  static int help_offsets[] = {0, 50, 100, 150, 201};
  static int attack_offsets[] = {0, 50, 100, 150, 201};
  const int size = arraysize(help_offsets);

  for (int i = 0; i < size; ++i) {
    Revive();
    int n = 1, v = GetVitality(help_offsets[size - 1 - i], MY_PLAYER, G);
    while ((n << 1) < v) n <<= 1;
    Help(help_offsets[size - 1 - i], n);
    Revive();
    Attack(attack_offsets[size - 1 - i]);
  }
}

void Work() {
  while (1) {
    try {
      DoWork();
    } catch (int e) {
    }
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
