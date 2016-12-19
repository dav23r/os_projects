#ifndef VM_UTIL
#define VM_UTIL
#include "lib/stdbool.h"
#include "supplemental_page.h"

// Initilizes the data structures needed for the VM utilities to function properly.
void vm_itil_init(void);

// Removes the given page from the list of evictables.
void undo_suppl_page_registration(struct suppl_page *page);
// Adds given page to evictables.
void register_suppl_page(struct suppl_page *page);

// Returns true, if the address can be a part of stack at some point in time.
bool addr_in_stack_range(const void *addr);

// Returns true, if stack growth is reasonable, if page faulted on the given address.
bool stack_grow_needed(const void *addr, const void *esp);

// Evicts and allocates a kernel page.
void *evict_and_get_kaddr(void);
// Restores given page from swap.
bool restore_page_from_swap(struct suppl_page *page, bool reg_page);

// Synchronised version of pagedir_set_page
bool pagedir_set_page_synch(uint32_t *pd, void *upage, void *kpage, bool rw);
// Synchronised version of pagedir_clear_page
void pagedir_clear_page_synch(uint32_t *pd, void *upage);

#endif