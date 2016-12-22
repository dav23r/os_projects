#include "cache.h"
#include "threads/malloc.h"
#include "devices/block.h"
#include "filesys/filesys.h"


static struct cache cache;

static unsigned sectors_hash(const struct hash_elem *e, void *aux UNUSED)
{
	struct sector *sec = hash_entry(e, struct sector, hash_elem);
	return sec->index;
}

static bool sectors_map_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
	ASSERT(!aux);
	struct sector *sector_a = hash_entry(a, struct sector, hash_elem);
	struct sector *sector_b = hash_entry(b, struct sector, hash_elem);
	return ((unsigned)sector_a->index < (unsigned)sector_b->index);
}

void sector_init(struct sector *sec)
{
	sec->index = (block_sector_t)(-1);
	sec->owners = 0;
	lock_init(&sec->sector_lock);
	lock_init(&sec->owners_lock);
}

void cache_init(void) {
	unsigned int i;
	for (i = 0; i < SECTOR_COUNT; i++) {
		sector_init(cache.sectors + i);
		list_push_back(&cache.free_list, &cache.sectors[i].list_elem);
	}
	lock_init(&cache.cache_lock);
	sema_init(&cache.cache_sem, SECTOR_COUNT);
	hash_init(&cache.index_sectors_map, sectors_hash, sectors_map_less, NULL);
	list_init(&cache.free_list);
}


static void evict_sector(void) {

}

struct sector * take_sector(block_sector_t index)
{
	lock_acquire(&cache.cache_lock);
	static struct sector element;
	element.index = index;
	struct hash_elem * elem = hash_find(&cache.index_sectors_map, &element.hash_elem);
	if (elem != NULL) {
		struct sector *res = hash_entry(elem, struct sector, hash_elem);
		
		lock_acquire(&res->owners_lock);
		if(res->owners == 0)
			sema_down(&cache.cache_sem);
		res->owners++;
		lock_release(&res->owners_lock);

		lock_release(&cache.cache_lock);
		lock_acquire(&res->sector_lock);
		return res;
	}

	sema_down(&cache.cache_sem);

	if (list_empty(&cache.free_list)) evict_sector();
	struct sector *res = list_entry(list_pop_back(&cache.free_list), struct sector, list_elem);
	res->index = index;
	res->dirty = false;
	res->owners = 1;
	block_read(fs_device, index, res->data);
	hash_insert(&cache.index_sectors_map, &res->hash_elem);
	
	lock_release(&cache.cache_lock);

	lock_acquire(&res->sector_lock);
	return res;
}

void release_sector(struct sector *sec, block_sector_t index, bool changed)
{
	ASSERT(sec->index == index);
	if (changed)
		sec->dirty = true;

	lock_acquire(&sec->owners_lock);
	sec->owners--;
	if (sec->owners == 0)
		sema_up(&cache.cache_sem);
	lock_release(&sec->owners_lock);

	lock_release(&sec->sector_lock);
}