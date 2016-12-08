#ifndef VM_UTIL
#define VM_UTIL
#include "lib/stdbool.h"
#include "supplemental_page.h"

void vm_itil_init(void);
void register_suppl_page(struct suppl_page *page);

bool stack_grow_needed(const void *addr, const void *esp);

void *evict_and_get_kaddr(void);
bool restore_page_from_swap(struct suppl_page *page);

#endif