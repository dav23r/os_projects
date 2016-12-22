#include "cache.h"
#include "threads/malloc.h"
#include "devices/block.h"


static struct cache cache;

static unsigned sectors_hash(const struct hash_elem *e, void *aux)
{
	ASSERT(!aux);
	struct sector *sec = hash_entry(e, struct sector, hash_elem);
	return sec->index;
}

static bool sectors_map_less(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
	ASSERT(!aux);
	struct sector *sector_a = hash_entry(a, struct sector, hash_elem);
	struct sector *sector_b = hash_entry(b, struct sector, hash_elem);
	return ((unsigned)sector_a->index < (unsigned)sector_b->index);
}

void sector_init(struct sector *sec)
{
	lock_init(&sec->sector_lock);
}

void cache_init(void) {
	lock_init(&cache.cache_lock);
	sema_init(&cache.cache_sem, 64);
	hash_init(&cache.index_sectors_map, sectors_hash, sectors_map_less, NULL);
}


struct sector * take_sector(block_sector_t index)
{
	lock_acquire(&cache.cache_lock);
	static struct sector element;
	element.index = index;
	struct hash_elem * elem = hash_find(&cache.index_sectors_map, &element.hash_elem);
	if (elem == NULL) return NULL;
	struct sector *res = hash_entry(elem, struct sector, hash_elem);
	lock_release(&cache.cache_lock);

	lock_acquire(&res->sector_lock);
	return res;
}

void release_sector(struct sector *sec, block_sector_t index, bool changed)
{
	if (changed)
		cache.sectors[index].dirty = true;
	lock_release(&sec->sector_lock);
}