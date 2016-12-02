//
// Created by maisu on 12/1/16.
//
#ifndef PROJECT4_SUPPLEMENTAL_PAGE_H
#define PROJECT4_SUPPLEMENTAL_PAGE_H

#include "lib/kernel/hash.h"
#include "threads/thread.h"

enum suppl_page_location {
	PG_LOCATION_UNKNOWN,
	PG_LOCATION_RAM,
	PG_LOCATION_SWAP,
	PG_LOCATION_FILE
};

struct suppl_page {
    uint32_t vaddr;
    // DATA
	enum suppl_page_location location;
    struct hash_elem hash_elem;
};

struct suppl_pt {
	struct thread *owner_thread;
	struct hash pages_map;
} ;

unsigned pages_map_hash(const struct hash_elem *e, void *aux);

bool pages_map_less(const struct hash_elem *a,
                    const struct hash_elem *b,
                    void *aux);

void suppl_page_init(struct suppl_page *page);
struct suppl_page * suppl_page_new(void);
void suppl_page_dispose(struct suppl_page *page);
void suppl_page_delete(struct suppl_page *page);

void suppl_pt_init(struct suppl_pt *pt);
struct suppl_pt * suppl_pt_new(void);
void suppl_pt_dispose(struct suppl_pt *pt);
void suppl_pt_delete(struct suppl_pt *pt);

bool suppl_table_set_page(struct thread *t, void *upage, void *kpage, bool rw); // line 1: call - pagedir_set_page (t->pagedir, upage, kpage, writable)
bool suppl_table_alloc_user_page(struct thread *t, void *upage, bool writeable);

struct suppl_page *suppl_pt_lookup(struct suppl_pt *pt, void *vaddr);

#endif //PROJECT4_SUPPLEMENTAL_PAGE_H
