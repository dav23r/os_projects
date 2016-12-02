//
// Created by maisu on 12/1/16.
//

#include "supplemental_page.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"

unsigned pages_map_hash(const struct hash_elem *e, void *aux UNUSED) {
	struct suppl_page *page = hash_entry(e, struct suppl_page, hash_elem);
	return (page->vaddr);
}
bool pages_map_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
	struct suppl_page *page_a = hash_entry(a, struct suppl_page, hash_elem);
	struct suppl_page *page_b = hash_entry(b, struct suppl_page, hash_elem);
	return (page_a->vaddr < page_b->vaddr);
}

static void suppl_page_hash_dispose(struct hash_elem *e, void *aux UNUSED) {
	struct suppl_page *page = hash_entry(e, struct suppl_page, hash_elem);
	suppl_page_delete(page);
}

void suppl_page_init(struct suppl_page *page) {
	if (page == NULL) return;
	// ETC...
	page->location = PG_LOCATION_UNKNOWN;
}
struct suppl_page * suppl_page_new(void) {
	struct suppl_page * page = malloc(sizeof(struct suppl_page));
	suppl_page_init(page);
	return page;
}
void suppl_page_dispose(struct suppl_page *page) {
	if (page == NULL) return;
	// ETC...
}
void suppl_page_delete(struct suppl_page *page) {
	suppl_page_dispose(page);
	if (page != NULL) free(page);
}

void suppl_pt_init(struct suppl_pt *pt) {
	if (pt == NULL) return;
	// ETC...
	if (!hash_init(&pt->pages_map, pages_map_hash, pages_map_less, NULL))
		PANIC("HASH INITIALISATION FAILED");
	pt->owner_thread = NULL;
}
struct suppl_pt * suppl_pt_new(void) {
	struct suppl_pt *pt = malloc(sizeof(struct suppl_pt));
	suppl_pt_init(pt);
	return pt;
}
void suppl_pt_dispose(struct suppl_pt *pt) {
	if (pt == NULL) return;
	// ETC...
	hash_destroy(&pt->pages_map, suppl_page_hash_dispose);
}
void suppl_pt_delete(struct suppl_pt *pt) {
	suppl_pt_dispose(pt);
	if(pt != NULL) free(pt);
}

bool suppl_table_set_page(struct thread *t, void *upage, void *kpage, bool rw) {
	upage = pg_round_down(upage);
	struct suppl_page * page = suppl_page_new();
	if (page == NULL) return false;
	if (!pagedir_set_page(t->pagedir, upage, kpage, rw)) {
		suppl_page_delete(page);
		return false;
	}
	page->vaddr = ((uint32_t)upage);
	hash_insert(&t->suppl_page_table->pages_map, &page->hash_elem);
	// ETC...
	return true;
}
bool suppl_table_alloc_user_page(struct thread *t, void *upage, bool writeable) {
	void* kpage = palloc_get_page(PAL_USER | PAL_ZERO);
	if(kpage != NULL)
		return suppl_table_set_page(t, upage, kpage, writeable);
	else {
		// ETC...
		return false;
	}
}


struct suppl_page *suppl_pt_lookup(struct suppl_pt *pt, void *vaddr) {
	struct suppl_page tmp;
	tmp.vaddr = ((uint32_t)pg_round_down(vaddr));
	struct hash_elem * elem = hash_find(&(pt->pages_map), &(tmp.hash_elem));
	if (elem == NULL) return NULL;
	return hash_entry(elem, struct suppl_page, hash_elem);
}

