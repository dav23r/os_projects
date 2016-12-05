#include "file_mapping.h"

static void file_mapping_unused(struct file_mapping *f) {
	return ((f->fl == NULL) && (f->start_vaddr == NULL));
}

void file_mapping_init(struct file_mapping *f) {
	f->fl = NULL;
	f->start_vaddr = NULL;
}
void file_mapping_dispose(struct file_mapping *f) {
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
		// ETC...
	}
}

int file_mappings_map(struct thread *t, struct file *fl, void *vaddr) {

}
int file_mappings_unmap(struct thread *t, int mapping_id) {

}

