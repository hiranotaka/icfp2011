#ifndef SIM_SIM_H
#define SIM_SIM_H

struct value;
struct game;

extern struct value I_value;
extern struct value zero_value;
extern struct value succ_value;
extern struct value dbl_value;
extern struct value get_value;
extern struct value put_value;
extern struct value S_value;
extern struct value K_value;
extern struct value inc_value;
extern struct value dec_value;
extern struct value attack_value;
extern struct value help_value;
extern struct value copy_value;
extern struct value revive_value;
struct value *find_card_value(const char *name);
const char *find_card_name(const struct value *value);  // Not tested.

void play_left(struct value *card_value, int slot_index, struct game *game);
void play_right(int slot_index, struct value *card_value, struct game *game);
void next_play(struct game *game);

struct game *create_game(void);
struct game *dup_game(struct game *game);  // Not tested.
void destroy_game(struct game *game);

#endif
