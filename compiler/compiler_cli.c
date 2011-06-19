#include <stdio.h>
#include "parser.h"
#include "compiler.h"
#include "../sim/types.h"
#include "../sim/debug.h"
#include "../sim/sim.h"

int main(int argc, char *argv[]) {
	struct value *value;
	struct game *game;
	int i;
	struct compile_result result;

	if (argc != 2)
		return 1;

	if (!parse(argv[1], NULL, &value))
		return 1;

	print_value(value);
	printf("\n");

	game = create_game();
	for (i = 0; i < 50; i++) {
		compile(value, &result, game);
		struct value *value = find_card_value(result.first_card_name);
		if (value == NULL)
			goto err;

		switch (result.first_method) {
		case METHOD_CS:
			printf("apply %s %d\n", result.first_card_name,
			       result.first_slot_index);
			apply_cs(value, result.first_slot_index, game);
			break;
		case METHOD_SC:
			printf("apply %d %s\n", result.first_slot_index,
			       result.first_card_name);
			apply_sc(result.first_slot_index, value, game);
			break;
		}
		switch_turn(game);
		print_game(game);
		if (result.nr_turns <= 1)
			break;
	}

	destroy_game(game);
	unref_value(value);
	return 0;

 err:
	unref_value(value);
	destroy_game(game);
	return 1;
}
