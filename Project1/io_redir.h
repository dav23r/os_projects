#ifndef IO_REDIR
#define IO_REDIR

#include "parser_util.h"

const char *IO_REDIRECT_STDIN;
const char *IO_REDIRECT_STDOUT;
const char *IO_REDIRECT_STDOUT_APPEND;
const char **IO_REDIRECT_OPERATORS;

/**
 * IO redirection (< > >>)
 * @param command - command before the operand
 * @param operand - operand
 * @param filename - name of the file IO is ment to be redirected to
 * @param c - context
 * @return true, if success
 */
bool io_redirect(const token_t *command, const char *operand, const char *filename, context *c);

#endif
