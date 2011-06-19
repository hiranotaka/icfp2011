#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../sim/sim.h"
#include "../sim/types.h"
#include "parser.h"

#define numberof(x) (sizeof(x) / sizeof(*(x)))

static int parse_integer(const char *expr, const char **endp,
			 struct value **valuep)
{
	struct value *value;
	value = create_value();
	value->type = TYPE_INTEGER;
	value->u.integer = strtol(expr, (char **)endp, 10);
	*valuep = value;
	return 1;
}

static int parse_function(const char *expr, const char **endp,
			  struct value **valuep)
{
	int namelen;
	char name[16];
	struct value *card_value, *value;
	const char* end;

	namelen = strcspn(expr, "()");
	if (namelen >= sizeof(name))
		return 0;

	memcpy(name, expr, namelen);
	name[namelen] = 0;

	card_value = find_card_value(name);
	if (!card_value || card_value->type != TYPE_FUNCTION)
		return 0;

	value = create_value();
	value->type = TYPE_FUNCTION;
	value->u.function.ops = card_value->u.function.ops;

	end = (char *)(expr + namelen);
	while (*end == '(') {
		if (value->u.function.nr_args >
		    numberof(value->u.function.args))
		    return 0;
		if (!parse(end + 1, &end,
			   &value->u.function.args[value->u.function.nr_args]))
			goto err;
		value->u.function.nr_args++;

		if (*end != ')')
			goto err;

		end++;
	}

	*valuep = value;
	if (endp)
		*endp = end;

	return 1;

 err:
	destroy_value(value);
	return 0;
}

int parse(const char *expr, const char **endp, struct value **value)
{
	if (isdigit(expr[0]))
		return parse_integer(expr, endp, value);
	else
		return parse_function(expr, endp, value);
}
