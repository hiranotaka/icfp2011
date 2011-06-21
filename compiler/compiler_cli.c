#include <stdio.h>
#include "parser.h"
#include "compiler.h"
#include "../sim/types.h"
#include "../sim/debug.h"
#include "../sim/sim.h"

int run_expr(const char *expr, struct game *game) {
        struct value *value;
	int i;
	struct compile_result result;

	if (!parse(expr, NULL, &value))
		return 0;

	for (i = 0; i < 50; i++) {
		compile(value, &result, game);
		if (!result.nr_turns)
			break;

		struct value *value = find_card_value(result.first_card_name);
		if (value == NULL)
			goto err;

		switch (result.first_method) {
		case METHOD_CS:
			printf("apply %s %d ", result.first_card_name,
			       result.first_slot_index);
			apply_cs(value, result.first_slot_index, game);
			break;
		case METHOD_SC:
			printf("apply %d %s ", result.first_slot_index,
			       result.first_card_name);
			apply_sc(result.first_slot_index, value, game);
			break;
		}
		printf("(turns left: %d)\n", result.nr_turns);
		switch_turn(game);
		if (result.nr_turns <= 1)
			break;
	}
        print_game(game);
	unref_value(value);
	return 1;
err:
	unref_value(value);
	return 0;
}

int main(int argc, char *argv[]) {
	struct game *game;
	int i;

	game = create_game();
	for (i = 1; i < argc; i++) {
		if (!run_expr(argv[i], game))
			goto err;
	}
	destroy_game(game);
	return 0;

 err:
	destroy_game(game);
	return 1;
}
