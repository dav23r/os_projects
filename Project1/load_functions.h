
#ifndef _load_functions_
#define _load_functions_

#include "hashset.h"
#include "vector.h"
#include "functions_runner.h"
#include "arguments.h"
#include "bool.h"
#include "context.h"
#include "fsh_kill.h"
#include "fsh_nice.h"
#include "fsh_type.h"
#include "fsh_ulimit.h"
#include "functions.h"
#include "iterator.h"
#include "util.h"
#include "fsh_unalias.h"


// loads fshell built-in functions into functions map
void load_functions(hashset *map);

void storeFuncIntoMap(hashset *map, char *fname, func_pointer fn);

#endif

