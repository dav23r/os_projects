//
// Created by maisu on 12/1/16.
//

#include "supplemental_page.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"

unsigned pages_map_hash(const struct hash_elem *e, void *aux UNUSED) {
	struct suppl_page *page = hash_entry(e, struct suppl_page, hash_elem);
	return (page->vaddr);
}
bool pages_map_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
	struct suppl_page *page_a = hash_entry(a, struct suppl_page, hash_elem);
	struct suppl_page *page_b = hash_entry(b, struct suppl_page, hash_elem);
	return (page_a->vaddr < page_b->vaddr);
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
}
void suppl_pt_delete(struct suppl_pt *pt) {
	suppl_pt_dispose(pt);
	if(pt != NULL) free(pt);
}

bool suppl_table_set_page(uint32_t *pd, void *upage, void *kpage, bool rw) {
	if (!pagedir_set_page(pd, upage, kpage, rw)) return false;
	// ETC...
	return true;
}
// line 1: call - pagedir_set_page (t->pagedir, upage, kpage, writable)

