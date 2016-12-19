#include "cache.h"
#include "devices/block.h"


static struct cache cache;

void cache_init(void) {
	cache.sectors = NULL;
	cache.sector_count = 0;
}

