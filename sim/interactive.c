#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include "debug.h"

#define numberof(x) (sizeof(x) / sizeof(*(x)))

static struct value *read_card_value(void)
{
	char line[16];
	if (!fgets(line, sizeof(line), stdin))
		return NULL;

	line[strlen(line) - 1] = 0;
	return find_card_value(line);
}

static int read_slot_index(void)
{
	char line[16];
	int slot_index;

	if (!fgets(line, sizeof(line), stdin))
		return -1;

	slot_index = atoi(line);
	if (slot_index < 0 || slot_index >= 255)
		return -1;

	return slot_index;
}

static int apply_interactive_cs(struct game *game)
{
	int slot_index;
	struct value *card_value;

	card_value = read_card_value();
	if (!card_value)
		goto err;

	slot_index = read_slot_index();
	if (slot_index < 0)
		goto err;

	return apply_cs(card_value, slot_index, game);

 err:
	return switch_turn(game);
}

static int apply_interactive_sc(struct game *game)
{
	int slot_index;
	struct value *card_value;

	slot_index = read_slot_index();
	if (slot_index < 0)
		goto err;

	card_value = read_card_value();
	if (!card_value)
		goto err;

	return apply_sc(slot_index, card_value, game);

 err:
	return switch_turn(game);
}

static int apply_interactive(struct game *game)
{
	char line[16];
	int dir;

	print_game(game);

	fgets(line, sizeof(line), stdin);
	dir = atoi(line);

	switch (dir) {
	case 1:
		return apply_interactive_cs(game);
	case 2:
		return apply_interactive_sc(game);
	default:
		return switch_turn(game);
	}
}


int main(int argc, char *argv[])
{
	struct game *game = create_game();
	while (apply_interactive(game));
	destroy_game(game);
	return 0;
}
