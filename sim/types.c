#include <stdlib.h>
#include "types.h"

struct value *create_value(void)
{
	struct value *field = malloc(sizeof(struct value));
	field->refcount = 1;
	return field;
}

void destroy_value(struct value *field)
{
	if (field->type == TYPE_FUNCTION) {
		int i;
		for (i = 0; i < field->u.function.nr_args; i++)
			unref_value(field->u.function.args[i]);
	}
	free(field);
}
