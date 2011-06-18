#include <cassert>
#include <iostream>
#include <string>

extern "C" {
#include "../../sim/sim.h"
#include "../../sim/types.h"
}

struct game* G;
int MY_PLAYER = 0;

using namespace std;

int GetOppVitality(int i, struct game* g) {
  return g->users[MY_PLAYER == 0 ? 1 : 0].slots[i].vitality;
}

int GetMyVitality(int i, struct game* g) {
  return g->users[MY_PLAYER].slots[i].vitality;
}

const int INVALID_VALUE = -100000;

int GetMyValue(int i, struct game* g) {
  struct value* v = g->users[MY_PLAYER].slots[i].field;
  if (v->type == TYPE_INTEGER)
    return v->u.integer;
  else
    return INVALID_VALUE;
}

struct function* GetMyFunc(int i, struct game* g) {
  struct value* v = g->users[MY_PLAYER].slots[i].field;
  if (v->type == TYPE_FUNCTION)
    return &(v->u.function);
  return NULL;
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
  cout << card_names[static_cast<int>(c)] << endl;
  cout << i << endl;
  struct value* v = find_card_value(card_names[c]);
  apply_cs(v, i, g);
}

template<>
void __(int i, Card c, struct game* g) {
  cout << 2 << endl;
  cout << i << endl;
  cout << card_names[static_cast<int>(c)] << endl;
  struct value* v = find_card_value(card_names[c]);
  apply_sc(i, v, g);
}

template<typename T, typename U>
void _(T lhs, U rhs) {
  __(lhs, rhs, G);
  Opp();
}

// If the ith slot is not I, call PUT.
void MaybePut(int i) {
  struct value* v = G->users[MY_PLAYER].slots[i].field;
  if (v->type == TYPE_FUNCTION &&
      string("I") == v->u.function.ops->name)
    return;
  _(PUT, i);
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

// return true when revived
bool ReviveIfDeath(int slot) {
  if (GetMyVitality(slot, G) > 0) {
    return false;
  }

  int alive = 0;
  for (int i = 10; i < 255; ++i) {
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

void TryRevive() {
  int alive = 0;
  for (int i = 10; i < 255; ++i) {
    if (GetMyVitality(i, G) > 1000) {
      alive = i;
      break;
    }
  }
  for (int i = 0; i < 255; ++i) {
    if (GetMyVitality(i, G) <= 0) {
      _(PUT, alive); // alive : I
      IToN(alive, i); // alive : i
      _(REVIVE, alive); // revive i
    }
  }
}

void DoWork() {
  MaybePut(0);
  IToN(0, 3);  // 0: 3
  _(GET, 0);  // 0: 8192

  // Repeat help(0)(1)(8192) and help(1)(0)(8192) to increase life.
  for (int i = 0; i < 68; ++i) {
    // help(0)(1)(8192)
    // Assumes 0: 8192
    if (GetMyValue(0, G) != 8192)
      return;
    MaybePut(1);
    IToN(1, 4);  // 1: 4
    _(GET, 1);  // 1: help(0)(1)
    CallWithSlot(1, 0);  // help(0)(1)(8192) -> I

    // help(1)(0)(8192)
    // Assumes 0: 8192
    if (GetMyValue(0, G) != 8192)
      return;
    MaybePut(1);
    IToN(1, 5);  // 1: 5
    _(GET, 1);  // 1: help(1)(0)
    CallWithSlot(1, 0);  // help(1)(0)(8192) -> I

    TryRevive();
  }

  _(PUT, 0);  // 0: I

  static int o = 0;

  // Using life of slot 0 and 1 kill opponent's slots repeatedly.

  // attack(0)(o)(11112)
  for (int i = 0; i < 5; ++i) {
    if (GetMyValue(2, G) != 11112)
      return;

    _(1, ZERO);  // 1: 0
    _(ATTACK, 1);  // 1: attack(0)
    _(0, ZERO);  // 0: 0
    ToN(0, o);  // 0: o
    CallWithSlot(1, 0);  // 1: attack(0)(o)
    CallWithSlot(1, 2);  // 1: attack(0)(o)(11112) -> I
    _(PUT, 0);  // 0: I

    int target = 255 - o;
    if (GetOppVitality(target, G) <= 0) {
      // The opppent o is dead.
      o++;
    }
  }

  TryRevive();

  // attack(1)(o)(11112)
  for (int i = 0; i < 5; ++i) {
    if (GetMyValue(2, G) != 11112)
      return;

    _(1, ZERO);  // 1: 0
    _(SUCC, 1);  // 1: 1
    _(ATTACK, 1);  // 1: attack(1)
    _(0, ZERO);  // 0: 0
    ToN(0, o);  // 0: o
    CallWithSlot(1, 0);  // 1: attack(1)(o)
    CallWithSlot(1, 2);  // 1: attack(1)(o)(11112) -> I
    _(PUT, 0);  // 0: I

    int target = 255 - o;
    if (GetOppVitality(target, G) <= 0) {
      // The opppent o is dead.
      o++;
    }
  }

  TryRevive();
}

void SetUp() {
  // Precompute functions/values.

  for (int i = 0; i < 1000; ++i) {
    // 2: 11112
    if (GetMyValue(2, G) != 11112) {
      MaybePut(2);
      IToN(2, 11112);  // 2: 11112
    }
    if (ReviveIfDeath(2))
      continue;

    // 3: 8192
    if (GetMyValue(3, G) != 8192) {
      MaybePut(3);
      IToN(3, 8192);  // 3: 8192
    }
    if (ReviveIfDeath(2) || ReviveIfDeath(3))
      continue;

    // 4: help(0)(1)
    struct function* func4 = GetMyFunc(4, G);
    if (func4 == NULL ||
	string("help") != func4->ops->name ||
	func4->nr_args != 2 ||
	func4->args[0]->type != TYPE_INTEGER ||
	func4->args[0]->u.integer != 0 ||
	func4->args[1]->type != TYPE_INTEGER ||
	func4->args[1]->u.integer != 1) {

      MaybePut(4);
      _(4, HELP);  // 4: help
      CallWithValue(4, 0);  // 4: help(0)
      CallWithValue(4, 1);  // 4: help(0)(1)
    }
    if (ReviveIfDeath(2) ||
	ReviveIfDeath(3) ||
	ReviveIfDeath(4))
      continue;
      
    // 5: help(1)(0)
    struct function* func5 = GetMyFunc(4, G);
    if (func5 == NULL ||
	string("help") != func5->ops->name ||
	func5->nr_args != 2 ||
	func5->args[0]->type != TYPE_INTEGER ||
	func5->args[0]->u.integer != 1 ||
	func5->args[1]->type != TYPE_INTEGER ||
	func5->args[1]->u.integer != 0) {

      MaybePut(5);
      _(5, HELP);  // 5: help
      CallWithValue(5, 1);  // 5: help(1)
      CallWithValue(5, 0);  // 5: help(1)(0)  
    }

    if (ReviveIfDeath(2) ||
	ReviveIfDeath(3) ||
	ReviveIfDeath(4) ||
	ReviveIfDeath(5))
      continue;

    break;
  }
}

void Work() {
  while (1) {
    SetUp();
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
  destroy_game(G);
  return 0;
}
