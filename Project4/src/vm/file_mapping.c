#include "file_mapping.h"
#include "threads/malloc.h"

static bool file_mapping_unused(struct file_mapping *f) {
	return ((f == NULL) || ((f->fl == NULL) && (f->start_vaddr == NULL)));
}

void file_mapping_init(struct file_mapping *f) {
	f->fl = NULL;
	f->start_vaddr = NULL;
}
void file_mapping_dispose(struct file_mapping *f) {
	if (file_mapping_unused(f)) return;
	// ETC...
	file_mapping_init(f);
}


void file_mappings_init(struct file_mappings *m) {
	m->mappings = NULL;
	m->pool_size = 0;
}
void file_mappings_dispose(struct file_mappings *m) {
	int i;
	for (i = 0; i < m->pool_size; i++)
		file_mapping_dispose(m->mappings + i);
	free(m->mappings);
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

static bool file_mappable(struct thread *t, struct file *fl, void *vaddr) {
	if (t == NULL || fl == NULL || vaddr == NULL) return false;
	int file_sz = file_length(fl);
	// ETC...
	return false;
}

static bool file_map(struct thread *t, struct file *fl, void *vaddr, struct file_mapping *mapping) {
	if (t == NULL || fl == NULL || vaddr == NULL || mapping == NULL) return false;
	struct file *new_file = file_reopen(fl);
	if (new_file == NULL) return false;
	int file_sz = file_length(new_file);
	if (file_sz < 0) {
		file_close(new_file);
		return false;
	}
	bool success = true;
	// ETC...
	if (success) {
		mapping->fl = new_file;
		mapping->start_vaddr = vaddr;
	}
	else file_close(new_file);
	return success;
}

int file_mappings_map(struct thread *t, struct file *fl, void *vaddr) {
	if (!file_mappable(t, fl, vaddr)) return (-1);
	struct file_mappings *mappings = &t->mem_mappings;
	int free_id = file_mappings_seek_free_id(mappings);
	if (free_id >= 0)
		if (!file_map(t, fl, vaddr, mappings->mappings + free_id))
			free_id = (-1);
	return free_id;
}
int file_mappings_unmap(struct thread *t, int mapping_id) {
	struct file_mappings *mappings = &t->mem_mappings;
	if (mapping_id >= 0 && mapping_id < mappings->pool_size)
		file_mapping_dispose(mappings->mappings + mapping_id);
	return 0;
}

