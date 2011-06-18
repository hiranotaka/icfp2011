#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sim.h"

#define numberof(x) (sizeof(x) / sizeof(*(x)))

static struct value *create_value(void)
{
	struct value *field = malloc(sizeof(struct value));
	field->refcount = 1;
	return field;
}

static inline struct value *ref_value(struct value *field)
{
	field->refcount++;
	return field;
}

static void destroy_value(struct value *field);

static inline void unref_value(struct value *field)
{
	if (--field->refcount <= 0)
		destroy_value(field);
}

static void destroy_value(struct value *field)
{
	if (field->type == TYPE_FUNCTION) {
		int i;
		for (i = 0; i < field->u.function.nr_args; i++)
			unref_value(field->u.function.args[i]);
	}
	free(field);
}

static int do_apply(struct value *f, struct value *x, struct value **retp,
		    struct game *game)
{
	struct value *g;
	int i;
	int completed;

	if (++game->nr_applications > 1000)
		return 0;

	if (f->type != TYPE_FUNCTION)
		return 0;

	g = create_value();
	g->type = TYPE_FUNCTION;
	g->u.function.ops = f->u.function.ops;
	g->u.function.nr_args = f->u.function.nr_args;
	for (i = 0; i < f->u.function.nr_args; i++)
		g->u.function.args[i] = ref_value(f->u.function.args[i]);
	g->u.function.args[g->u.function.nr_args++] = ref_value(x);
	if (g->u.function.nr_args < g->u.function.ops->nr_required_args) {
		*retp = g;
		return 1;
	}

	completed = g->u.function.ops->run(&g->u.function, retp, game);
	unref_value(g);
	return completed;
}

static int is_slot_index(struct value *i)
{
	return i->type == TYPE_INTEGER && i->u.integer >= 0 &&
		i->u.integer < 256;
}

struct value zero_value = {
	.type = TYPE_INTEGER,
	.refcount = 1,
	.u = {
		.integer = 0,
	},
};

#define DEFINE_FUNCTION(__name, __nr_required_args)		\
	static const struct function_operations			\
	__name##_function_operations = {			\
		.name = #__name,				\
		.run = __name,					\
		.nr_required_args = __nr_required_args,		\
	};							\
								\
	struct value __name##_value = {					\
		.type = TYPE_FUNCTION,					\
		.refcount = 1,						\
		.u = {							\
			.function = {					\
				.ops = &__name##_function_operations,	\
			}						\
		},							\
	}

static int I(struct function *f, struct value **retp, struct game *game)
{
	*retp = ref_value(f->args[0]);
	return 1;
}
DEFINE_FUNCTION(I, 1);

static int succ(struct function *f, struct value **retp, struct game *game)
{
	struct value *n;
	struct value *ret;

	n = f->args[0];
	if (n->type != TYPE_INTEGER)
		return 0;

	ret = create_value();
	ret->type = TYPE_INTEGER;
	ret->u.integer = n->u.integer < 65535 ? n->u.integer + 1 : 65535;

	*retp = ret;
	return 1;
}
DEFINE_FUNCTION(succ, 1);

static int dbl(struct function *f, struct value **retp, struct game *game)
{
	struct value *n;
	struct value *ret;

	n = f->args[0];
	if (n->type != TYPE_INTEGER)
		return 0;

	ret = create_value();
	ret->type = TYPE_INTEGER;
	ret->u.integer = n->u.integer < 32768 ? 2 * n->u.integer : 65535;

	*retp = ret;
	return 1;
}
DEFINE_FUNCTION(dbl, 1);

static int get(struct function *f, struct value **retp, struct game *game)
{
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[game->turn].slots[i->u.integer];
	if (slot->vitality < 0)
		return 0;

	*retp = ref_value(slot->field);
	return 1;
}
DEFINE_FUNCTION(get, 1);

static int put(struct function *f, struct value **retp, struct game *game)
{
	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(put, 1);

static int S(struct function *f, struct value **retp, struct game *game)
{
	struct value *h, *y;
	int ret = 0;

	if (!do_apply(f->args[0], f->args[2], &h, game))
		goto err;

	if (!do_apply(f->args[1], f->args[2], &y, game))
		goto err_unref_value_h;

	if (!do_apply(h, y, retp, game))
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

static int K(struct function *f, struct value **retp, struct game *game)
{
	*retp = ref_value(f->args[0]);
	return 1;
}
DEFINE_FUNCTION(K, 2);

static int inc(struct function *f, struct value **retp, struct game *game)
{
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[game->turn].slots[i->u.integer];
	if (!game->zombie) {
		if (slot->vitality < 65535 || slot->vitality > 0)
			slot->vitality++;
	} else {
		if (slot->vitality > 0)
			slot->vitality--;
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(inc, 1);

static int dec(struct function *f, struct value **retp, struct game *game)
{
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[1 - game->turn].slots[255 - i->u.integer];
	if (!game->zombie) {
		if (slot->vitality > 0)
			slot->vitality--;
	} else {
		if (slot->vitality < 65535 || slot->vitality > 0)
			slot->vitality++;
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(dec, 1);

static int attack(struct function *f, struct value **retp, struct game *game)
{
	struct value *i, *j, *n;
	struct slot *slot0, *slot1;

	i = f->args[0];
	j = f->args[1];
	n = f->args[2];
	if (!is_slot_index(i) || !is_slot_index(j))
		return 0;

	slot0 = &game->users[game->turn].slots[i->u.integer];
	if (slot0->vitality < n->u.integer)
		return 0;

	slot0->vitality -= n->u.integer;

	slot1 = &game->users[1 - game->turn].slots[255 - j->u.integer];
	if (!game->zombie) {
		if (slot1->vitality > 0) {
			slot1->vitality -= n->u.integer * 9 / 10;
			if (slot1->vitality < 0)
				slot1->vitality = 0;
		}
	} else {
		if (slot1->vitality > 0) {
			slot1->vitality += n->u.integer * 9 / 10;
			if (slot1->vitality > 65535)
				slot1->vitality = 65535;
		}
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(attack, 3);

static int help(struct function *f, struct value **retp, struct game *game)
{
	struct value *i, *j, *n;
	struct slot *slot0, *slot1;

	i = f->args[0];
	j = f->args[1];
	n = f->args[2];
	if (!is_slot_index(i) || !is_slot_index(j))
		return 0;

	slot0 = &game->users[game->turn].slots[i->u.integer];
	if (slot0->vitality < n->u.integer)
		return 0;

	slot0->vitality -= n->u.integer;

	slot1 = &game->users[game->turn].slots[j->u.integer];
	if (!game->zombie) {
		if (slot1->vitality > 0) {
			slot1->vitality += n->u.integer * 11 / 10;
			if (slot1->vitality > 65535)
				slot1->vitality = 65535;
		}
	} else {
		if (slot1->vitality > 0) {
			slot1->vitality -= n->u.integer * 11 / 10;
			if (slot1->vitality > 65535)
				slot1->vitality = 65535;
		}
	}

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(help, 3);

static int copy(struct function *f, struct value **retp, struct game *game)
{
	struct value *i;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	*retp = ref_value(game->users[1 - game->turn].slots[i->u.integer].
			  field);
	return 1;
}
DEFINE_FUNCTION(copy, 1);

static int revive(struct function *f, struct value **retp, struct game *game)
{
	struct value *i;
	struct slot *slot;

	i = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[game->turn].slots[i->u.integer];
	if (slot->vitality <= 0)
		slot->vitality = 1;

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(revive, 1);

static int zombie(struct function *f, struct value **retp, struct game *game)
{
	struct value *i, *x;
	struct slot *slot;

	i = f->args[0];
	x = f->args[0];
	if (!is_slot_index(i))
		return 0;

	slot = &game->users[1 - game->turn].slots[255 - i->u.integer];
	if (slot->vitality > 0)
		return 0;

	unref_value(slot->field);
	slot->field = ref_value(x);
	slot->vitality = -1;

	*retp = ref_value(&I_value);
	return 1;
}
DEFINE_FUNCTION(zombie, 1);

struct card {
	const char *name;
	struct value *value;
};

#define CARD(name) { #name, &name##_value }

static const struct card cards[] = {
	CARD(I), CARD(zero), CARD(succ), CARD(dbl), CARD(get), CARD(put),
	CARD(S), CARD(K), CARD(inc), CARD(dec), CARD(attack), CARD(help),
	CARD(copy), CARD(revive), CARD(zombie),
};

struct value *find_card_value(const char *name)
{
	int i;
	for (i = 0; i < numberof(cards); i++) {
		const struct card *card = &cards[i];
		if (!strcmp(card->name, name)) {
			return card->value;
		}
	}
	return NULL;
}

const char *find_card_name(const struct value *value)
{
	int i;
	for (i = 0; i < numberof(cards); i++) {
		const struct card *card = &cards[i];
		if (card->value == value) {
			return card->name;
		}
	}
	return NULL;
}

static int apply(struct value *f, struct value *x, struct value **retp,
		 struct game *game)
{
	game->nr_applications = 0;
	return do_apply(f, x, retp, game);
}

static int user_dead(struct user *user)
{
	int i;
	for (i = 0; i < 256; i++) {
		if (user->slots[i].vitality > 0)
			return 0;
	}
	return 1;
}

static int game_dead(struct game *game)
{
	int i;
	for (i = 0; i < 2; i++) {
		if (user_dead(&game->users[i]))
			return 1;
	}
	return 0;
}

static void apply_zombie(struct slot *slot, struct game *game)
{
	if (slot->vitality > 0)
		return;

	struct value *ret;
	if (apply(slot->field, &I_value, &ret, game))
		unref_value(ret);

	slot->field = ref_value(&I_value);
	slot->vitality = 0;
}

static void apply_zombies(struct game *game)
{
	struct user *user;
	int i;

	game->zombie = 1;
	user = &game->users[game->turn];
	for (i = 0; i < 256; i++) {
		apply_zombie(&user->slots[i], game);
	}
	game->zombie = 0;
}

int switch_turn(struct game *game)
{
	if (game_dead(game))
		return 0;

	if (++game->nr_turns >= 2000000)
		return 0;

	game->turn = 1 - game->turn;

	apply_zombies(game);
	return 1;
}

int apply_cs(struct value *card_value, int slot_index, struct game *game)
{
	struct slot *slot;
	struct value *field;
	slot = &game->users[game->turn].slots[slot_index];
	if (slot->vitality <= 0)
		goto err;
	field = slot->field;
	if (!apply(card_value, field, &slot->field, game))
		goto err;
	unref_value(field);
	return switch_turn(game);
 err:
	slot->field = ref_value(&I_value);
	return switch_turn(game);
}

int apply_sc(int slot_index, struct value *card_value, struct game *game)
{
	struct slot *slot;
	struct value *field;
	slot = &game->users[game->turn].slots[slot_index];
	if (slot->vitality <= 0)
		goto err;
	field = slot->field;
	if (!apply(field, card_value, &slot->field, game))
		goto err;
	unref_value(field);
	return switch_turn(game);
 err:
	slot->field = ref_value(&I_value);
	return switch_turn(game);
}

static void init_slot(struct slot *slot)
{
	slot->field = ref_value(&I_value);
	slot->vitality = 10000;
}

static void init_user(struct user *user)
{
	int i;
	for (i = 0; i < 256; i++)
		init_slot(&user->slots[i]);
}

struct game *create_game(void)
{
	struct game *game;
	int i;
	game = malloc(sizeof(struct game));
	for (i = 0; i < 2; i++)
		init_user(&game->users[i]);
	game->nr_turns = 0;
	game->turn = 0;
	return game;
}

static void copy_slot(struct slot *old_slot, struct slot *new_slot)
{
	new_slot->field = ref_value(old_slot->field);
	new_slot->vitality = old_slot->vitality;
}

static void copy_user(struct user *old_user, struct user *new_user)
{
	int i;
	for (i = 0; i < 256; i++)
		copy_slot(&old_user->slots[i], &new_user->slots[i]);
}

struct game *dup_game(struct game* old_game)
{
	struct game *new_game;
	int i;
	new_game = malloc(sizeof(struct game));
	for (i = 0; i < 2; i++)
		copy_user(&old_game->users[i], &new_game->users[i]);
	new_game->nr_turns = old_game->nr_turns;
	new_game->turn = old_game->turn;
	new_game->nr_applications = old_game->nr_applications;
	return new_game;
}

static void clean_slot(struct slot *slot)
{
	unref_value(slot->field);
}

static void clean_user(struct user *user)
{
	int i;
	for (i = 0; i < 256; i++)
		clean_slot(&user->slots[i]);
}

void destroy_game(struct game *game)
{
	int i;
	for (i = 0; i < 2; i++)
		clean_user(&game->users[i]);
	free(game);
}
