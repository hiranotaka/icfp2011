#ifndef SIM_TYPES_H
#define SIM_TYPES_H

enum type {
	TYPE_INTEGER, TYPE_FUNCTION,
};

struct function;
struct value;
struct game;

struct function_operations {
	const char* name;
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
	} u;
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
	int nr_turns;
	int turn;
	int zombie;
	int nr_applications;
};

#endif
