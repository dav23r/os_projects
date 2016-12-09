#ifndef FILE_MAPPING_H
#define FILE_MAPPING_H
#include "filesys/file.h"
#include "lib/stdbool.h"
#include "threads/vaddr.h"
#define PAGE_SIZE PGSIZE

struct file_mapping {
	struct file *fl;
	uint32_t offset;
	uint32_t file_size;
	void *start_vaddr;
	bool writable;
	bool fl_writable;
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

int file_mappings_map(struct thread *t, struct file *fl, void *vaddr, uint32_t offset, uint32_t file_size, uint32_t overshoot, bool writable, bool fl_writable);
int file_mappings_unmap(struct thread *t, int mapping_id);

#endif
