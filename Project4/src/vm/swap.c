#include "swap.h"
#include "lib/stdbool.h"
#include "lib/kernel/bitmap.h"
#include "devices/block.h"
#include "lib/debug.h"
#include "threads/synch.h"
#include "vm_util.h"

static bool swap_initialized = false;
static struct bitmap *alloc_map = NULL;
static struct block *swap_block = NULL;

// pgsize - 4kb, block_size - 512b
#define SECTORS_PER_PAGE (PAGE_SIZE / BLOCK_SECTOR_SIZE)

#define START 0

// Initilizes swap block.
void swap_init(void) {
	if (!swap_initialized) {
		swap_block = block_get_role(BLOCK_SWAP);
        alloc_map = bitmap_create(block_size(swap_block) / SECTORS_PER_PAGE);
        ASSERT(alloc_map != NULL && swap_block != NULL);
		vm_itil_init();
		swap_initialized = true;
	}
}

// Finds and returns free swap page.
swap_page swap_get_page(void) {
    // Find interval of SECTORS_PER_PAGE consecutive 'false' bits and flip them  
    swap_page start_sector = bitmap_scan_and_flip(alloc_map, START, 1, false);
    if (start_sector == BITMAP_ERROR)
        return SWAP_NO_PAGE;
    return start_sector;
}

// Releases swap page.
void swap_free_page(swap_page page) {
    // Ensure all needed sectors are marked in bitmap
    if (bitmap_contains(alloc_map, page, 1, false))
        PANIC("Attempting to free non-allocated swap sector %d", (int) page);

    bitmap_set_multiple(alloc_map, page, 1, false);
}

// Loads the content of the addr page into given swap page.
void swap_load_page_to_ram(swap_page page, void *addr) {

    // Assert that all sectors of 'page' are allocated
    if (bitmap_contains(alloc_map, page, 1, false))
        PANIC("Attempting to load non-allocated swap sector %d", (int) page);

    char *addr_for_cur_sector = addr;
	page *= SECTORS_PER_PAGE;
    int i;
    for (i = 0; i < SECTORS_PER_PAGE; i++){
        block_read(swap_block, page + i, addr_for_cur_sector);    
        addr_for_cur_sector += BLOCK_SECTOR_SIZE;
    }
}

// Loads the content of the swap page to physical memory.
void swap_load_page_to_swap(swap_page page, void *addr) {
    
    // Assert that all sectors pointed by 'page' are allocated
    if (bitmap_contains(alloc_map, page, 1, false))
        PANIC("Attempting to write to non-allocated swap sector %d", (int) page);

    char *addr_for_cur_sector = addr;
	page *= SECTORS_PER_PAGE;
    int i;
    for (i = 0; i < SECTORS_PER_PAGE; i++){
        block_write(swap_block, page + i, addr_for_cur_sector);
        addr_for_cur_sector += BLOCK_SECTOR_SIZE;
    }
}

