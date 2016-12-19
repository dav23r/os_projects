//
// Created by maisu on 12/1/16.
//
#ifndef PROJECT4_SUPPLEMENTAL_PAGE_H
#define PROJECT4_SUPPLEMENTAL_PAGE_H

#include "lib/kernel/hash.h"
#include "threads/thread.h"
#include "vm/file_mapping.h"
#include "swap.h"
#include <list.h>

// Enumeration for the current location of the page:
enum suppl_page_location {
	PG_LOCATION_UNKNOWN,	// Unknown location (this would generally mean, the page is invalid)
	PG_LOCATION_RAM,		// Page in RAM
	PG_LOCATION_SWAP,		// Page in swap
	PG_LOCATION_FILE		// Page in file
};

// SP structure:
struct suppl_page {
    uint32_t vaddr;			// Virtual address
    uint32_t kaddr;			// Kernel address (NULL if not mapped)
	swap_page saddr;		// Swap block identifier (SWAP_NO_PAGE if not in swap)
	uint32_t *pagedir;		// PD of the owner thread
    const struct file_mapping *mapping;	// File mapping (NULL if none)
	enum suppl_page_location location;	// Current location of the page
	bool dirty;			// True, if page is dirty (variable should never be accessed directly)
	bool accessed;		// True, if page is accessed (variable should never be accessed directly)
    struct hash_elem hash_elem;	// Element for hash map
	struct list_elem list_elem;	// Element for the list of evictables
};

// SPT structure:
struct suppl_pt {
	struct thread *owner_thread;	// Owner thread
	struct hash pages_map;			// Hash map for current thread's PS-s
};

// Hash function for SP
unsigned pages_map_hash(const struct hash_elem *e, void *aux);

// Less function for SP
bool pages_map_less(const struct hash_elem *a,
                    const struct hash_elem *b,
                    void *aux);

// Initializes empty SP.
void suppl_page_init(uint32_t *pagedir, struct suppl_page *page);
// Allocates, initializes and returns an empty SP.
struct suppl_page * suppl_page_new(uint32_t *pagedir);
// Cleans SP.
void suppl_page_dispose(struct suppl_page *page);
// Cleans and deallocates SP.
void suppl_page_delete(struct suppl_page *page);

// Returns true, if SP is/ever was dirty.
bool suppl_page_dirty(struct suppl_page *page);
// Returns true, if SP is/ever was accessed.
bool suppl_page_accessed(struct suppl_page *page);

// Loads page from file.
bool suppl_page_load_from_file(struct suppl_page *page);
// Loads page to file.
bool suppl_page_load_to_file(struct suppl_page *page, bool eviction_call);

// Initializes SPT.
void suppl_pt_init(struct suppl_pt *pt);
// Allocates and initializes SPT.
struct suppl_pt * suppl_pt_new(void);
// Cleans SPT.
void suppl_pt_dispose(struct suppl_pt *pt);
// Cleans and deallocates SPT.
void suppl_pt_delete(struct suppl_pt *pt);

// Sets SP to the given kernel address.
bool suppl_table_set_page(struct thread *t, void *upage, void *kpage, bool rw); 
// Sets file mapping.
bool suppl_table_set_file_mapping(struct thread *t, void *upage, struct file_mapping *mapping);
// Allocates SP (almost exclusively used for stack growth).
bool suppl_table_alloc_user_page(struct thread *t, void *upage, bool writeable);
// Searches for SP in given SPT.
struct suppl_page *suppl_pt_lookup(struct suppl_pt *pt, void *vaddr);

#endif 
