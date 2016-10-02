#ifndef PROJECT1_FSH_UNALIAS_H
#define PROJECT1_FSH_UNALIAS_H

#include <unistd.h>
#include "util.h"
#include "hashset.h"
#include "string.h"
#include <stdio.h>
#include "arguments.h"
#include "context.h"

/* Not part of the interface */
bool fsh_unalias_helper(pos_arguments *args, context *context);

/* Implements shell built-in 'unalias' */
bool fsh_unalias(pos_arguments *args);

#endif

