#include "vm_util.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"

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
#define EVICTION_MOVE_TO_NEXT page_elem = list_next(page_elem); if (page_elem == list_end(&page_list)) page_elem = list_begin(&page_list); if (page_elem == terminal) break

static bool evict_page(void) {
	if (list_empty(&page_list)) return false;
	if (page_elem == NULL || page_elem == list_end(&page_list))
		page_elem = list_begin(&page_list);
	struct list_elem *terminal = page_elem;
	while (true) {
		EVICTION_GET_PAGE;
		EVICTION_MOVE_TO_NEXT;
	}
	while (true){
		EVICTION_GET_PAGE;
		EVICTION_MOVE_TO_NEXT;
	}
	while (true) {
		EVICTION_GET_PAGE;
		EVICTION_MOVE_TO_NEXT;
	}
	while (true) {
		EVICTION_GET_PAGE;
		EVICTION_MOVE_TO_NEXT;
	}
	PANIC("########################### EVICTION NEEDED ###########################\n");
	return false;
}

#undef EVICTION_GET_PAGE
#undef EVICTION_MOVE_TO_NEXT

void *evict_and_get_kaddr(void) {
	if (!evict_page()) return NULL;
	void* kpage = palloc_get_page(PAL_USER | PAL_ZERO);
	return kpage;
}
