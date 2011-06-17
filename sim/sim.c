#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define numberof(x) (sizeof(x) / sizeof(*(x)))

enum type {
	TYPE_INTEGER, TYPE_FUNCTION,
};

struct function;
struct value;
struct game;

struct function_operations {
	int (*run)(struct function *f, struct value **retp, struct game *game);
	int nr_required_args;
};

struct function {
	const struct function_operations *ops;
	int nr_args;
	struct value *args[3];
};

struct value {
	enum type type;
	int refcount;
	union {
		int integer;
		struct function function;
	};
};

struct slot {
	struct value *field;
	int vitality;
};

struct user {
	struct slot slots[256];
};

struct game {
	struct user users[2];
	int turn;
	int nr_applications;
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

static int apply(struct value *f, struct value *x, struct value **retp,
		 struct game *game) {
	struct value *g;
	int i;

	if (++game->nr_applications > 1000)
		return 0;

	if (f->type != TYPE_FUNCTION)
		return 0;

	g = create_value();
	g->type = TYPE_FUNCTION;
	g->function.ops = f->function.ops;
	g->function.nr_args = f->function.nr_args;
	for (i = 0; i < numberof(f->function.args); i++)
		g->function.args[i] = ref_value(f->function.args[i]);
	g->function.args[g->function.nr_args++] = ref_value(x);
	if (g->function.nr_args < g->function.ops->nr_required_args) {
		*retp = g;
		return 1;
	}

	g->function.ops->run(&g->function, retp, game);
	unref_value(g);
	return 1;
}

static int is_slot_index(struct value *i) {
	return i->type == TYPE_INTEGER && i->integer >= 0 && i->integer < 256;
}

struct value_info {
	struct value *value;
	const char* name;
};

static struct value zero_value = {
	.type = TYPE_INTEGER,
	.refcount = 1,
	.integer = 0,
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


static int I(struct function *f, struct value **retp, struct game *game) {
	*retp = ref_value(f->args[0]);
	return 1;
}
DEFINE_FUNCTION(I, 1);

static int succ(struct function *f, struct value **retp, struct game *game) {
	struct value *n;
	struct value *ret;

	n = f->args[0];
	if (n->type != TYPE_INTEGER)
		return 0;

	ret = create_value();
	ret->type = TYPE_INTEGER;
	ret->integer = n->integer < 65535 ? n->integer + 1 : 65535;

	*retp = ret;
	return 1;
}
DEFINE_FUNCTION(succ, 1);

static int dbl(struct function *f, struct value **retp, struct game *game) {
	struct value *n;
	struct value *ret;

	n = f->args[0];
	if (n->type != TYPE_INTEGER)
		return 0;

	ret = create_value();
	ret->type = TYPE_INTEGER;
	ret->integer = n->integer < 32768 ? 2 * n->integer : 65535;

	*retp = ret;
	return 1;
}
DEFINE_FUNCTION(dbl, 1);

static int get(struct function *f, struct value **retp,
	       struct game *game) {
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[game->turn].slots[i->integer];
	if (slot->vitality < 0)
		return 0;

	*retp = ref_value(slot->field);
	return 1;
}
DEFINE_FUNCTION(get, 1);

static int put(struct function *f, struct value **retp,
	       struct game *game) {
	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(put, 1);

static int S(struct function *f, struct value **retp, struct game *game) {
	struct value *h, *y;
	int ret = 0;

	if (!apply(f->args[0], f->args[2], &h, game))
		goto err;

	if (!apply(f->args[1], f->args[2], &y, game))
		goto err_unref_value_h;

	if (!apply(h, y, retp, game))
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

static int K(struct function *f, struct value **retp, struct game *game) {
	*retp = ref_value(f->args[0]);
	return 1;
}
DEFINE_FUNCTION(K, 2);

static int inc(struct function *f, struct value **retp, struct game *game) {
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[game->turn].slots[i->integer];
	if (slot->vitality < 65535 || slot->vitality > 0)
		slot->vitality++;

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(inc, 1);

static int dec(struct function *f, struct value **retp, struct game *game) {
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[1 - game->turn].slots[255 - i->integer];
	if (slot->vitality > 0)
		slot->vitality--;

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(dec, 1);

static int attack(struct function *f, struct value **retp, struct game *game) {
	struct value *i, *j, *n;
	struct slot *slot0, *slot1;

	i = f->args[0];
	j = f->args[1];
	n = f->args[2];
	if (!is_slot_index(i) || !is_slot_index(j))
		return 0;

	slot0 = &game->users[game->turn].slots[i->integer];
	if (slot0->vitality < n->integer)
		return 0;

	slot0->vitality -= n->integer;

	slot1 = &game->users[1 - game->turn].slots[255 - j->integer];
	if (slot1->vitality > 0) {
		slot1->vitality -= n->integer * 9 / 10;
		if (slot1->vitality < 0) {
			slot1->vitality = 0;
		}
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(attack, 3);

static int help(struct function *f, struct value **retp, struct game *game) {
	struct value *i, *j, *n;
	struct slot *slot0, *slot1;

	i = f->args[0];
	j = f->args[1];
	n = f->args[2];
	if (!is_slot_index(i) || !is_slot_index(j))
		return 0;

	slot0 = &game->users[game->turn].slots[i->integer];
	if (slot0->vitality < n->integer)
		return 0;

	slot0->vitality -= n->integer;

	slot1 = &game->users[game->turn].slots[j->integer];
	if (slot1->vitality > 0) {
		slot1->vitality += n->integer * 11 / 10;
		if (slot1->vitality > 65535) {
			slot1->vitality = 65535;
		}
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(help, 3);

static int copy(struct function *f, struct value **retp,
		struct game *game) {
	struct value *i;

	i = f->args[0];
	if (is_slot_index(i))
		return 0;

	*retp = ref_value(game->users[game->turn].slots[i->integer].field);
	return 1;
}
DEFINE_FUNCTION(copy, 1);

static int revive(struct function *f, struct value **retp,
		  struct game *game) {
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[game->turn].slots[i->integer];
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

static void play_left(struct value *card_value, int slot_index,
		       struct game *game) {
	struct slot *slot;
	struct value *field;
	slot = &game->users[game->turn].slots[slot_index];
	field = slot->field;
	apply(card_value, field, &slot->field, game);
	unref_value(field);
}

static void play_right(int slot_index, struct value *card_value,
			struct game *game) {
	struct slot *slot;
	struct value *field;
	slot = &game->users[game->turn].slots[slot_index];
	field = slot->field;
	apply(field, card_value, &slot->field, game);
	unref_value(field);
}

static void next_play(struct game *game) {
	game->turn = 1 - game->turn;
	game->nr_applications = 0;
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

static const char *find_card_name(struct function *f) {
	int i;
	for (i = 0; i < numberof(cards); i++) {
		const struct card *card = &cards[i];
		struct value *value = card->value;
		if (value->type == TYPE_FUNCTION &&
		    value->function.ops == f->ops) {
			return card->name;
		}
	}
	return NULL;
}

static void print_function(struct function *f) {
	int i;
	fprintf(stderr, "%s", find_card_name(f));
	for (i = 0; i < f->nr_args; i++) {
		fprintf(stderr, "(");
		print_value(f->args[i]);
		fprintf(stderr, ")");
	}
}

static void print_value(struct value *value) {
	switch (value->type) {
	case TYPE_INTEGER:
		fprintf(stderr, "%d", value->integer);
		break;
	case TYPE_FUNCTION:
		print_function(&value->function);
		break;
	}
}

static void print_slot(int user_index, int slot_index, struct slot *slot) {
	if (slot->field != &I_value || slot->vitality != 10000) {
		fprintf(stderr, "user = %d / slot = %d / field: ", user_index,
			slot_index);
		print_value(slot->field);
		fprintf(stderr, ", vitality: %d\n", slot->vitality);
	}
}


static void print_user(int user_index, struct user *user) {
	int i;
	for (i = 0; i < 256; i++) {
		print_slot(user_index, i, &user->slots[i]);
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
	for (i = 0; i < 2; i++)
		init_user(&game->users[i]);
	game->turn = 0;
	game->nr_applications = 0;
}

int main(int argc, char *argv[]) {
	int i;
	struct game game;
	init_game(&game);
	for (i = 0; i < 100000; i++)
		play_interactive(&game);
	return 0;
}
