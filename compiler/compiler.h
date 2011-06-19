#ifndef COMPILER_COMPILER_H
#define COMPILER_COMPILER_H

enum method { METHOD_CS, METHOD_SC };

struct compile_result {
	int nr_turns;
	int slot_index;
	char used_slots[256 / 8];

	enum method first_method;
	const char *first_card_name;
	int first_slot_index;
};

static inline int is_slot_used(const struct compile_result *result,
			       int slot_index)
{
	return result->used_slots[slot_index / 8] >> slot_index % 8 & 1;
}

static inline void set_slot_used(struct compile_result *result, int slot_index)
{
	result->used_slots[slot_index / 8] |= 1 << slot_index % 8;
}

struct value;
struct game;
void compile(struct value *value, struct compile_result *result,
	     const struct game *game);

#endif
