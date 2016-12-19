#ifndef SWAP_H
#define SWAP_H

typedef long long swap_page; // Swap page identifier
#define SWAP_NO_PAGE -1 // Error code for swap page(returned if none found)

// Initilizes swap block.
void swap_init(void);

// Finds and returns free swap page.
swap_page swap_get_page(void);

// Releases swap page.
void swap_free_page(swap_page page);

// Loads the content of the addr page into given swap page.
void swap_load_page_to_ram(swap_page page, void *addr);

// Loads the content of the swap page to physical memory.
void swap_load_page_to_swap(swap_page page, void *addr);

#endif
