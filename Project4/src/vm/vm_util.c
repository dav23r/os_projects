#include "vm_util.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"

#define VM_UTIL_MAX_STACK_OFFSET 32
#define VM_MAX_STACK_SIZE (1024 * 1024 * 8)
#define VM_STACK_END (PHYS_BASE - VM_MAX_STACK_SIZE)

// List of the pages that are in RAM
static struct list page_list;

// The "hand" of our Clock algorithm
static struct list_elem *page_elem;

// Lock for synchronizing eviction
static struct semaphore eviction_lock;

// Initilizes the data structures needed for the VM utilities to function properly.
void vm_itil_init(void) {
	list_init(&page_list);
	sema_init(&eviction_lock, 1);
	page_elem = NULL;
}

// Removes the given page from the list of evictables.
void undo_suppl_page_registration(struct suppl_page *page) {
	sema_down(&eviction_lock);
	if (&page->list_elem == page_elem) {
		page_elem = list_next(page_elem);
		if (page_elem == list_end(&page_list))
			page_elem = NULL;
	}
	if (page->kaddr != 0)
		list_remove(&page->list_elem);
	sema_up(&eviction_lock);
}
// Adds given page to evictables.
void register_suppl_page(struct suppl_page *page) {
	if (page->kaddr != 0) {
		sema_down(&eviction_lock);
		list_push_front(&page_list, &page->list_elem);
		sema_up(&eviction_lock);
	}
}

// Returns true, if stack growth is reasonable, if page faulted on the given address.
bool stack_grow_needed(const void *addr, const void *esp) {
	return (is_user_vaddr(addr) && ((const char*)addr) >= ((const char*)esp - VM_UTIL_MAX_STACK_OFFSET) && ((uint32_t)addr) >= ((uint32_t)VM_STACK_END));
}




/* Some macros for Clock algorithm; they look ugly, but reduced the code repetition to minimum. */

// Gets supplemental page from hand pointer
#define EVICTION_GET_PAGE struct suppl_page *page = list_entry(page_elem, struct suppl_page, list_elem); ASSERT(page != NULL)

// Gets information about page and stores it in two booleans: modified and referenced.
#define EVICTION_GET_PAGE_TYPE bool modified = suppl_page_dirty(page); bool referenced = suppl_page_accessed(page)

// Cleans the page, so that it's no longer mapped on physical address.
#define EVICTION_EVICT_PAGE \
	void *page_kaddr = (void*)page->kaddr; \
	page->kaddr = 0; \
	pagedir_clear_page(page->pagedir, (void*)page->vaddr); \
	palloc_free_page(page_kaddr); \
	page_elem = list_next(page_elem); \
	if (page_elem == list_end(&page_list)) \
		page_elem = NULL; \
	list_remove(&page->list_elem); \
	/*printf("page evicted (addr: %u)\n", ((uint32_t)page->vaddr / PAGE_SIZE)); */ \
	sema_up(&eviction_lock); \
	return true

// Moves the content of the page to swap.
#define EVICTION_MOVE_TO_SWAP \
	swap_page spage = swap_get_page(); \
	if (spage == SWAP_NO_PAGE){ \
		sema_up(&eviction_lock); \
		return false; \
	} \
	swap_load_page_to_swap(spage, (void*)page->vaddr); \
	page->saddr = spage; \
	page->location = PG_LOCATION_SWAP; \
	/*printf("page->saddr = %d; page->vaddr = %d\n", (int)page->saddr, (int)page->vaddr); */

// Evicts unmodified page.
#define EVICTION_EVICT_NOT_MODIFIED \
	/*printf("NOT MODIFIED....\n"); */\
	if (page->mapping != NULL && page->mapping->fl_writable) \
		page->location = PG_LOCATION_FILE; \
	else{ \
		EVICTION_MOVE_TO_SWAP; \
	} \
	EVICTION_EVICT_PAGE

// Evicts a modified page.
#define EVICTION_EVICT_MODIFIED \
	/*printf("MODIFIED....\n"); */\
	if (page->mapping != NULL && page->mapping->fl_writable) { \
		suppl_page_load_to_file(page, true); \
		page->location = PG_LOCATION_FILE; \
	} \
	else { \
		EVICTION_MOVE_TO_SWAP; \
	} \
	EVICTION_EVICT_PAGE

// Moves "hand" of our Clock algorithm
#define EVICTION_MOVE_TO_NEXT page_elem = list_next(page_elem); if (page_elem == list_end(&page_list)) page_elem = list_begin(&page_list); if (page_elem == terminal) break

// Evicts a page using Clock algorithm
static bool evict_page(void) {
	sema_down(&eviction_lock);
	if (list_empty(&page_list)) {
		sema_up(&eviction_lock);
		return false;
	}
	//PANIC("########################## WILLING TO EVICT ###################################\n");
	if (page_elem == NULL || page_elem == list_end(&page_list))
		page_elem = list_begin(&page_list);
	struct list_elem *terminal = page_elem;
	while (true) {
		EVICTION_GET_PAGE;
		EVICTION_GET_PAGE_TYPE;
		if ((!referenced) && (!modified)) {
			//printf("############### THROWING THE PAGE AWAY ###########################\n");
			/*EVICTION_EVICT_MODIFIED; /*/ EVICTION_EVICT_NOT_MODIFIED; //*/
		}
		EVICTION_MOVE_TO_NEXT;
	}
	while (true){
		EVICTION_GET_PAGE;
		EVICTION_GET_PAGE_TYPE;
		if ((!referenced) && (modified)) {
			//PANIC("############### EVICTING THE PAGE ###########################\n");
			EVICTION_EVICT_MODIFIED;
		}
		EVICTION_MOVE_TO_NEXT;
	}
	while (true) {
		EVICTION_GET_PAGE;
		bool modified = suppl_page_dirty(page);
		if ((!modified)) {
			//PANIC("############### THROWING THE PAGE AWAY ###########################\n");
			/*EVICTION_EVICT_MODIFIED; /*/ EVICTION_EVICT_NOT_MODIFIED; //*/
		}
		EVICTION_MOVE_TO_NEXT;
	}
	EVICTION_GET_PAGE;
	//PANIC("############### EVICTING THE PAGE ###########################\n");
	EVICTION_EVICT_MODIFIED;
	//PANIC("########################### EVICTION NEEDED ###########################\n");
	//return false;
}

// Undefs for the macros:
#undef EVICTION_GET_PAGE
#undef EVICTION_GET_PAGE_TYPE
#undef EVICTION_MOVE_TO_SWAP
#undef EVICTION_EVICT_PAGE
#undef EVICTION_EVICT_NOT_MODIFIED
#undef EVICTION_MOVE_TO_NEXT

// Evicts and allocates a kernel page.
void *evict_and_get_kaddr(void) {
	//printf("evicting...\n");
	if (!evict_page()) return NULL;
	void* kpage = palloc_get_page(PAL_USER | PAL_ZERO);
	//if (kpage == NULL) PANIC("ERROR");
	//else printf("ok...\n");
	return kpage;
}


// Restores given page from swap.
bool restore_page_from_swap(struct suppl_page *page, bool reg_page) {
	//PANIC("################### RESTORING ######################\n");
	sema_down(&eviction_lock);
	//printf("Restoring...\n");
	ASSERT(page->location == PG_LOCATION_SWAP && page->saddr != SWAP_NO_PAGE);
	void* kpage = palloc_get_page(PAL_USER | PAL_ZERO);
	if (kpage == NULL) {
		sema_up(&eviction_lock);
		kpage = evict_and_get_kaddr();
		sema_down(&eviction_lock);
		if (kpage == NULL) {
			sema_up(&eviction_lock);
			return false;
		}
	}
	if (!pagedir_set_page(page->pagedir, (void*)page->vaddr, kpage, true)) {
		palloc_free_page(kpage);
		sema_up(&eviction_lock);
		return false;
	}
	swap_load_page_to_ram(page->saddr, (void*)page->vaddr);
	swap_free_page(page->saddr);
	page->saddr = SWAP_NO_PAGE;
	page->kaddr = ((uint32_t)kpage);
	page->location = PG_LOCATION_RAM;
	sema_up(&eviction_lock);
	if(reg_page)
		register_suppl_page(page);
	//printf("done...\n");
	return true;
}

// Synchronised version of pagedir_set_page
bool pagedir_set_page_synch(uint32_t *pd, void *upage, void *kpage, bool rw) {
	sema_down(&eviction_lock);
	bool rv = pagedir_set_page(pd, upage, kpage, rw);
	sema_up(&eviction_lock);
	return rv;
}
// Synchronised version of pagedir_clear_page
void pagedir_clear_page_synch(uint32_t *pd, void *upage) {
	sema_down(&eviction_lock);
	pagedir_clear_page(pd, upage);
	sema_up(&eviction_lock);
}
