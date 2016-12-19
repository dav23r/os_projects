
#ifndef CACHE_H
#define CACHE_H
#include "lib/stdbool.h"

struct sector {
	void *roja;
	bool dirty;
};
struct cache {
	struct sector *sectors;
	int sector_count;
};

void cache_init(void);

#endif
