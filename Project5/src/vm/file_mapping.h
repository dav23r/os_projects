#ifndef FILE_MAPPING_H
#define FILE_MAPPING_H
#include "filesys/file.h"
#include "lib/stdbool.h"
#include "threads/vaddr.h"
#define PAGE_SIZE PGSIZE

// Structure for file mapping information:
struct file_mapping {
	struct file *fl;		// File
	uint32_t offset;		// Offset from file's start
	uint32_t file_size;		// File size (at least, it's readable size)
	void *start_vaddr;		// Start vaddr of the mapping
	bool writable;			// True, if vaddr is writable
	bool fl_writable;		// True, if the file content is updatable
};

// List of file mappings:
struct file_mappings {
	struct file_mapping *mappings;	// File mapping pool
	int pool_size;					// Current size of the mapping pool
};

#include "threads/thread.h"

// Initializes file mapping.
void file_mapping_init(struct file_mapping *f);
// Dosposes file mapping.
void file_mapping_dispose(struct thread *t, struct file_mapping *f);

// Initializes the list of file mappings.
void file_mappings_init(struct file_mappings *m);
// Disposes the list of file mappings.
void file_mappings_dispose(struct thread *t, struct file_mappings *m);

/* MMAP function.
	Parameters:
			t			- thread;
			fl			- file;
			offset		- IO offset from file's start
			file_size	- readable size fo the file (mmap can be used for partial mapping)
			overshoot	- number of bytes that should be filled with zeroes after the file's end (or the end of the readable segment of the file)
			writable	- true, if the virtual address should be writable
			fl_writable	- true, if the file can be updated
	Return value:
			Mapping identifier, if the mapping was successful, or (-1) if it failed.
*/
int file_mappings_map(struct thread *t, struct file *fl, void *vaddr, uint32_t offset, uint32_t file_size, uint32_t overshoot, bool writable, bool fl_writable);
// Unmaps file mapping (updates file automatically, if possible).
int file_mappings_unmap(struct thread *t, int mapping_id);

#endif
