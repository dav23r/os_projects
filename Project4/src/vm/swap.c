#include "swap.h"
#include "lib/stdbool.h"
#include "lib/kernel/bitmap.h"
#include "devices/block.h"
#include "lib/debug.h"

static bool swap_initialized = false;
static struct bitmap *alloc_map;
static char *SWAP_ID = "swap_block";

// pgsize - 4kb, block_size - 512b
#define SECTORS_PER_PAGE 8                
#define MAX_SECTORS_IN_SWAP 1024 * 1024 * SECTORS_PER_PAGE
#define BITMAP_SIZE MAX_SECTORS_IN_SWAP

#define START 0

// Prototypes of static functions 
static struct block *create_swap_block(void);
static void swap_init(void);

static struct block *create_swap_block(){
    struct block *swap_block = block_register(SWAP_ID, BLOCK_SWAP, 
                                NULL, MAX_SECTORS_IN_SWAP, NULL, NULL);
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
    swap_page start_sector = bitmap_scan_and_flip(alloc_map, START, SECTORS_PER_PAGE, false);
    if (start_sector == BITMAP_ERROR)
        return SWAP_NO_PAGE;
    return start_sector;
}

void swap_free_page(swap_page page) {
    swap_init();

    if (bitmap_contains(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, false))
        PANIC("Attempting to free non-allocated swap sector %d", (int) page);

    bitmap_set_multiple(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, false); 
}

void swap_load_page_to_ram(swap_page page, void *addr) {
    swap_init();
    struct block *swap_block = block_get_by_name(SWAP_ID);    
    if (swap_block == NULL)
        swap_block = create_swap_block();

    if (bitmap_contains(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, false))
        PANIC("Attempting to load non-allocated swap sector %d", (int) page);
    bitmap_set_multiple(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, false); 

    char *addr_for_cur_sector = addr;
    int i;
    for (i = 0; i < SECTORS_PER_PAGE; i++){
        block_read(swap_block, page, addr_for_cur_sector);    
        addr_for_cur_sector += BLOCK_SECTOR_SIZE;
    }
}

void swap_load_page_to_swap(swap_page page, void *addr) {
    swap_init();
    struct block *swap_block = block_get_by_name(SWAP_ID);    
    if (swap_block == NULL)
        swap_block = create_swap_block();

    if (bitmap_contains(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, true))
        PANIC("Attempting to write to used swap sector %d", (int) page);

    bitmap_set_multiple(alloc_map, page * SECTORS_PER_PAGE, SECTORS_PER_PAGE, true); 

    char *addr_for_cur_sector = addr;
    int i;
    for (i = 0; i < SECTORS_PER_PAGE; i++){
        block_write(swap_block, page, addr_for_cur_sector);
        addr_for_cur_sector += BLOCK_SECTOR_SIZE;
    }
}

