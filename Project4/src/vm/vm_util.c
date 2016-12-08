#include "vm_util.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"

#define VM_UTIL_MAX_STACK_OFFSET 32
#define VM_MAX_STACK_SIZE (1024 * 1024 * 8)
#define VM_STACK_END (PHYS_BASE - VM_MAX_STACK_SIZE)

static struct list page_list;
static struct list_elem *page_elem;

void vm_itil_init(void) {
	list_init(&page_list);
	page_elem = NULL;
}
void register_suppl_page(struct suppl_page *page) {
	if (page->kaddr != 0)
		list_push_front(&page_list, &page->list_elem);
}

bool stack_grow_needed(const void *addr, const void *esp) {
	return (is_user_vaddr(addr) && ((const char*)addr) >= ((const char*)esp - VM_UTIL_MAX_STACK_OFFSET) && ((uint32_t)addr) >= ((uint32_t)VM_STACK_END));
}

#define EVICTION_GET_PAGE struct suppl_page *page = list_entry(page_elem, struct suppl_page, list_elem); ASSERT(page != NULL)
#define EVICTION_GET_PAGE_TYPE bool modified = suppl_page_dirty(page); bool referenced = suppl_page_accessed(page)
#define EVICTION_EVICT_PAGE \
	pagedir_clear_page(page->pagedir, (void*)page->vaddr); \
	palloc_free_page((void*)page->kaddr); \
	page->kaddr = 0; \
	page_elem = list_next(page_elem); \
	if (page_elem == list_end(&page_list)) \
		page_elem = NULL; \
	list_remove(&page->list_elem); \
	return true
#define EVICTION_EVICT_NOT_MODIFIED \
	if (page->mapping != NULL) \
		page->location = PG_LOCATION_FILE; \
	else page->location = PG_LOCATION_UNKNOWN; \
	EVICTION_EVICT_PAGE
#define EVICTION_EVICT_MODIFIED \
	if (page->mapping != NULL) { \
		suppl_page_load_to_file(page); \
		page->location = PG_LOCATION_FILE; \
	} \
	else {  \
		swap_page spage = swap_get_page(); \
		if (spage == SWAP_NO_PAGE) return false; \
		swap_load_page_to_swap(spage, (void*)page->vaddr); \
		page->saddr = spage; \
		page->location = PG_LOCATION_SWAP; \
	} \
	EVICTION_EVICT_PAGE
#define EVICTION_MOVE_TO_NEXT page_elem = list_next(page_elem); if (page_elem == list_end(&page_list)) page_elem = list_begin(&page_list); if (page_elem == terminal) break

static bool evict_page(void) {
	if (list_empty(&page_list)) return false;
	if (page_elem == NULL || page_elem == list_end(&page_list))
		page_elem = list_begin(&page_list);
	struct list_elem *terminal = page_elem;
	while (true) {
		EVICTION_GET_PAGE;
		EVICTION_GET_PAGE_TYPE;
		if ((!referenced) && (!modified)) {
			EVICTION_EVICT_NOT_MODIFIED;
		}
		EVICTION_MOVE_TO_NEXT;
	}
	while (true){
		EVICTION_GET_PAGE;
		EVICTION_GET_PAGE_TYPE;
		if ((!referenced) && (modified)) {
			EVICTION_EVICT_MODIFIED;
		}
		EVICTION_MOVE_TO_NEXT;
	}
	while (true) {
		EVICTION_GET_PAGE;
		bool modified = suppl_page_dirty(page);
		if ((!modified)) {
			EVICTION_EVICT_NOT_MODIFIED;
		}
		EVICTION_MOVE_TO_NEXT;
	}
	EVICTION_GET_PAGE;
	EVICTION_EVICT_MODIFIED;
	//PANIC("########################### EVICTION NEEDED ###########################\n");
	//return false;
}

#undef EVICTION_GET_PAGE
#undef EVICTION_GET_PAGE_TYPE
#undef EVICTION_EVICT_PAGE
#undef EVICTION_EVICT_NOT_MODIFIED
#undef EVICTION_MOVE_TO_NEXT

void *evict_and_get_kaddr(void) {
	if (!evict_page()) return NULL;
	void* kpage = palloc_get_page(PAL_USER | PAL_ZERO);
	return kpage;
}
