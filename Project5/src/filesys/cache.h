
#ifndef CACHE_H
#define CACHE_H
#include "lib/stdbool.h"
#include "devices/block.h"
#include "threads/synch.h"
#include "lib/kernel/hash.h"

#define SECTOR_COUNT 64
#define WRITE_BEHIND_SLEEP_TICKS 256

struct sector {
	block_sector_t index;
	char data[BLOCK_SECTOR_SIZE];
	bool dirty;
	uint32_t owners;
	struct lock sector_lock;
	struct lock owners_lock;
	struct hash_elem hash_elem;
	struct list_elem list_elem;
};
struct cache {
	struct sector sectors[SECTOR_COUNT];
	struct lock cache_lock;
	struct semaphore cache_sem;
	struct hash index_sectors_map;
	struct list free_list;
};

void sector_init(struct sector *sec);
void cache_init(void);

struct sector * take_sector(block_sector_t index, bool block);

void release_sector(struct sector *sec, block_sector_t index, bool changed, bool blocked);

void sector_cache_flush(bool force);

#endif
