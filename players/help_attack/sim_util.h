#ifndef SIM_UTIL_H
#define SIM_UTIL_H

extern "C" {
#include "../../sim/sim.h"
#include "../../sim/types.h"
#include "../../sim/debug.h"
}

int GetOppVitality(int i, struct game* g);
int GetMyVitality(int i, struct game* g);

const int INVALID_VALUE = -100000;

int GetMyValue(int i, struct game* g);
int GetOppValue(int i, struct game* g);

struct function* GetMyFunc(int i, struct game* g);
struct function* GetOppFunc(int i, struct game* g);

#endif
