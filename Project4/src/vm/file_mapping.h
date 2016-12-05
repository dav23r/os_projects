#ifndef FILE_MAPPING_H
#define FILE_MAPPING_H
#include "filesys/file.h"

struct file_mapping {
	struct file *fl;
	void *start_vaddr;
};

void file_mapping_init(struct file_mapping *f);
void file_mapping_dispose(struct file_mapping *f);

struct file_mappings {
	struct file_mapping *mappings;
	int pool_size;
};

void file_mappings_init(struct file_mappings *m);
void file_mappings_dispose(struct file_mappings *m);

#include "threads/thread.h"

int file_mappings_map(struct thread *t, struct file *fl, void *vaddr);
int file_mappings_unmap(struct thread *t, int mapping_id);

#endif
