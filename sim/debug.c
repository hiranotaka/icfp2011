#include <stdio.h>
#include "types.h"
#include "sim.h"
#include "debug.h"

void print_function(const struct function *f) {
	int i;
	fprintf(stderr, "%s", f->ops->name);
	for (i = 0; i < f->nr_args; i++) {
		fprintf(stderr, "(");
		print_value(f->args[i]);
		fprintf(stderr, ")");
	}
}

void print_value(const struct value *value) {
	switch (value->type) {
	case TYPE_INTEGER:
		fprintf(stderr, "%d", value->u.integer);
		break;
	case TYPE_FUNCTION:
		print_function(&value->u.function);
		break;
	}
}

void print_slot(int user_index, int slot_index, const struct slot *slot) {
	if (slot->field != &I_value || slot->vitality != 10000) {
		fprintf(stderr, "user = %d / slot = %d / field: ", user_index,
			slot_index);
		print_value(slot->field);
		fprintf(stderr, ", vitality: %d\n", slot->vitality);
	}
}


void print_user(int user_index, const struct user *user) {
	int i;
	for (i = 0; i < 256; i++) {
		print_slot(user_index, i, &user->slots[i]);
	}
}

void print_game(const struct game *game) {
	int i;
	fprintf(stderr, "=== PRIN GAME BEGIN ===\n");
	fprintf(stderr, "turn: %d\n", game->turn);
	for (i = 0; i < 2; i++) {
		print_user(i, &game->users[i]);
	}
	fprintf(stderr, "=== PRINT GAME END ===\n");
}
