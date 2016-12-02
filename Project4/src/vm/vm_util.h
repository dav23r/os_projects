#ifndef VM_UTIL
#define VM_UTIL
#include "lib/stdbool.h"

bool stack_grow_needed(const void *addr, const void *esp);

#endif