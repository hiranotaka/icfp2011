#include "global.h"
#include "util.h"
#include <string>

using std::string;

const char* card_names[] = {
  "I", "zero", "succ", "dbl", "get", "put", "S", "K", "inc", "dec", "attack",
  "help", "copy", "revive", "zombie"
};

const char* GetCardName(Card card) {
  return card_names[card];
}

template<typename T, typename U>
void __(T lhs, U rhs, struct game* g);

template<>
void __(Card c, int i, struct game* g) {
  cout << 1 << endl;
  cout << GetCardName(c) << endl;
  cout << i << endl;
  struct value* v = find_card_value(GetCardName(c));
  apply_cs(v, i, g);
}

template<>
void __(int i, Card c, struct game* g) {
  cout << 2 << endl;
  cout << i << endl;
  cout << GetCardName(c) << endl;
  struct value* v = find_card_value(GetCardName(c));
  apply_sc(i, v, g);
}

void _(Card c, int i) {
  __(c, i, G);
  Opp();
}

void _(int i, Card c) {
  __(i, c, G);
  Opp();
}

template<>
void __(const char* c, int i, struct game* g) {
  cout << 1 << endl;
  cout << c << endl;
  cout << i << endl;
  struct value* v = find_card_value(c);
  apply_cs(v, i, g);
}

template<>
void __(int i, const char* c, struct game* g) {
  cout << 2 << endl;
  cout << i << endl;
  cout << c << endl;
  struct value* v = find_card_value(c);
  apply_sc(i, v, g);
}

void _(const char* c, int i) {
  __(c, i, G);
  Opp();
}

void _(int i, const char* c) {
  __(i, c, G);
  Opp();
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
  if (SLEEP_TIME) {
    sleep(SLEEP_TIME);
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


/*
struct Op {
  Card card;
  int slot;
  int type; // 0 card -> slot, 1 slot -> card
};


bool Make(struct value* v) {
}

bool Compile(const char* exp, vector<Op> ops) {
  struct value* v = Parse(exp);
  if (v->type == TYPE_INTEGER) {
    return false;
  }
  for (int i = 0; i < v->u.function.nr_args; ++i) {
    Make(v->u.function.args[i]);
    
  }
}
  
*/
  
