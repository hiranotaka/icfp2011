#ifndef SIM_DEBUG_H
#define SIM_DEBUG_H

struct function;
struct value;
struct slot;
struct user;
struct game;

void print_function(const struct function *f);
void print_value(const struct value *value);
void print_slot(int user_index, int slot_index, const struct slot *slot);
void print_user(int user_index, const struct user *user);
void print_game(const struct game *game);

#endif
