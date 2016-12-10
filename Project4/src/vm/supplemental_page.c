//
// Created by maisu on 12/1/16.
//

#include "supplemental_page.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"
#include "vm_util.h"


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

void suppl_page_init(uint32_t *pagedir, struct suppl_page *page) {
	if (page == NULL) return;
    //page->vaddr = 0;
    page->kaddr = 0;
	page->saddr = SWAP_NO_PAGE;
	page->pagedir = pagedir;
    page->mapping = NULL;
	page->location = PG_LOCATION_UNKNOWN;
	page->dirty = false;
	page->accessed = false;
}

struct suppl_page * suppl_page_new(uint32_t *pagedir) {
	struct suppl_page * page = malloc(sizeof(struct suppl_page));
	suppl_page_init(pagedir, page);
	return page;
}

void suppl_page_dispose(struct suppl_page *page) {
	if (page == NULL) return;
	if (page->pagedir == NULL) PANIC("\n########################## PAGE MISSING PAGEDIR ############################\n");
	if (page->kaddr != 0) {
		undo_suppl_page_registration(page);
		pagedir_clear_page_synch(page->pagedir, (void*)page->vaddr);
		palloc_free_page((void*)page->kaddr);
	}
	if (page->saddr != SWAP_NO_PAGE)
		swap_free_page(page->saddr);
    //file_mapping_dispose(page->mapping);
    suppl_page_init(page->pagedir, page);
}

void suppl_page_delete(struct suppl_page *page) {
	suppl_page_dispose(page);
	if (page != NULL) free(page);
}

#define BIT_CHECK_FN(bit, function) \
	if (bit) return true; \
	else if (page->kaddr != 0) { \
		bit = function(page->pagedir, ((const void*)page->vaddr)); \
		return bit; \
	} else return false
bool suppl_page_dirty(struct suppl_page *page) {
	BIT_CHECK_FN((page->dirty), pagedir_is_dirty);
}
bool suppl_page_accessed(struct suppl_page *page) {
	BIT_CHECK_FN((page->accessed), pagedir_is_accessed);
}
#undef BIT_CHECK_FN


static bool set_kpage_if_needed(struct suppl_page *page, bool eviction_call) {
	if (eviction_call) return (page->kaddr != 0);
	void *kpage = ((void*)page->kaddr);
	undo_suppl_page_registration(page);
	bool kpage_new = (kpage == NULL);
	if (kpage_new && (!suppl_page_dirty(page)))
		kpage = palloc_get_page(PAL_USER | PAL_ZERO);
	if (kpage == NULL) {
		if (page->location == PG_LOCATION_SWAP) {
			return restore_page_from_swap(page, false);
		}
		else {
			//printf("EVICTING to read addr: %d\n", (int)page->vaddr);
			kpage = evict_and_get_kaddr();
		}
		if (kpage == NULL) return false;
		// ETC... ?
		//return false;
	}
	void *vaddr = ((void*)page->vaddr);
	if (kpage_new) {
		if (!pagedir_set_page_synch(page->pagedir, vaddr, kpage, 1)) {
			palloc_free_page(kpage);
			return false;
		}
		page->kaddr = ((uint32_t)kpage);
		//register_suppl_page(page);
	}
	return true;
}
bool suppl_page_load_from_file(struct suppl_page *page) {
	ASSERT(page->location == PG_LOCATION_FILE && page->mapping != NULL);
	if (page->mapping->fl == NULL) return false;
	if (!set_kpage_if_needed(page, false)) return false;
	char *start = ((char*)page->vaddr);
	char *buff = start;
	char *end = (start + PAGE_SIZE);
	bool file_r = false;
	//printf("\n########################\n");
	while (buff < end) {
		(*buff) = 0;
		if ((!file_r) && ((uint32_t)buff >= ((uint32_t)page->mapping->start_vaddr))) {
			//if (page->mapping == NULL || page->mapping->fl == NULL)
				//PANIC("\n############################ SOMEONE DECIDED IT WAS A GOOD IDEA TO MMAP TO NULL ###################################\n");
			filesys_lock_acquire();
			file_seek(page->mapping->fl, ((char*)buff) - ((char*)page->mapping->start_vaddr) + page->mapping->offset);
			char *fl_end = ((char*)page->mapping->start_vaddr) + ((long long)page->mapping->file_size - (long long)page->mapping->offset);
			long long buf_sz = (long long)(PAGE_SIZE - (buff - start));
			long long till_fl_end = ((long long)(fl_end - buff));
			/*
			printf("buffer:      %d\n", (int)buf_sz);
			printf("till end:    %d\n", (int)till_fl_end);
			printf("buff:        %d\n", (int)buff);
			*/
			if (till_fl_end < buf_sz) buf_sz = till_fl_end;
			if (buf_sz > 0)
				buff += file_read(page->mapping->fl, (((char*)page->kaddr) + (buff - start)), buf_sz);
			filesys_lock_release();
			file_r = true;
		}
		else buff++;
	}
	/*
	printf("########################\n");
	int i;
	printf("Map ADDRESS: %u\n", (uint32_t)page->mapping);
	printf("writable:    %u\n", (uint32_t)page->mapping->writable);
	printf("fl_writable: %u\n", (uint32_t)page->mapping->fl_writable);
	printf("vaddr:       %u\n", (uint32_t)page->vaddr);
	printf("map vaddr:   %u\n", (uint32_t)page->mapping->start_vaddr);
	printf("file_size:   %u\n", (uint32_t)page->mapping->file_size);
	printf("offset:      %u\n", (uint32_t)page->mapping->offset);
	printf("---------------\n");
	for (i = 0; i < PAGE_SIZE; i++) {
		char c = ((char*)page->vaddr)[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || c == '\n' || c == '\t')
			printf("%c", c);
		//else printf("<%d>", (int)c);
	}
	printf("\n########################\n");
	//*/

	pagedir_set_dirty(page->pagedir, (const void*)page->vaddr, false);
	if (!page->mapping->writable) {
		pagedir_clear_page_synch(page->pagedir, (void*)page->vaddr);
		if (!pagedir_set_page_synch(page->pagedir, (void*)page->vaddr, (void*)page->kaddr, false)) {
			palloc_free_page((void*)page->kaddr);
			return false;
		}
	}
	register_suppl_page(page);
	page->location = PG_LOCATION_RAM;
	// PANIC("################################ READING KINDA SEEMS SUCCESSFUL ###############################\n");

	return true;
}
bool suppl_page_load_to_file(struct suppl_page *page, bool eviction_call) {
	if (page == NULL || page->mapping == NULL || page->mapping->fl == NULL) return false;
	else if (!suppl_page_dirty(page)) return true;
	else if (!page->mapping->fl_writable) return true;
	else {
		if (!set_kpage_if_needed(page, eviction_call)) return false;
		char *start = ((char*)page->vaddr);
		char *end = (start + PAGE_SIZE);
		char *file_start = ((char*)page->mapping->start_vaddr);
		if (((uint32_t)start) < ((uint32_t)file_start))
			start = file_start;
		char *file_end = (((char*)page->mapping->start_vaddr) + page->mapping->file_size - page->mapping->offset);
		if (((uint32_t)end) > ((uint32_t)file_end))
			end = file_end;
		if (start < end) {
			filesys_lock_acquire();
			file_seek(page->mapping->fl, (start - file_start));
			int buffer_size = (end - start);
			bool rv = (file_write(page->mapping->fl, ((char*)page->kaddr) + (start - ((char*)page->vaddr)), buffer_size) == buffer_size);
			filesys_lock_release();
			if (!eviction_call) register_suppl_page(page);
			return rv;
		}
		else {
			if (!eviction_call) register_suppl_page(page);
			return true;
		}
	}
}


void suppl_pt_init(struct suppl_pt *pt) {
	if (pt == NULL) return;
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
	hash_destroy(&pt->pages_map, suppl_page_hash_dispose);
}

void suppl_pt_delete(struct suppl_pt *pt) {
	suppl_pt_dispose(pt);
	if(pt != NULL) free(pt);
}

bool suppl_table_set_page(struct thread *t, void *upage, void *kpage, bool rw) {
	upage = pg_round_down(upage);
	struct suppl_page * page = suppl_page_new(t->pagedir);
	if (page == NULL) return false;
    // TODO check below line (unsure about negation)
	if (!pagedir_set_page_synch(t->pagedir, upage, kpage, rw)) {
		suppl_page_delete(page);
		return false;
	}
	page->vaddr = ((uint32_t)upage);
	page->kaddr = ((uint32_t)kpage);
	page->location = PG_LOCATION_RAM;
	page->dirty = true;
	page->accessed = true;
	pagedir_set_dirty(page->pagedir, (const void*)page->vaddr, true);
	register_suppl_page(page);

	hash_insert(&t->suppl_page_table->pages_map, &page->hash_elem);
	// ETC...
	return true;
}

bool suppl_table_set_file_mapping(struct thread *t, void* upage, struct file_mapping *mapping){
	upage = pg_round_down(upage);
    ASSERT(pg_ofs(upage) == 0);
    ASSERT(mapping != NULL);

    struct suppl_pt *spt = t->suppl_page_table;
    struct suppl_page *page = suppl_page_new(t->pagedir);
    if (page == NULL) return false;
    page->vaddr = (uint32_t) upage;
	//page->map_cln = *mapping;
	page->mapping = mapping;//&page->map_cln;
	page->location = PG_LOCATION_FILE;
    
    hash_insert(&spt->pages_map, &page->hash_elem);
    
	//PANIC("####################################### MAPPED #########################################\n");

	return true;
}

bool suppl_table_alloc_user_page(struct thread *t, void *upage, bool writeable) {
	void* kpage = palloc_get_page(PAL_USER | PAL_ZERO);
	if(kpage == NULL) {
		kpage = evict_and_get_kaddr();
		// ETC... ?
		if(kpage == NULL) return false;
	}
	if (!suppl_table_set_page(t, upage, kpage, writeable)) {
		palloc_free_page(kpage);
		return false;
	}
	else return true;
}

struct suppl_page *suppl_pt_lookup(struct suppl_pt *pt, void *vaddr) {
	struct suppl_page tmp;
	tmp.vaddr = ((uint32_t)pg_round_down(vaddr));
	struct hash_elem * elem = hash_find(&(pt->pages_map), &(tmp.hash_elem));
	if (elem == NULL) return NULL;
	return hash_entry(elem, struct suppl_page, hash_elem);
}

