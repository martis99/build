#include "hash_map.h"

#include "mem.h"

#include <stdlib.h>
#include <string.h>

#define HASHMAP_MAX_LOAD	  0.75f
#define HASHMAP_RESIZE_FACTOR 2

void hashmap_create(hashmap_t *map, int capacity)
{
	map->capacity = capacity;
	map->count	  = 0;

	map->buckets = m_calloc(capacity, sizeof(struct bucket));
	map->first	 = NULL;

	map->last = (struct bucket *)&map->first;
}

void hashmap_free(hashmap_t *map)
{
	m_free(map->buckets, map->capacity * sizeof(struct bucket));
}

static struct bucket *resize_entry(hashmap_t *map, struct bucket *old_entry)
{
	uint32_t index = old_entry->hash % map->capacity;
	for (;;) {
		struct bucket *entry = &map->buckets[index];

		if (entry->key == NULL) {
			*entry = *old_entry;
			return entry;
		}

		index = (index + 1) % map->capacity;
	}
}

static void hashmap_resize(hashmap_t *map)
{
	struct bucket *old_buckets = map->buckets;
	int old_capacity		   = map->capacity;

	map->capacity *= HASHMAP_RESIZE_FACTOR;
	map->buckets = m_calloc(map->capacity, sizeof(struct bucket));

	map->last = (struct bucket *)&map->first;

	do {
		map->last->next = resize_entry(map, map->last->next);
		map->last		= map->last->next;
	} while (map->last->next != NULL);

	m_free(old_buckets, old_capacity * sizeof(struct bucket));
}

#define HASHMAP_HASH_INIT 2166136261u

static inline uint32_t hash_data(const unsigned char *data, size_t size)
{
	size_t nblocks = size / 8;
	uint64_t hash  = HASHMAP_HASH_INIT;
	for (size_t i = 0; i < nblocks; ++i) {
		hash ^= (uint64_t)data[0] << 0 | (uint64_t)data[1] << 8 | (uint64_t)data[2] << 16 | (uint64_t)data[3] << 24 | (uint64_t)data[4] << 32 | (uint64_t)data[5] << 40 |
				(uint64_t)data[6] << 48 | (uint64_t)data[7] << 56;
		hash *= 0xbf58476d1ce4e5b9;
		data += 8;
	}

	uint64_t last = size & 0xff;
	switch (size % 8) {
	case 7:
		last |= (uint64_t)data[6] << 56;
	case 6:
		last |= (uint64_t)data[5] << 48;
	case 5:
		last |= (uint64_t)data[4] << 40;
	case 4:
		last |= (uint64_t)data[3] << 32;
	case 3:
		last |= (uint64_t)data[2] << 24;
	case 2:
		last |= (uint64_t)data[1] << 16;
	case 1:
		last |= (uint64_t)data[0] << 8;
		hash ^= last;
		hash *= 0xd6e8feb86659fd93;
	}

	return (uint32_t)(hash ^ hash >> 32);
}

static struct bucket *find_entry(const hashmap_t *map, void *key, size_t ksize, uint32_t hash)
{
	uint32_t index = hash % map->capacity;

	for (;;) {
		struct bucket *entry = &map->buckets[index];

		if (entry->key == NULL || (entry->ksize == ksize && entry->hash == hash && memcmp(entry->key, key, ksize) == 0)) {
			return entry;
		}

		index = (index + 1) % map->capacity;
	}
}

void hashmap_set(hashmap_t *map, void *key, size_t ksize, void *val)
{
	if (map->count + 1 > HASHMAP_MAX_LOAD * map->capacity) {
		hashmap_resize(map);
	}

	uint32_t hash		 = hash_data(key, ksize);
	struct bucket *entry = find_entry(map, key, ksize, hash);
	if (entry->key == NULL) {
		map->last->next = entry;
		map->last		= entry;
		entry->next		= NULL;

		++map->count;

		entry->key	 = key;
		entry->ksize = ksize;
		entry->hash	 = hash;
	}
	entry->value = val;
}

int hashmap_get(const hashmap_t *map, void *key, size_t ksize, void **out_val)
{
	uint32_t hash		 = hash_data(key, ksize);
	struct bucket *entry = find_entry(map, key, ksize, hash);

	if (out_val != NULL) {
		*out_val = entry->value;
	}

	return entry->key != NULL;
}

void hashmap_iterate(hashmap_t *map, hashmap_callback callback, void *usr)
{
	struct bucket *current = map->first;

	int co = 0;

	while (current != NULL) {
		callback(current->key, current->ksize, current->value, usr);

		current = current->next;

		if (co > 1000) {
			break;
		}
		co++;
	}
}

void hashmap_iterate_c(const hashmap_t *map, hashmap_callback_c callback, const void *usr)
{
	struct bucket *current = map->first;

	int co = 0;

	while (current != NULL) {
		callback(current->key, current->ksize, current->value, usr);

		current = current->next;

		if (co > 1000) {
			break;
		}
		co++;
	}
}

void hashmap_iterate_hc(const hashmap_t *map, hashmap_callback_hc callback, void *usr)
{
	struct bucket *current = map->first;

	int co = 0;

	while (current != NULL) {
		callback(current->key, current->ksize, current->value, usr);

		current = current->next;

		if (co > 1000) {
			break;
		}
		co++;
	}
}
