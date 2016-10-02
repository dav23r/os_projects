
#ifndef __input_parser__
#define __input_parser__

#include "bool.h"
#include "context.h"
#include "functions.h"

/**
 * Parses and executes the input line.
 * @param line - user input
 * @param c - context
 * @return true, if successful
 */
bool parse_input_line(const char *line, context *c);

#endif
