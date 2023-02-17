#ifndef hash_map_h
#define hash_map_h

#include <stddef.h>
#include <stdint.h>

struct bucket {
	struct bucket *next;

	void *key;
	size_t ksize;
	uint32_t hash;
	void *value;
};

typedef struct hashmap_s {
	struct bucket *buckets;
	int capacity;
	int count;

	struct bucket *first;
	struct bucket *last;
} hashmap_t;

typedef void (*hashmap_callback)(void *key, size_t ksize, void *value, void *priv);
typedef void (*hashmap_callback_c)(void *key, size_t ksize, void *value, const void *priv);
typedef void (*hashmap_callback_hc)(void *key, size_t ksize, void *value, void *priv);

void hashmap_create(hashmap_t *map, int capacity);

void hashmap_free(hashmap_t *map);

void hashmap_set(hashmap_t *map, void *key, size_t ksize, void *value);

int hashmap_get(const hashmap_t *map, void *key, size_t ksize, void **out_val);

void hashmap_iterate(hashmap_t *map, hashmap_callback callback, void *priv);
void hashmap_iterate_c(const hashmap_t *map, hashmap_callback_c callback, const void *priv);
void hashmap_iterate_hc(const hashmap_t *map, hashmap_callback_hc callback, void *priv);

#endif
