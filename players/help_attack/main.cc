#include "global.h"
#include "sim_util.h"

#include <cassert>
#include <iostream>
#include <string>
#include <queue>

using namespace std;

struct game* G;
bool DEBUG;
int MY_PLAYER;
int OPP_PLAYER;

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



int GetFuncLength(const struct function* func) {
  int len = 1;
  for (int i = 0; i < func->nr_args; ++i) {
    if (func->args[i]->type == TYPE_FUNCTION)
      len += GetFuncLength(&(func->args[i]->u.function));
    else
      len++;
  }
  return len;
}

int GetScore(int i, struct game* g) {
  struct function* func = GetOppFunc(i, g);
  if (func && string("I") != func->ops->name) {
    return 20000 + i + GetFuncLength(func) * 100;
  } else {
    int value = GetOppValue(i, g);
    if (value != INVALID_VALUE) {
      int v = static_cast<int>(10000.0 * value / 6535);
      return 10000 + v + i;
    }
  }
  return i;
}


void Zombie(int target) {
  int tmp = 0;
  for (int i = 20; i < 255; ++i) {
    if (GetMyVitality(i, G) > 0)
      tmp = i;
  }
  if (tmp <= 0)
    return;

  MaybePut(tmp);
  IToN(tmp, 255 - target);  // tmp : target - 255
  _(ZOMBIE, tmp);   // tmp : zombie(target - 255)
  _(tmp, I);  // tmp : zomvie(target - 255, I)
  if (DEBUG)
    cerr << "zomvie!!" << endl;
}

void MaybeZombie(int target) {
  if (GetOppVitality(target, G) == 0) {
    if (DEBUG) {
      cerr << "kill " << target << endl;
      print_slot(OPP_PLAYER, target, &G->users[OPP_PLAYER].slots[target]);
    }
    struct function* func = GetOppFunc(target, G);
    if (func && GetFuncLength(func) > 5) {
      Zombie(target);
    }
  }
}

// Find alive target in [start, end)
int FindTarget(int start, int end) {
  for (int i = 255; i >= 0; --i) {
    if (GetOppVitality(i, G) > 0)
      return i;
  }
  priority_queue< pair<int,int> > queue;
  for (int i = start; i < end; ++i) {
    if (GetOppVitality(i, G) > 0) {
      int score = GetScore(i, G);
      if (score > 0)
	queue.push(make_pair(score, i));
    }
  }
  if (queue.size() > 0) {
    return queue.top().second;
  } else {
    return -1;
  }
}

void DoWork() {
  int mv = -1;  // Maximum vitality.
  int mvi = -1;  // Slot number of maximum vitality.
  for (int i = 0; i < 256; ++i) {
    int v = GetMyVitality(i, G);
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
      int v = GetMyVitality(i, G);
      if (sv < v) {
        sv = v;
        svi = i;
      }
    }

    assert(sv > 0 && svi >= 0);

    CallWithValue(mvi, svi);  // mvi: help(mvi)(svi)
    int move = min(mv - 100, 65535 - sv);  // We leave at least 100 life.
    CallWithValue(mvi, move);  // mvi: I
    mv = GetMyVitality(svi, G);
    mvi = svi;
  }

  while (mv > 10000) {
    int av = -1;  // Maximum vitality in opponent.
    int avi = -1;  // Number of slot which have maximum vitality in opponent.
    for (int i = 0; i < 256; ++i) {
      int v = GetOppVitality(i, G);
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

    MaybeZombie(avi);

    // We want to simply do CallWithValue(mvi, attack) here, but this takes time
    // and before we become to be able to attack, opponent may move his life to
    // other slot using help. Thus instead of that we take fast approach.
    //CallWithValue(mvi, attack);  // mvi: attack(mvi)(255 - avi)(attack)

    // Fast approach.
    for (int i = 0; i < 256; ++i) {
      if (i == mvi) continue;
      if (GetMyVitality(i, G) == 0) continue;
      _(PUT, i);
      IToN(i, attack);
      CallWithSlot(mvi, i);

      MaybeZombie(avi);
      break;
    }

    mv = GetMyVitality(mvi, G);
  }
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
    DoWork();
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
