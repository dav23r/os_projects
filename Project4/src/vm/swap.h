#ifndef SWAP_H
#define SWAP_H

typedef long long swap_page;
#define SWAP_NO_PAGE -1

void swap_init(void);

swap_page swap_get_page(void);
void swap_free_page(swap_page page);
void swap_load_page_to_ram(swap_page page, void *addr);
void swap_load_page_to_swap(swap_page page, void *addr);

#endif
