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
  cout << card_names[c] << endl;
  cout << i << endl;
}

template<>
void __(int i, Card c) {
  cout << 2 << endl;
  cout << i << endl;
  cout << card_names[c] << endl;
}

template<typename T, typename U>
void _(T lhs, U rhs) {
  __(lhs, rhs);
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
  _(i, ZERO);  // i: F(succ(dbl(succ(...(zero)...)))) -> F(j)
}

void DoWork() {
  int base = rand() % 256;
  for (int i = 0; i < 256; ++i) {
    _(base, INC);
    CallWithValue(base, i);
  }
}

void Work() {
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
