#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sim.h"
#include "debug.h"

#define numberof(x) (sizeof(x) / sizeof(*(x)))

struct card {
	const char *name;
	struct value *value;
};

#define CARD(name) { #name, &name##_value }

static const struct card cards[] = {
	CARD(I), CARD(zero), CARD(succ), CARD(dbl), CARD(get), CARD(put),
	CARD(S), CARD(K), CARD(inc), CARD(dec), CARD(attack), CARD(help),
	CARD(copy), CARD(revive),
};

static struct value *find_card_value(const char *name) {
	int i;
	for (i = 0; i < numberof(cards); i++) {
		const struct card *card = &cards[i];
		if (!strcmp(card->name, name)) {
			return card->value;
		}
	}
	return NULL;
}

static void play_interactive_left(struct game *game) {
	char line[16];
	int slot_index;
	struct value *card_value;

	if (!fgets(line, sizeof(line), stdin))
		return;

	line[strlen(line) - 1] = 0;
	card_value = find_card_value(line);
	if (!card_value)
		return;

	if (!fgets(line, sizeof(line), stdin))
		return;

	slot_index = atoi(line);
	if (slot_index < 0 || slot_index >= 255)
		return;

	play_left(card_value, slot_index, game);
}

static void play_interactive_right(struct game *game) {
	char line[16];
	int slot_index;
	struct value *card_value;

	if (!fgets(line, sizeof(line), stdin))
		return;

	slot_index = atoi(line);
	if (slot_index < 0 || slot_index >= 255)
		return;

	if (!fgets(line, sizeof(line), stdin))
		return;
	line[strlen(line) - 1] = 0;
	card_value = find_card_value(line);
	if (!card_value)
		return;

	play_right(slot_index, card_value, game);
}

static void play_interactive(struct game *game) {
	char line[16];
	int dir;

	print_game(game);

	fgets(line, sizeof(line), stdin);
	dir = atoi(line);

	switch (dir) {
	case 1:
		play_interactive_left(game);
		break;
	case 2:
		play_interactive_right(game);
		break;
	}

	next_play(game);
}


int main(int argc, char *argv[]) {
	int i;
	struct game game;
	init_game(&game);
	for (i = 0; i < 200000; i++)
		play_interactive(&game);
	clean_game(&game);
	return 0;
}
