
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


// loads fshell built-in functions into functions map
bool load_functions(hashset *map);

// void storeFuncIntoMap(hashset *map);

#endif

