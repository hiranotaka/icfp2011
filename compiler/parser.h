#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

struct value;
int parse(const char *expr, const char **endp, struct value **value);

#endif
