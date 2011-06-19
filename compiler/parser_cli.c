#include <stdio.h>
#include "parser.h"
#include "../sim/types.h"
#include "../sim/debug.h"

int main(int argc, char *argv[]) {
	struct value *value;
	if (argc != 2)
		return 1;

	if (!parse(argv[1], NULL, &value))
		return 1;

	print_value(value);
	printf("\n");
	unref_value(value);
	return 0;
}
