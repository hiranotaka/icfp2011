#include <string.h>
#include <limits.h>
#include "../sim/sim.h"
#include "../sim/types.h"
#include "parser.h"
#include "compiler.h"

static int value_equals(const struct value *value1, const struct value *value2);

static int function_equals(const struct function *function1,
			   const struct function *function2)
{
	int i;
	if (function1->ops != function2->ops ||
	    function1->nr_args != function2->nr_args)
		return 0;

	for (i = 0; i < function1->nr_args; i++) {
		if (!value_equals(function1->args[i], function2->args[i]))
			return 0;
	}

	return 1;
}

static int value_equals(const struct value *value1, const struct value *value2)
{
	if (value1->type != value2->type)
		return 0;

	switch (value1->type) {
	case TYPE_INTEGER:
		return value1->u.integer == value2->u.integer;
	case TYPE_FUNCTION:
		return function_equals(&value1->u.function,
				       &value2->u.function);
	}
	return 0;
}

static int values_equal(struct value *const *values1,
			struct value *const *values2, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		if (!value_equals(values1[i], values2[i]))
			return 0;
	}
	return 1;
}

static inline void copy_result(const struct compile_result *source,
			       struct compile_result *target)
{
	memcpy(target, source, sizeof(struct compile_result));
}

static int compile_i_at_fast(int integer, const struct value *field,
			     struct compile_result *result)
{
	int nr_turns;

	if (field->type != TYPE_INTEGER)
		return 0;

	if (integer == field->u.integer)
		return 1;

	nr_turns = 0;
	while (integer > field->u.integer) {
		if (integer % 2) {
			integer--;
			nr_turns++;
			if (integer == field->u.integer) {
				if (!result->nr_turns) {
					result->first_method = METHOD_CS;
					result->first_card_name = "succ";
				}
				result->nr_turns += nr_turns;
				return 1;
			}
		}

		integer /= 2;
		nr_turns++;
		if (integer == field->u.integer) {
			if (!result->nr_turns) {
				result->first_method = METHOD_CS;
				result->first_card_name = "dbl";
			}
			result->nr_turns += nr_turns;
			return 1;
		}
	}

	return 0;
}

static void compile_i_at_slow(int integer, const struct value *field,
			      struct compile_result *result)
{
	if (field->type != TYPE_FUNCTION ||
	    strcmp(field->u.function.ops->name, "I")) {
		if (!result->nr_turns++) {
			result->first_method = METHOD_CS;
			result->first_card_name = "put";
		}
	}

	if (!result->nr_turns++) {
		result->first_method = METHOD_SC;
		result->first_card_name = "zero";
	}

	compile_i_at_fast(integer, &zero_value, result);
}

static void compile_i_at(int integer, const struct value *field,
			 struct compile_result *result)
{
	if (compile_i_at_fast(integer, field, result))
		return;

	compile_i_at_slow(integer, field, result);
}

static int compile_f_at_fast(const struct function *function,
			     const struct value *field,
			     struct compile_result *result)
{
	int i;

	if (field->type != TYPE_FUNCTION ||
	    field->u.function.ops != function->ops ||
	    field->u.function.nr_args > function->nr_args ||
	    !values_equal(field->u.function.args, function->args,
			 field->u.function.nr_args))
		return 0;

	for (i = field->u.function.nr_args; i < function->nr_args; i++) {
		if (!find_card_name(function->args[i]))
			return 0;
	}

	for (i = field->u.function.nr_args; i < function->nr_args; i++) {
		if (!result->nr_turns++) {
			result->first_method = METHOD_SC;
			result->first_card_name =
				find_card_name(function->args[i]);
		}
	}
	return 1;
}

static int get_compound_arg_index(struct function *function) {
	int i;
	for (i = function->nr_args - 1; i >= 0; i--) {
		if (!find_card_name(function->args[i]))
			return i;
	}
	return -1;
}

static void do_compile(struct value *value, struct compile_result *result,
		       const struct game *game);

void do_compile_at(struct value *value, const struct value *field,
		   struct compile_result *result, const struct game *game);

static int compile_f_at_slow_compound(struct function *function,
				      const struct value *field,
				      struct compile_result *result,
				      const struct game *game)
{
	struct value *value, *prev_value;
	int compound_arg_index, i, slot_index;

	compound_arg_index = get_compound_arg_index(function);
	if (compound_arg_index < 0)
		return 0;

	if (!compound_arg_index) {
		if (value_equals(field, function->args[0])) {
			if (!result->nr_turns++) {
				result->first_method = METHOD_CS;
				result->first_card_name = function->ops->name;
			}
			result->nr_turns += function->nr_args - 1;
		} else {
			do_compile_at(function->args[0], field, result, game);
			result->nr_turns += function->nr_args;
		}
		return 1;
	}

	value = create_value();
	value->type = TYPE_FUNCTION;
	value->u.function.ops = function->ops;
	value->u.function.nr_args = compound_arg_index;
	for (i = 0; i < compound_arg_index; i++) {
		value->u.function.args[i] = ref_value(function->args[i]);
	}

	do_compile(function->args[value->u.function.nr_args], result, game);
	slot_index = result->slot_index;

	prev_value = value;
	value = create_value();
	value->type = TYPE_FUNCTION;
	value->u.function.ops = K_value.u.function.ops;
	value->u.function.nr_args = 1;
	value->u.function.args[0] = prev_value;

	prev_value = value;
	value = create_value();
	value->type = TYPE_FUNCTION;
	value->u.function.ops = S_value.u.function.ops;
	value->u.function.nr_args = 2;
	value->u.function.args[0] = prev_value;
	value->u.function.args[1] = ref_value(&get_value);

	while (1) {
		if (slot_index % 2) {
			prev_value = value;
			value = create_value();
			value->type = TYPE_FUNCTION;
			value->u.function.ops = K_value.u.function.ops;
			value->u.function.nr_args = 1;
			value->u.function.args[0] = prev_value;

			prev_value = value;
			value = create_value();
			value->type = TYPE_FUNCTION;
			value->u.function.ops = S_value.u.function.ops;
			value->u.function.nr_args = 2;
			value->u.function.args[0] = prev_value;
			value->u.function.args[1] =
				ref_value(&succ_value);
			slot_index--;
		}

		if (!slot_index)
			break;

		prev_value = value;
		value = create_value();
		value->type = TYPE_FUNCTION;
		value->u.function.ops = K_value.u.function.ops;
		value->u.function.nr_args = 1;
		value->u.function.args[0] = prev_value;

		prev_value = value;
		value = create_value();
		value->type = TYPE_FUNCTION;
		value->u.function.ops = S_value.u.function.ops;
		value->u.function.nr_args = 2;
		value->u.function.args[0] = prev_value;
		value->u.function.args[1] = ref_value(&dbl_value);
		slot_index /= 2;
	}

	value->u.function.nr_args++;
	value->u.function.args[2] = ref_value(&zero_value);

	do_compile_at(value, field, result, game);
	unref_value(value);
	result->nr_turns += function->nr_args - compound_arg_index - 1;
	return 1;
}

static void compile_f_at_slow_simple(struct function *function,
				     const struct value *field,
				     struct compile_result *result)
{
	if (field->type != TYPE_FUNCTION ||
	    strcmp(field->u.function.ops->name, "I")) {
		if (!result->nr_turns++) {
			result->first_method = METHOD_CS;
			result->first_card_name = "put";
		}
	}

	if (!result->nr_turns++) {
		result->first_method = METHOD_SC;
		result->first_card_name = function->ops->name;
	}

	result->nr_turns += function->nr_args;
}

static void compile_f_at(struct function *function, const struct value *field,
			 struct compile_result *result, const struct game *game)
{
	if (compile_f_at_fast(function, field, result))
		return;

	if (compile_f_at_slow_compound(function, field, result, game))
		return;

	compile_f_at_slow_simple(function, field, result);
}

void do_compile_at(struct value *value, const struct value *field,
		   struct compile_result *result, const struct game *game)
{
	switch (value->type) {
	case TYPE_INTEGER:
		compile_i_at(value->u.integer, field, result);
		break;
	case TYPE_FUNCTION:
		compile_f_at(&value->u.function, field, result, game);
		break;
	}
}

static int compile_at(struct value *value, int slot_index,
		      struct compile_result *result, const struct game *game)
{
	int saved_slot_index;
	const struct slot *slot;

	if (is_slot_used(result, slot_index))
		return 0;
	set_slot_used(result, slot_index);

	slot = &game->users[game->turn].slots[slot_index];
	if (slot->vitality <= 0)
		return 0;

	saved_slot_index = result->first_slot_index;
	if (!result->nr_turns) {
		result->first_slot_index = slot_index;
	}
	do_compile_at(value, slot->field, result, game);
	if (!result->nr_turns)
		result->first_slot_index = saved_slot_index;
	result->slot_index = slot_index;
	return 1;
}

static void do_compile(struct value *value, struct compile_result *result,
		       const struct game *game)
{
	int i;
	struct compile_result slot_result, best_slot_result;

	best_slot_result.nr_turns = 2000000;
	for (i = 0; i < 256; i++) {
		copy_result(result, &slot_result);
		if (!compile_at(value, i, &slot_result, game))
			continue;
		if (slot_result.nr_turns < best_slot_result.nr_turns)
			copy_result(&slot_result, &best_slot_result);
	}

	copy_result(&best_slot_result, result);
}

void compile(struct value *value, struct compile_result *result,
	     const struct game *game)
{
	result->nr_turns = 0;
	memset(result->used_slots, 0, sizeof(result->used_slots));
	do_compile(value, result, game);
}
