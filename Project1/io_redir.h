#ifndef IO_REDIR
#define IO_REDIR

#include "parser_util.h"

const char *IO_REDIRECT_STDIN;
const char *IO_REDIRECT_STDOUT;
const char *IO_REDIRECT_STDOUT_APPEND;
const char **IO_REDIRECT_OPERATORS;

bool io_redirect(const token_t *command, const char *operand, const char *filename, context *c);

#endif
