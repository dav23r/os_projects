#include "file_mapping.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "vm/supplemental_page.h"
#include "userprog/pagedir.h"
#include "lib/kernel/hash.h"
#include "userprog/syscall.h"

static bool file_mapping_unused(struct file_mapping *f) {
	return ((f == NULL) || ((f->fl == NULL) && (f->start_vaddr == NULL)));
}

void file_mapping_init(struct file_mapping *f) {
	f->fl = NULL;
	f->start_vaddr = NULL;
	f->offset = 0;
	f->file_size = 0;
	f->writable = false;
	f->fl_writable = false;
}

void file_mapping_dispose(struct thread *t, struct file_mapping *f) {
	if (file_mapping_unused(f)) return;
	if (f->fl_writable) {
		char *cur_page = f->start_vaddr;
		char *end = (cur_page + f->file_size - f->offset);
		while (cur_page < end) {
			struct suppl_page *page = suppl_pt_lookup(t->suppl_page_table, cur_page);
			if (!suppl_page_load_to_file(page, false))
				PANIC("\n############################### ERROR LOADING CHANGES TO THE FILE ##############################\n");
			suppl_page_dispose(page);
			hash_delete(&t->suppl_page_table->pages_map, &page->hash_elem);
			free(page);
			cur_page += PAGE_SIZE;
		}
	}
	filesys_lock_acquire();
	file_close(f->fl);
	filesys_lock_release();
	file_mapping_init(f);
}

void file_mappings_init(struct file_mappings *m) {
	m->mappings = NULL;
	m->pool_size = 0;
}

void file_mappings_dispose(struct thread *t, struct file_mappings *m) {
	int i;
	for (i = 0; i < m->pool_size; i++)
		file_mapping_dispose(t, m->mappings + i);
	if (m->mappings != NULL) free(m->mappings);
	file_mappings_init(m);
}

static int file_mappings_seek_free_id(struct file_mappings *m) {
	int i, free_id;
	free_id = (-1);
	for (i = 0; i < m->pool_size; i++)
		if (file_mapping_unused(m->mappings + i)) {
			free_id = i;
			break;
		}
	if (free_id >= 0) return free_id;
	else {
		int new_pool_size = (2 * m->pool_size);
		if (new_pool_size < 2) new_pool_size = 2;
		
		struct file_mapping *new_pool = malloc(sizeof(struct file_mapping) * new_pool_size);
		if (new_pool == NULL) PANIC("UNABLE TO ALLOCATE MEMORY TO STORE FILE MAPPINGS");
		free_id = m->pool_size;
		for (i = 0; i < m->pool_size; i++)
			new_pool[i] = m->mappings[i];
		free(m->mappings);

		m->mappings = new_pool;
		m->pool_size = new_pool_size;
		for (i = free_id; i < m->pool_size; i++)
			file_mapping_init(m->mappings + i);
		return free_id;
	}
}

static bool file_mappable(struct thread *t, struct file *fl, void *vaddr, uint32_t offset, uint32_t file_size, uint32_t overshoot) {
	if (t == NULL || fl == NULL || vaddr == NULL) return false;
    if (pg_ofs(vaddr) != 0) return false;
    char *cur_page = vaddr;
	char *end = ((((char*)vaddr) + file_size + overshoot) - offset);
    while (cur_page < end){
        if (pagedir_get_page(t->pagedir, cur_page) != NULL) 
            return false;
        struct suppl_page *page = suppl_pt_lookup(t->suppl_page_table, cur_page);
		if (page != NULL) return false;
		cur_page += PAGE_SIZE;	
    }

	return true;
}

static bool file_map(struct thread *t, struct file *fl, void *vaddr, struct file_mapping *mapping, uint32_t offset, uint32_t file_size, uint32_t overshoot, bool writable, bool fl_writable) {
	if (t == NULL || fl == NULL || vaddr == NULL || mapping == NULL) return false;
	filesys_lock_acquire();
	struct file *new_file = file_reopen(fl);
	if (new_file == NULL) {
		filesys_lock_release();
		return false;
	}
	filesys_lock_release();

	mapping->fl = new_file;
	mapping->start_vaddr = vaddr;
	mapping->offset = offset;
	mapping->file_size = file_size;
	mapping->writable = writable;
	mapping->fl_writable = fl_writable;

    // Iterate over pages and put them in suppl pt with new file mapping
    char *cur_page = vaddr;
	char *end = (((char*)vaddr) + (file_size + overshoot - offset));
	while (cur_page < end) {
        suppl_table_set_file_mapping(t, cur_page, mapping);
        cur_page += PAGE_SIZE; 
    }
	return true;
}

int file_mappings_map(struct thread *t, struct file *fl, void *vaddr, uint32_t offset, uint32_t file_size, uint32_t overshoot, bool writable, bool fl_writable) {
	if (!file_mappable(t, fl, vaddr, offset, file_size, overshoot)) {
		//PANIC("############################################# NOT MAPPABLE ###################################################\n");
		return (-1);
	}
	//PANIC("############################################# ATTEMPTING TO MAP ###################################################\n");
	struct file_mappings *mappings = &t->mem_mappings;
	int free_id = file_mappings_seek_free_id(mappings);
	if (free_id >= 0)
		if (!file_map(t, fl, vaddr, mappings->mappings + free_id, offset, file_size, overshoot, writable, fl_writable))
			free_id = (-1);
	return free_id;
}

int file_mappings_unmap(struct thread *t, int mapping_id) {
	struct file_mappings *mappings = &t->mem_mappings;
	if (mapping_id >= 0 && mapping_id < mappings->pool_size)
		file_mapping_dispose(t, mappings->mappings + mapping_id);
	return 0;
}

