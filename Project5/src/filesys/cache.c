#include "cache.h"
#include "threads/malloc.h"
#include "devices/block.h"
#include "filesys/filesys.h"
#include "devices/timer.h"
#include "threads/interrupt.h"
#include "threads/thread.h"


static struct cache cache;

static unsigned sectors_hash(const struct hash_elem *e, void *aux UNUSED)
{
	struct sector *sec = hash_entry(e, struct sector, hash_elem);
	return sec->index;
}

static void write_behind(void *arg UNUSED) { 
    while (true){
        timer_sleep(WRITE_BEHIND_SLEEP_TICKS); 
        sector_cache_flush(false); 
    }
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
	lock_init(&cache.cache_lock);
	sema_init(&cache.cache_sem, SECTOR_COUNT);
	hash_init(&cache.index_sectors_map, sectors_hash, sectors_map_less, NULL);
	list_init(&cache.free_list);
	unsigned int i;
	for (i = 0; i < SECTOR_COUNT; i++) {
		sector_init(cache.sectors + i);
		list_push_back(&cache.free_list, &cache.sectors[i].list_elem);
	}
    tid_t tid = thread_create("write-behind", PRI_DEFAULT, write_behind, NULL);
    ASSERT (tid);
}

static uint32_t clock_hand = 0;
static void evict_sector(void) {
	uint32_t end = clock_hand;
	while (true) {
		struct sector * cur = (cache.sectors + clock_hand);
		if (cur->index != (block_sector_t)(-1)) {
			if (cur->owners == 0) {
				if (cur->dirty)
					block_write(fs_device, cur->index, cur->data);
				hash_delete(&cache.index_sectors_map, &cur->hash_elem);
				list_push_back(&cache.free_list, &cur->list_elem);
				cur->index = (block_sector_t)(-1);
				return;
			}
		}
		clock_hand = ((clock_hand + 1) % SECTOR_COUNT);
		if (clock_hand == end) break;
	}
	PANIC("WELL... THIS SHOULD NOT HAPPEN...\n");
}

struct sector * take_sector(block_sector_t index, bool block)
{
	ASSERT(!intr_context());
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
		if (block) 
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
	if (block) 
		lock_acquire(&res->sector_lock);
	return res;
}

void release_sector(struct sector *sec, block_sector_t index, bool changed, bool blocked)
{
	ASSERT(!intr_context());
	ASSERT(sec->index == index);
	lock_acquire(&sec->owners_lock);
	if (changed)
		sec->dirty = true;
	sec->owners--;
	if (sec->owners == 0)
		sema_up(&cache.cache_sem);
	lock_release(&sec->owners_lock);

	if(blocked)
		lock_release(&sec->sector_lock);
}



int64_t last_flush_t = 0;
void sector_cache_flush(bool force) {
	ASSERT(!intr_context());
	int64_t t = timer_ticks();
	if (force || (t - last_flush_t) > 512) {
		lock_acquire(&cache.cache_lock);
		uint32_t i;
		for (i = 0; i < SECTOR_COUNT; i++) {
			struct sector * cur = (cache.sectors + i);
			if (cur->index != (block_sector_t)(-1))
				if (cur->dirty) {
					lock_acquire(&cur->owners_lock);
					block_write(fs_device, cur->index, cur->data);
					cur->dirty = false;
					lock_release(&cur->owners_lock);
				}
		}
		lock_release(&cache.cache_lock);
		last_flush_t = t;
	}
}
