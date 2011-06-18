#include <iostream>

using namespace std;

void Self() {
  cout << "1" << endl;
  cout << "I" << endl;
  cout << "0" << endl;
}

void Opp() {
  string dummy;
  for (int i = 0; i < 3; ++i) cin >> dummy;
}

int main(int argc, char** argv) {
  if (argc == 1) return 0;
  if (argv[1][0] == '1') Opp();
  while (1) {
    Self();
    Opp();
  }
  return 0;
}
