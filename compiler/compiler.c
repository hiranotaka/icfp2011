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

static void do_compile(struct value *value, struct compile_result *result,
		       struct game *game);

static void relax_function(struct function *function,
			   struct value **relaxed_valuep,
			   struct compile_result *result, struct game *game)
{
	struct value *value, *prev_value;
	int i, slot_index;

	value = create_value();
	value->type = TYPE_FUNCTION;
	value->u.function.ops = function->ops;
	value->u.function.nr_args = 0;
	if (function->nr_args >= 1)
		value->u.function.args[value->u.function.nr_args++] =
			ref_value(function->args[0]);

	for (i = 1; i < function->nr_args; i++) {
		do_compile(function->args[i], result, game);
		slot_index = result->slot_index;
		set_slot_used(result, slot_index);

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
	}

	*relaxed_valuep = value;
}

static void relax_value(struct value *value, struct value **relax_valuep,
			struct compile_result *result, struct game *game)
{
	switch (value->type) {
	case TYPE_INTEGER:
		*relax_valuep = ref_value(value);
		return;
	case TYPE_FUNCTION:
		relax_function(&value->u.function, relax_valuep, result, game);
		return;
	default:
		*relax_valuep = NULL;
		return;
	}
}

static void do_compile_at(const struct value *value, const struct value *field,
			  struct compile_result *result);

static void compile_f_at(const struct function *function,
			 const struct value *field,
			 struct compile_result *result)
{
	int i;
	if (field->type == TYPE_FUNCTION &&
	    field->u.function.ops == function->ops &&
	    (field->u.function.nr_args > 0 || function->nr_args <= 0) &&
	    field->u.function.nr_args <= function->nr_args &&
	    values_equal(field->u.function.args, function->args,
			 field->u.function.nr_args)) {
		for (i = field->u.function.nr_args; i < function->nr_args;
		     i++) {
			result->first_method = METHOD_SC;
			result->first_card_name =
				find_card_name(function->args[i]);
		}
	} else if (function->nr_args > 0 &&
		   value_equals(field, function->args[0])) {
		if (!result->nr_turns++) {
			result->first_method = METHOD_CS;
			result->first_card_name = function->ops->name;
		}
		result->nr_turns += function->nr_args - 1;
	} else if (function->nr_args > 0) {
		do_compile_at(function->args[0], field, result);
		result->nr_turns += function->nr_args;
	} else {
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
	}
}

void do_compile_at(const struct value *value, const struct value *field,
		   struct compile_result *result)
{
	switch (value->type) {
	case TYPE_INTEGER:
		compile_i_at(value->u.integer, field, result);
		break;
	case TYPE_FUNCTION:
		compile_f_at(&value->u.function, field, result);
		break;
	}
}

static void compile_at(const struct value *value, int slot_index,
		       struct compile_result *result, struct game *game)
{
	struct slot *slot;
	slot = &game->users[game->turn].slots[slot_index];
	if (!result->nr_turns)
		result->first_slot_index = slot_index;
	do_compile_at(value, slot->field, result);
	result->slot_index = slot_index;
}

static void do_compile(struct value *value, struct compile_result *result,
		       struct game *game)
{
	int i;
	struct value *relaxed_value;
	struct compile_result slot_result, best_slot_result;

	relax_value(value, &relaxed_value, result, game);

	best_slot_result.nr_turns = INT_MAX;
	for (i = 0; i < 256; i++) {
		if (is_slot_used(result, i))
			continue;

		copy_result(result, &slot_result);
		compile_at(relaxed_value, i, &slot_result, game);
		if (slot_result.nr_turns < best_slot_result.nr_turns)
			copy_result(&slot_result, &best_slot_result);
	}

	copy_result(&best_slot_result, result);
	unref_value(relaxed_value);
}

void compile(struct value *value, struct compile_result *result,
	     struct game *game)
{
	result->nr_turns = 0;
	memset(result->used_slots, 0, sizeof(result->used_slots));
	do_compile(value, result, game);
}
