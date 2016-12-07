#ifndef FILE_MAPPING_H
#define FILE_MAPPING_H
#include "filesys/file.h"
#define PAGE_SIZE 1024 * 4

struct file_mapping {
	struct file *fl;
    int file_size;
	void *start_vaddr;
};

struct file_mappings {
	struct file_mapping *mappings;
	int pool_size;
};

#include "threads/thread.h"

void file_mapping_init(struct file_mapping *f);
void file_mapping_dispose(struct thread *t, struct file_mapping *f);

void file_mappings_init(struct file_mappings *m);
void file_mappings_dispose(struct thread *t, struct file_mappings *m);

int file_mappings_map(struct thread *t, struct file *fl, void *vaddr);
int file_mappings_unmap(struct thread *t, int mapping_id);

#endif
