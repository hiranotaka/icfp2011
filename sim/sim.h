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

void play_left(struct value *card, int slot_index, struct game *game);
void play_right(int slot_index, struct value *card, struct game *game);
void next_play(struct game *game);

void init_game(struct game *game);
void clean_game(struct game *game);

#endif
