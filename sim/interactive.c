#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sim.h"
#include "debug.h"

#define numberof(x) (sizeof(x) / sizeof(*(x)))

static struct value *read_card_value(void) {
	char line[16];
	if (!fgets(line, sizeof(line), stdin))
		return NULL;

	line[strlen(line) - 1] = 0;
	return find_card_value(line);
}

static int read_slot_index(void) {
	char line[16];
	int slot_index;

	if (!fgets(line, sizeof(line), stdin))
		return -1;

	slot_index = atoi(line);
	if (slot_index < 0 || slot_index >= 255)
		return -1;

	return slot_index;
}

static void play_interactive_left(struct game *game) {
	int slot_index;
	struct value *card_value;

	card_value = read_card_value();
	if (!card_value)
		return;

	slot_index = read_slot_index();
	if (slot_index < 0)
		return;

	play_left(card_value, slot_index, game);
}

static void play_interactive_right(struct game *game) {
	int slot_index;
	struct value *card_value;

	slot_index = read_slot_index();
	if (slot_index < 0)
		return;

	card_value = read_card_value();
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
