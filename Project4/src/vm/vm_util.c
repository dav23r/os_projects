#include "vm_util.h"
#include "threads/vaddr.h"

#define VM_UTIL_MAX_STACK_OFFSET 64
#define VM_MAX_STACK_SIZE (1024 * 1024 * 8)
#define VM_STACK_END (PHYS_BASE - VM_MAX_STACK_SIZE)

bool stack_grow_needed(const void *addr, const void *esp) {
	return (is_user_vaddr(addr) && ((const char*)addr) >= ((const char*)esp - VM_UTIL_MAX_STACK_OFFSET) && ((const char *)addr) >= VM_STACK_END);
}
