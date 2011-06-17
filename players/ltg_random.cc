#include <iostream>
#include <string>

using namespace std;

const char* card_names[] = {
  "I", "zero", "succ", "dbl", "get", "put", "S", "K", "inc", "dec", "attack",
  "help", "copy", "revive", "zombie"
};

#define arraysize(a) (sizeof(a)/sizeof((a)[0]))

void Self() {
  int digit = rand() % 2 + 1;
  if (digit == 1) {
    cout << '1' << endl;
    cout << card_names[rand() % arraysize(card_names)] << endl;
    cout << 1 << endl;
  } else {
    cout << '2' << endl;
    cout << 1 << endl;
    cout << card_names[rand() % arraysize(card_names)] << endl;
  }
}

void Opp() {
  string dummy;
  for (int i = 0; i < 3; ++i) cin >> dummy;
}

int main(int argc, char** argv) {
  if (argc == 1) return 0;
  if (argv[1] == "1") Opp();
  while (1) {
    Self();
    Opp();
  }
  return 0;
}
