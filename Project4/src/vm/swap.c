#include "swap.h"
#include "lib/stdbool.h"

static bool swap_initialized = false;
static void swap_init() {
	if (!swap_initialized) {
		// Initialisation goes here...
		swap_initialized = true;
	}
}


swap_page swap_get_page(void) {
	swap_init();
	return SWAP_NO_PAGE;
}
void swap_free_page(swap_page page) {
	swap_init();
}
void swap_load_page_to_ram(swap_page page, void *addr) {
	swap_init();
}
void swap_load_page_to_swap(swap_page page, void *addr) {
	swap_init();
}

