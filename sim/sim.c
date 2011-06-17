#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define numberof(x) (sizeof(x) / sizeof(*(x)))

enum type {
	TYPE_NUMBER, TYPE_FUNCTION,
};

struct function;
struct value;
struct game;

struct function_operations {
	int (*run)(struct function *function, struct value **retp,
		   struct game *game);
	int nr_required_args;
};

struct function {
	const struct function_operations *ops;
	int nr_args;
	struct value* args[3];
};

struct value {
	enum type type;
	int refcount;
	union {
		int number;
		struct function function;
	};
};

struct slot {
	struct value* field;
	int vitality;
};

struct user {
	struct slot slots[256];
};

struct game {
	struct user users[2];
	int turn;
};

static inline struct value *create_value(void) {
	struct value *field = malloc(sizeof(struct value));
	field->refcount = 1;
	return field;
}

static inline struct value *ref_value(struct value *field) {
	if (field)
		field->refcount++;
	return field;
}

static void unref_value(struct value *field);

static inline void destroy_value(struct value *field) {
	if (field->type == TYPE_FUNCTION) {
		int i;
		for (i = 0; i < numberof(field->function.args); i++)
			unref_value(field->function.args[i]);
	}
	free(field);
}

static inline void unref_value(struct value *field) {
	if (field && --field->refcount <= 0)
		destroy_value(field);
}

static void print_value(struct value *value);

static int run_function(struct value *f, struct value *x,
			struct value **retp, struct game *game) {
	int i;
	struct value *ret;

	if (f->type != TYPE_FUNCTION)
		return 0;

	ret = create_value();
	ret->type = TYPE_FUNCTION;
	ret->function.ops = f->function.ops;
	ret->function.nr_args = f->function.nr_args + 1;
	for (i = 0; i < numberof(f->function.args); i++) {
		ret->function.args[i] = ref_value(f->function.args[i]);
	}
	ret->function.args[f->function.nr_args] = ref_value(x);

	if (ret->function.nr_args >= ret->function.ops->nr_required_args) {
		ret->function.ops->run(&ret->function, retp, game);
		unref_value(ret);
		return 1;
	}

	*retp = ret;
	return 1;
}

static int is_slot_number(struct value *i) {
	return i->type == TYPE_NUMBER && i->number >= 0 && i->number < 256;
}

static struct value zero_value = {
	.type = TYPE_NUMBER,
	.refcount = 1,
	.number = 0,
};

#define DEFINE_FUNCTION(name, nr_args)				\
	static const struct function_operations name##_function_operations = { \
		.run = name,						\
		.nr_required_args = nr_args,				\
	};								\
									\
	static struct value name##_value = {				\
		.type = TYPE_FUNCTION,					\
		.refcount = 1,						\
		.function = {						\
			.ops = &name##_function_operations,		\
		},							\
	};


static int I(struct function *function, struct value **retp,
	      struct game *game) {
	*retp = ref_value(function->args[0]);
	return 1;
}
DEFINE_FUNCTION(I, 1);

static int succ(struct function *function, struct value **retp,
		struct game *game) {
	struct value *n;
	struct value *ret;

	n = function->args[0];
	if (n->type != TYPE_NUMBER)
		return 0;

	ret = create_value();
	ret->type = TYPE_NUMBER;
	ret->number = n->number < 65535 ? n->number + 1 : 65535;

	*retp = ret;
	return 1;
}
DEFINE_FUNCTION(succ, 1);

static int dbl(struct function *function, struct value **retp,
	       struct game *game) {
	struct value *n;
	struct value *ret;

	n = function->args[0];
	if (n->type != TYPE_NUMBER)
		return 0;

	ret = create_value();
	ret->type = TYPE_NUMBER;
	ret->number = n->number < 32768 ? 2 * n->number : 65535;

	*retp = ret;
	return 1;
}
DEFINE_FUNCTION(dbl, 1);

static int get(struct function *function, struct value **retp,
	       struct game *game) {
	struct value *i;
	struct slot *slot;

	i = function->args[0];
	if (!is_slot_number(i))
		return 0;

	slot = &game->users[game->turn].slots[i->number];
	if (slot->vitality < 0)
		return 0;

	*retp = ref_value(slot->field);
	return 1;
}
DEFINE_FUNCTION(get, 1);

static int put(struct function *function, struct value **retp,
	       struct game *game) {
	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(put, 1);

static int S(struct function *function, struct value **retp,
	     struct game *game) {
	struct value *h, *y;
	int ret = 0;

	if (!run_function(function->args[0], function->args[2], &h, game))
		goto err;

	if (!run_function(function->args[1], function->args[2], &y, game))
		goto err_unref_value_h;

	if (!run_function(h, y, retp, game))
		goto err_unref_value_y;

	ret = 1;
 err_unref_value_y:
	unref_value(y);
 err_unref_value_h:
	unref_value(h);
 err:
	return ret;
}
DEFINE_FUNCTION(S, 3);

static int K(struct function *function, struct value **retp,
	     struct game *game) {
	*retp = ref_value(function->args[0]);
	return 1;
}
DEFINE_FUNCTION(K, 2);

static int inc(struct function *function, struct value **retp,
	       struct game *game) {
	struct value *i;
	struct slot *slot;

	i = function->args[0];
	if (!is_slot_number(i))
		return 0;

	slot = &game->users[game->turn].slots[i->number];
	if (slot->vitality < 65535 || slot->vitality > 0)
		slot->vitality++;

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(inc, 1);

static int dec(struct function *function, struct value **retp,
	       struct game *game) {
	struct value *i;
	struct slot *slot;

	i = function->args[0];
	if (!is_slot_number(i))
		return 0;

	slot = &game->users[1 - game->turn].slots[255 - i->number];
	if (slot->vitality > 0)
		slot->vitality--;

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(dec, 1);

static int attack(struct function *function, struct value **retp,
		  struct game *game) {
	struct value *i, *j, *n;
	struct slot *slot0, *slot1;

	i = function->args[0];
	j = function->args[1];
	n = function->args[2];
	if (!is_slot_number(i) || !is_slot_number(j))
		return 0;

	slot0 = &game->users[game->turn].slots[i->number];
	if (slot0->vitality < n->number)
		return 0;

	slot0->vitality -= n->number;

	slot1 = &game->users[1 - game->turn].slots[255 - j->number];
	if (slot1->vitality > 0) {
		slot1->vitality -= n->number * 9 / 10;
		if (slot1->vitality < 0) {
			slot1->vitality = 0;
		}
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(attack, 3);

static int help(struct function *function, struct value **retp,
		struct game *game) {
	struct value *i, *j, *n;
	struct slot *slot0, *slot1;

	i = function->args[0];
	j = function->args[1];
	n = function->args[2];
	if (!is_slot_number(i) || !is_slot_number(j))
		return 0;

	slot0 = &game->users[game->turn].slots[i->number];
	if (slot0->vitality < n->number)
		return 0;

	slot0->vitality -= n->number;

	slot1 = &game->users[game->turn].slots[j->number];
	if (slot1->vitality > 0) {
		slot1->vitality += n->number * 11 / 10;
		if (slot1->vitality > 65535) {
			slot1->vitality = 65535;
		}
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(help, 3);

static int copy(struct function *function, struct value **retp,
		struct game *game) {
	struct value *i;

	i = function->args[0];
	if (is_slot_number(i))
		return 0;

	*retp = ref_value(game->users[game->turn].slots[i->number].field);
	return 1;
}
DEFINE_FUNCTION(copy, 1);

static int revive(struct function *function, struct value **retp,
		  struct game *game) {
	struct value *i;
	struct slot *slot;

	i = function->args[0];
	if (!is_slot_number(i))
		return 0;

	slot = &game->users[game->turn].slots[i->number];
	if (slot->vitality <= 0)
		slot->vitality = 1;

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(revive, 1);

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

static void run_left(struct game *game) {
	char line[16];
	int slot_number;
	struct value *card_value, *slot_value;

	if (!fgets(line, sizeof(line), stdin))
		return;

	line[strlen(line) - 1] = 0;
	card_value = find_card_value(line);
	if (!card_value)
		return;

	if (!fgets(line, sizeof(line), stdin))
		return;

	slot_number = atoi(line);
	if (slot_number < 0 || slot_number >= 255)
		return;

	slot_value = game->users[game->turn].slots[slot_number].field;
	run_function(card_value, slot_value,
		     &game->users[game->turn].slots[slot_number].field,
		     game);
	unref_value(slot_value);
}

static void run_right(struct game *game) {
	char line[16];
	int slot_number;
	struct value *card_value, *slot_value;

	if (!fgets(line, sizeof(line), stdin))
		return;

	slot_number = atoi(line);
	if (slot_number < 0 || slot_number >= 255)
		return;

	if (!fgets(line, sizeof(line), stdin))
		return;
	line[strlen(line) - 1] = 0;
	card_value = find_card_value(line);
	if (!card_value)
		return;

	slot_value = game->users[game->turn].slots[slot_number].field;
	run_function(slot_value,
		     card_value,
		     &game->users[game->turn].slots[slot_number].field,
		     game);
	unref_value(slot_value);
}

static const char *find_card_name(struct function *function) {
	int i;
	for (i = 0; i < numberof(cards); i++) {
		const struct card *card = &cards[i];
		struct value *value = card->value;
		if (value->type == TYPE_FUNCTION &&
		    value->function.ops == function->ops) {
			return card->name;
		}
	}
	return NULL;
}

static void print_function(struct function *function) {
	int i;
	fprintf(stderr, "%s", find_card_name(function));
	for (i = 0; i < function->nr_args; i++) {
		fprintf(stderr, "(");
		print_value(function->args[i]);
		fprintf(stderr, ")");
	}
}

static void print_value(struct value *value) {
	switch (value->type) {
	case TYPE_NUMBER:
		fprintf(stderr, "%d", value->number);
		break;
	case TYPE_FUNCTION:
		print_function(&value->function);
		break;
	}
}

static void print_slot(int user_number, int slot_number, struct slot *slot) {
	if (slot->field != &I_value || slot->vitality != 10000) {
		fprintf(stderr, "user = %d / slot = %d / field: ", user_number,
			slot_number);
		print_value(slot->field);
		fprintf(stderr, ", vitality: %d\n", slot->vitality);
	}
}


static void print_user(int user_number, struct user *user) {
	int i;
	for (i = 0; i < 256; i++) {
		print_slot(user_number, i, &user->slots[i]);
	}
}

static void print_game(struct game *game) {
	int i;
	fprintf(stderr, "=== PRIN GAME BEGIN ===\n");
	fprintf(stderr, "turn: %d\n", game->turn);
	for (i = 0; i < 2; i++) {
		print_user(i, &game->users[i]);
	}
	fprintf(stderr, "=== PRINT GAME END ===\n");
}

static void run(struct game *game) {
	char line[16];
	int dir;

	print_game(game);

	fgets(line, sizeof(line), stdin);
	dir = atoi(line);

	switch (dir) {
	case 1:
		run_left(game);
		break;
	case 2:
		run_right(game);
		break;
	}

	game->turn = 1 - game->turn;
}

static void init_slot(struct slot *slot) {
	slot->field = ref_value(&I_value);
	slot->vitality = 10000;
}

static void init_user(struct user *user) {
	int i;
	for (i = 0; i < 256; i++)
		init_slot(&user->slots[i]);
}

static void init_game(struct game *game) {
	int i;
	game->turn = 0;
	for (i = 0; i < 2; i++)
		init_user(&game->users[i]);
}

int main(int argc, char *argv[]) {
	int i;
	struct game game;
	init_game(&game);
	for (i = 0; i < 1000; i++)
		run(&game);
	return 0;
}
