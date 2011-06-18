#include "global.h"
#include "sim_util.h"

int GetVitality(int i, int player, struct game* g) {
  return g->users[player].slots[i].vitality;
}

int GetOppVitality(int i, struct game* g) {
  return GetVitality(i, MY_PLAYER == 0 ? 1 : 0, g);
}

int GetMyVitality(int i, struct game* g) {
  return GetVitality(i, MY_PLAYER, g);
}

int GetValue(int i, int player, struct game* g) {
  if (GetVitality(i, player, g) <= 0)
    return INVALID_VALUE;
  struct value* v = g->users[player].slots[i].field;
  if (v->type == TYPE_INTEGER)
    return v->u.integer;
  else
    return INVALID_VALUE;
}

int GetMyValue(int i, struct game* g) {
  return GetValue(i, MY_PLAYER, g);
}

int GetOppValue(int i, struct game* g) {
  return GetValue(i, MY_PLAYER ? 0 : 1, g);
}

struct function* GetFunc(int i, int player, struct game* g) {
  struct value* v = g->users[player].slots[i].field;
  if (v->type == TYPE_FUNCTION)
    return &(v->u.function);
  return 0;
}

struct function* GetMyFunc(int i, struct game* g) {
  return GetFunc(i, MY_PLAYER, g);
}

struct function* GetOppFunc(int i, struct game* g) {
  return GetFunc(i, MY_PLAYER ? 0 : 1, g);
}
