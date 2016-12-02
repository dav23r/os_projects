#include "swap.h"
#include "lib/stdbool.h"
#include "lib/kernel/bitmap.h"
#include "block.h"

static bool swap_initialized = false;
static struct bitmap *alloc_map;
static char *SWAP_ID = "swap_block";

// pgsize - 4kb, block_size - 512b
#define SECTORS_PER_PAGE 8                
#define MAX_SECTORS_IN_SWAP 1024 * 1024 * SECTORS_PER_PAGE
#define BITMAP_SIZE MAX_SECTORS_IN_SWAP

#define START 0

static block *create_swap_block(){
    block *swap_block = block_register(SWAP_ID, BLOCK_SWAP, NULL, 
                                       MAX_SECTORS_IN_SWAP, NULL, NULL);
    return swap_block;
}

static void swap_init() {
	if (!swap_initialized) {
        alloc_map = bitmap_create(BITMAP_SIZE);
		swap_initialized = true;
	}
}

swap_page swap_get_page(void) {
	swap_init();
    size_t start_index = bitmap_scan_and_flip(alloc_map, START, 1, false);
    if (start_index == BITMAP_ERROR)
        return SWAP_NO_PAGE;
    return start_index;
}

void swap_free_page(swap_page page) {
	swap_init();
    if (bitmap_test(alloc_map, page) != true)
        PANIC("Attempting to free non-allocated swap sector %d", swap_page);
    bitmap_set(alloc_map, page, true); 
}



