#include "swap.h"
#include "lib/stdbool.h"
#include "lib/kernel/bitmap.h"
#include "devices/block.h"
#include "lib/debug.h"
#include "threads/synch.h"

static bool swap_initialized = false;
static struct bitmap *alloc_map = NULL;
static struct block *swap_block = NULL;
static struct lock allocation_lock;

// pgsize - 4kb, block_size - 512b
#define SECTORS_PER_PAGE 8                
#define MAX_SECTORS_IN_SWAP 1024 * 1024 * SECTORS_PER_PAGE
#define BITMAP_SIZE MAX_SECTORS_IN_SWAP

#define START 0

// Prototypes of static functions 
static void swap_init(void);

static void swap_init() {
	if (!swap_initialized) {
        alloc_map = bitmap_create(BITMAP_SIZE);
        swap_block = block_get_role(BLOCK_SWAP);
        lock_init(&allocation_lock);
        ASSERT(alloc_map != NULL && swap_block != NULL);
		swap_initialized = true;
	}
}

swap_page swap_get_page(void) {

    // Synchronize access to this code segment
    lock_acquire(&allocation_lock);

    // Find interval of SECTORS_PER_PAGE consecutive 'false' bits and flip them  
    swap_page start_sector = bitmap_scan_and_flip(alloc_map, START, SECTORS_PER_PAGE, false);

    lock_release(&allocation_lock);

    if (start_sector == BITMAP_ERROR)
        return SWAP_NO_PAGE;
    return start_sector;
}

void swap_free_page(swap_page page) {

    // Ensure all needed sectors are marked in bitmap
    if (bitmap_contains(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, false))
        PANIC("Attempting to free non-allocated swap sector %d", (int) page);

    bitmap_set_multiple(alloc_map, page, SECTORS_PER_PAGE, false); 
}

void swap_load_page_to_ram(swap_page page, void *addr) {

    // Assert that all sectors of 'page' are allocated
    if (bitmap_contains(alloc_map, page, SECTORS_PER_PAGE, false))
        PANIC("Attempting to load non-allocated swap sector %d", (int) page);

    char *addr_for_cur_sector = addr;
    int i;
    for (i = 0; i < SECTORS_PER_PAGE; i++){
        block_read(swap_block, page + i, addr_for_cur_sector);    
        addr_for_cur_sector += BLOCK_SECTOR_SIZE;
    }
}

void swap_load_page_to_swap(swap_page page, void *addr) {
    
    // Assert that all sectors pointed by 'page' are allocated
    if (bitmap_contains(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, false))
        PANIC("Attempting to write to non-allocated swap sector %d", (int) page);

    char *addr_for_cur_sector = addr;
    int i;
    for (i = 0; i < SECTORS_PER_PAGE; i++){
        block_write(swap_block, page + i, addr_for_cur_sector);
        addr_for_cur_sector += BLOCK_SECTOR_SIZE;
    }
}

