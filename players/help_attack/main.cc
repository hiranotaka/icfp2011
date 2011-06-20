#include "global.h"
#include "sim_util.h"
#include "util.h"
extern "C" {
#include "../../compiler/compiler.h"
#include "../../compiler/parser.h"
}

#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
#include <queue>

using namespace std;

struct game* G;
bool DEBUG;
int SLEEP_TIME;
int MY_PLAYER;
int OPP_PLAYER;

#define arraysize(a) (sizeof(a)/sizeof((a)[0]))

bool do_run_expr(const char* expr) {
	struct compile_result result;
	struct value* value;
	if (!parse(expr, NULL, &value)) {
		fprintf(stderr, "PARSE FAILED: %s\n", expr);
		return false;
	}
	compile(value, &result, G);
	unref_value(value);
	if (result.nr_turns > 2000000 || !result.nr_turns)
		return false;
	if (result.first_method == METHOD_CS)
		_(result.first_card_name, result.first_slot_index);
	else
		_(result.first_slot_index, result.first_card_name);
	return result.nr_turns > 1;
}

void run_expr(const char* expr) {
	while (do_run_expr(expr));
}

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

void TryRevive() {
#ifndef USE_COMPILER
  int alive = 0;
  for (int i = 10; i < 256; ++i) {
    if (GetMyVitality(i, G) > 1000) {
      alive = i;
      break;
    }
  }
#endif
  for (int i = 0; i < 256; ++i) {
    if (GetMyVitality(i, G) <= 0) {
#ifdef USE_COMPILER
	    char expr[32];
	    sprintf(expr, "revive(%d)", i);
	    run_expr(expr);
#else
      _(PUT, alive); // alive : I
      IToN(alive, i); // alive : i
      _(REVIVE, alive); // revive i
#endif
    }
  }
}

void TryAllRevive() {
  priority_queue<pair<int, int> > queue;
  for (int i = 0; i < 256; i++) {
    int v = GetMyVitality(i, G);
    if (v > 100)
      queue.push(make_pair(255 - i, i));
  }
  if (queue.size() < 3)
    return;

  int tmp1 = queue.top().second;
  queue.pop();
  MaybePut(tmp1);


  _(tmp1, GET);
  _(K, tmp1);
  _(S, tmp1); // tmp1: S(K(get)

  int tmp2 = queue.top().second;
  queue.pop();
  MaybePut(tmp2);
  IToN(tmp2, tmp1);
  _(K, tmp2); // tmp2: K(tmp2)

  CallWithSlot(tmp1, tmp2); // tmp1: S(K(get) K(zero))

  _(S, tmp1);

  MaybePut(tmp2);
  _(tmp2, REVIVE);
  _(S, tmp2);
  _(tmp2, SUCC); // tmp2: S(revive succ)

  // S(S(K(get) K(zero)) S(revive succ))
  CallWithSlot(tmp1, tmp2);

  sleep(10);
  _(ZERO, tmp1);
  sleep(10000);
  return;

  int tmp3 = queue.top().second;
  queue.pop();
  for (int i = 0; i < 4; ++i) {
    MaybePut(tmp2);
    IToN(tmp2, tmp1);
    _(GET, tmp2);

    MaybePut(tmp3);
    IToN(tmp3, 64 * i);

    CallWithSlot(tmp2, tmp3);

    sleep(1000);

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
    return 20000 + i + GetFuncLength(func) * 1000;
  } else {
    int value = GetOppValue(i, g);
    if (value != INVALID_VALUE) {
      int v = static_cast<int>(10000.0 * value / 6535);
      return 10000 + v + i;
    }
  }
  return i;
}


// Find alive target in [start, end)
int FindTarget(int start, int end) {
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

void Zombie(int target) {
  int tmp = 0;
  for (int i = 20; i < 256; ++i) {
    if (GetMyVitality(i, G) > 0)
      tmp = i;
  }
  if (tmp <= 0)
    return;

#ifdef USE_COMPILER
  char expr[32];
  sprintf(expr, "zombie(%d)(I)", 255 - target);
  run_expr(expr);
#else
  MaybePut(tmp);
  IToN(tmp, 255 - target);  // tmp : target - 255
  _(ZOMBIE, tmp);   // tmp : zombie(target - 255)
  _(tmp, I);  // tmp : zomvie(target - 255, I)
#endif

  if (DEBUG)
    cerr << "zomvie!!" << endl;
}

void ZombiePowder(int death) {
  int target = FindTarget(0, 256);
  if (target < 0)
    return;
  
  priority_queue< pair<int,int> > queue;
  for (int i = 0; i < 256; ++i) {
    int v = GetMyVitality(i, G);
    if (v > 100)
      queue.push(make_pair(255-i, i));
  }
  if (queue.size() < 2)
    return;

  int tmp1 = queue.top().second;
  queue.pop();
  tmp1 = 0;

  MaybePut(tmp1);
  IToN(tmp1, target); // tmp1: target
  _(ATTACK, tmp1); // tmp1: attack(target)
  _(tmp1, ZERO); // tmp1: attack(target, 0)
  _(K, tmp1); // tmp1: K(attack(target, 0))
  _(S, tmp1); // tmp1: S(K(attack(target, 0)))

  int tmp2 = queue.top().second;
  queue.pop();
  MaybePut(tmp2);
  int power = GetOppVitality(target, G);
  power = 1;
  IToN(tmp2, power); // tmp2: power
  _(K, tmp2); // tmp2: K(power)

  // tmp1: S(attack(target, 0), K(power))
  CallWithSlot(tmp1, tmp2);

  MaybePut(tmp2); // tmp2: I
  IToN(tmp2, 255 - death);  // tmp2: death
  _(ZOMBIE, tmp2); // tmp2: zombie(death)

  cerr << "before zombie" << endl;

  sleep(5);
 
  // tmp2: zombie(death, S(attack(target, 0), K(power))) 
  CallWithSlot(tmp2, tmp1);

  cerr << "zombie powder attack("
       << target << ", 0, " << power << ")" << endl;
  cerr << "after zombie" << endl;

  SLEEP_TIME = 100;
}

void ZombiePowder2(int death) {
  int target = FindTarget(0, 256);
  if (target < 0)
    return;
  
  priority_queue<pair<int, int> > queue;
  for (int i = 0; i < 256; i++) {
    int v = GetMyVitality(i, G);
    if (v > 10)
      queue.push(make_pair(255 - i, i));
  }
  if (queue.size() < 3)
    return;

  int tmp1 = queue.top().second;
  queue.pop();

  MaybePut(tmp1);
  IToN(tmp1, target); // tmp1: target
  _(ATTACK, tmp1); // tmp1: attack(target)
  _(tmp1, ZERO); // tmp1: attack(target, 0)
  _(K, tmp1); // tmp1: K(attack(target, 0))
  _(S, tmp1); // tmp1: S(K(attack(target, 0)))


  int tmp2 = queue.top().second;
  queue.pop();
  MaybePut(tmp2);
  int power = min(GetOppVitality(target, G), 1);

  IToN(tmp2, power); // tmp2: power
  _(K, tmp2); // tmp2: K(power)

  // tmp1: S(K(attack(target, 0)), K(power))
  CallWithSlot(tmp1, tmp2);

  
  //MaybePut(tmp2);
  //_(K, tmp2); // tmp2: K(I)
  //_(K, tmp2); // tmp2: K(K(I))
  //_(S, tmp2); // tmp2: S(K(K(I)))
  // tmp2: S(K(K(I)) S(K(attack(target, 0)), K(power)))
  //CallWithSlot(tmp2, tmp1); 

  MaybePut(tmp2); // tmp2: I
  _(tmp2, GET); // tmp2: get
  _(K, tmp2);  // tmp2: K(get)
  _(S, tmp2); // tmp2: S(K(get))

  int tmp3 = queue.top().second;
  queue.pop();
  MaybePut(tmp3); // tmp3: I
  IToN(tmp3, death); // tmp3: death
  _(K, tmp3); // tmp3: K(death)
  
  CallWithSlot(tmp2, tmp3); // tmp2: S(K(get) K(death))

  _(S, tmp2); // tmp2: S(S(K(get) K(target)))

  // tmp2: S(K(get) K(death)) S(K(attack(target, 0)), K(power))
  CallWithSlot(tmp2, tmp1);

  MaybePut(tmp1); // tmp2: I
  IToN(tmp1, 255 - death);  // tmp2: death
  _(ZOMBIE, tmp1); // tmp1: zombie(255 - death)

  cerr << "before zombie" << endl;

  sleep(5);
 
  // tmp2: zombie(death, S(attack(target, 0), K(power))) 
  CallWithSlot(tmp1, tmp2);

  cerr << "zombie powder attack("
       << target << ", 0, " << power << ")" << endl;
  cerr << "after zombie" << endl;

  SLEEP_TIME = 100;
}

void ZombiePowder3(int death) {
  int target = FindTarget(0, 256);
  if (target < 0)
    return;
  
  priority_queue<pair<int, int> > queue;
  for (int i = 0; i < 256; i++) {
    int v = GetMyVitality(i, G);
    if (v > 10)
      queue.push(make_pair(255 - i, i));
  }
  if (queue.size() < 3)
    return;

  int tmp1 = queue.top().second;
  queue.pop();

  MaybePut(tmp1);
  _(tmp1, INC); // tmp1: inc
  _(K, tmp1); // tmp1: K(inc)
  _(S, tmp1); // tmp1: S(K(inc))

  int tmp2 = queue.top().second;
  queue.pop();
  IToN(tmp2, target); // tmp2: target
  _(K, tmp2); // tmp2: K(target)

  // tmp1: S(K(inc), K(target))
  CallWithSlot(tmp1, tmp2);

  MaybePut(tmp2); // tmp2: I
  _(tmp2, GET); // tmp2: get
  _(K, tmp2);  // tmp2: K(get)
  _(S, tmp2); // tmp2: S(K(get))

  int tmp3 = queue.top().second;
  queue.pop();
  MaybePut(tmp3); // tmp3: I
  IToN(tmp3, death); // tmp3: death
  _(K, tmp3); // tmp3: K(death)
  
  CallWithSlot(tmp2, tmp3); // tmp2: S(K(get) K(death))

  _(S, tmp2); // tmp2: S(S(K(get) K(target)))

  CallWithSlot(tmp2, tmp1);

  MaybePut(tmp1); // tmp1: I
  IToN(tmp1, 255 - death);  // tmp2: death
  _(ZOMBIE, tmp1); // tmp1: zombie(255 - death)

  cerr << "before zombie" << endl;

  sleep(5);
 
  // tmp2: zombie(death, S(attack(target, 0), K(power))) 
  CallWithSlot(tmp1, tmp2);

  cerr << "after zombie" << endl;

  SLEEP_TIME = 100;
}

void ZombiePowder4(int death) {
  int target = FindTarget(0, 256);
  if (target < 0)
    return;
  
  priority_queue<pair<int, int> > queue;
  for (int i = 0; i < 256; i++) {
    int v = GetMyVitality(i, G);
    if (v > 10)
      queue.push(make_pair(255 - i, i));
  }
  if (queue.size() < 3)
    return;

  int tmp1 = queue.top().second;
  queue.pop();

  MaybePut(tmp1);
  _(tmp1, REVIVE); // tmp1: revive
  _(S, tmp1); // tmp1: S(revive)
  _(tmp1, GET); // tmp1: S(revive, get)
  _(K, tmp1); // tmp1: K(S(revive, get))
  _(S, tmp1); // tmp1: S(K(S(revive, get)))
  
  int tmp2 = queue.top().second;
  queue.pop();
  IToN(tmp2, death); // tmp2: death
  _(K, tmp2); // tmp2: K(death)

  // tmp1: S(K(S(revive, get)), K(death))
  CallWithSlot(tmp1, tmp2);

  MaybePut(tmp2); // tmp2: I
  IToN(tmp2, 255 - death);  // tmp2: death
  _(ZOMBIE, tmp2); // tmp2: zombie(255 - death)

  cerr << "before zombie" << endl;

  sleep(5);
 
  // tmp2: zombie(death, S(attack(target, 0), K(power))) 
  CallWithSlot(tmp2, tmp1);

  cerr << "after zombie" << endl;

  SLEEP_TIME = 100;
}

void ZombiePowder5(int death) {
  int target = FindTarget(0, 256);
  if (target < 0)
    return;
  
  priority_queue<pair<int, int> > queue;
  for (int i = 0; i < 256; i++) {
    int v = GetMyVitality(i, G);
    if (v > 10)
      queue.push(make_pair(255 - i, i));
  }
  if (queue.size() < 3)
    return;

  int tmp1 = queue.top().second;
  queue.pop();

  MaybePut(tmp1);
  _(tmp1, REVIVE); // tmp1: revive
  _(S, tmp1); // tmp1: S(revive)
  _(tmp1, GET); // tmp1: S(revive, get)
  _(K, tmp1); // tmp1: K(S(revive, get))
  _(S, tmp1); // tmp1: S(K(S(revive, get)))
  
  int tmp2 = queue.top().second;
  queue.pop();
  IToN(tmp2, death); // tmp2: death
  _(K, tmp2); // tmp2: K(death)

  // tmp1: S(K(S(revive, get)), K(death))
  CallWithSlot(tmp1, tmp2);

  _(S, tmp1); // S(S(K(S(revive, get)), K(death)))

  MaybePut(tmp2);
  IToN(tmp2, target); // tmp2: target
  _(ATTACK, tmp2); // tmp2: attack(target)
  _(tmp2, ZERO); // tmp2: attack(target, 0)
  _(K, tmp2); // tmp2: K(attack(target, 0))
  _(S, tmp2); // tmp2: S(K(attack(target, 0)))
  
  int tmp3 = queue.top().second;
  queue.pop();
  MaybePut(tmp3);
  IToN(tmp3, 1); // tmp3: 1
  _(K, tmp3); // tmp3: K(1)
  
  CallWithSlot(tmp2, tmp3);

  CallWithSlot(tmp1, tmp2);

  MaybePut(tmp3); // tmp3: I
  IToN(tmp3, 255 - death);  // tmp3: death
  _(ZOMBIE, tmp3); // tmp3: zombie(255 - death)

  cerr << "before zombie" << endl;

  sleep(5);
 
  // tmp2: zombie(death, S(attack(target, 0), K(power))) 
  CallWithSlot(tmp3, tmp1);

  cerr << "after zombie" << endl;

  SLEEP_TIME = 100;
}

void ZombiePowder6(int death) {
  int target = FindTarget(0, 256);
  if (target < 0)
    return;
  
  priority_queue<pair<int, int> > queue;
  for (int i = 0; i < 256; i++) {
    int v = GetMyVitality(i, G);
    if (v > 10)
      queue.push(make_pair(255 - i, i));
  }
  if (queue.size() < 3)
    return;

  int tmp1 = queue.top().second;
  queue.pop();

  MaybePut(tmp1);
  _(tmp1, COPY); // tmp1: copy
  _(K, tmp1); // tmp1: K(copy)
  _(S, tmp1); // tmp1: S(K(copy))
  
  int tmp2 = queue.top().second;
  queue.pop();
  MaybePut(tmp2);
  IToN(tmp2, tmp1); // tmp2: tmp1
  _(K, tmp2); // tmp2: K(tmp1)

  // tmp1: S(K(copy), K(tmp1))
  CallWithSlot(tmp1, tmp2);

  _(S, tmp1);

  MaybePut(tmp2);
  IToN(tmp2, target); // tmp2: target
  _(ATTACK, tmp2); // tmp2: attack(target)
  _(tmp2, ZERO); // tmp2: attack(target, 0)
  _(K, tmp2); // tmp2: K(attack(target, 0))
  _(S, tmp2); // tmp2: S(K(attack(target, 0)))
  
  int tmp3 = queue.top().second;
  queue.pop();
  MaybePut(tmp3);
  IToN(tmp3, 1); // tmp3: 1
  _(K, tmp3); // tmp3: K(1)
  
  CallWithSlot(tmp2, tmp3);

  CallWithSlot(tmp1, tmp2);

  MaybePut(tmp3); // tmp3: I
  IToN(tmp3, 255 - death);  // tmp3: death
  _(ZOMBIE, tmp3); // tmp3: zombie(255 - death)

  cerr << "before zombie" << endl;

  sleep(5);
 
  // tmp2: zombie(death, S(attack(target, 0), K(power))) 
  CallWithSlot(tmp3, tmp1);

  cerr << "after zombie" << endl;

  SLEEP_TIME = 100;
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
      //ZombiePowder6(target);
    }
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

  if (mv <= 0 || mvi < 0)
    return;

  while (mv < 65535) {
#ifndef USE_COMPILER
    _(PUT, mvi);  // mvi: I
    IToN(mvi, mvi);  // mvi: mvi
    _(HELP, mvi);  // mvi: help(mvi)
#endif

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

    if (sv <= 0 || svi < 0)
      return;

#ifndef USE_COMPILER
    CallWithValue(mvi, svi);  // mvi: help(mvi)(svi)
#endif

    int move = min(mv - 100, 65535 - sv);  // We leave at least 100 life.
#ifdef USE_COMPILER
    char expr[32];
    sprintf(expr, "help(%d)(%d)(%d)", mvi, svi, move);
    run_expr(expr);
#else
    CallWithValue(mvi, move);  // mvi: I
#endif

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

    if (av <= 0 || avi < 0)
      return;

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

void KillZero() {
  int help_i = 1;

  while (help_i < 250) {
    int help_j = help_i + 1;

    int v = GetOppVitality(0, G);
    if (v <= 0)
      return;

    int opp_v = GetOppVitality(0, G);
    int power = opp_v * 10 / 9 + 1;
   
#ifndef USE_COMPILER
    MaybePut(0);
    IToN(0, help_i);
    _(HELP, 0);  // 0: help(help_i)
    CallWithValue(0, help_j); // help(help_i, help_j)
#endif

    // We leave at least 100 life.
    int my_v = GetMyVitality(help_i, G);
    int move = min(power, my_v - 100);
#ifndef USE_COMPILER
    CallWithValue(0, move);

    MaybePut(0);
    IToN(0, help_j);
    _(ATTACK, 0); // 0: attack(help_j)
    CallWithValue(0, 255); // 0: attack(help_j, 255)
#endif

    int mv = GetMyVitality(help_j, G);
    power = min(mv - 100, move);
#ifdef USE_COMPILER
    char expr[32];
    sprintf(expr, "help(%d)(%d)(%d)", help_i, help_j, move);
    run_expr(expr);
    sprintf(expr, "attack(0)(255)(%d)", power);
    run_expr(expr);
#else
    CallWithValue(0, power); // 0: attack(0, 255, move);
#endif

    help_i += 2;
  }
}

void Work() {
  KillZero();
  while (1) {
    DoWork();
    KillZero();
    //TryAllRevive();
    TryRevive();
  }
}


int main(int argc, char** argv) {
  assert(argc == 2);
  DEBUG = false;
  SLEEP_TIME = 0;
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
