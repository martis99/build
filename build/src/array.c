#include "array.h"

#include "mem.h"

#include <stdlib.h>
#include <string.h>

#define ARRAY_MAX_LOAD	    0.75f
#define ARRAY_RESIZE_FACTOR 2

void array_init(array_t *arr, int capacity, size_t vsize)
{
	arr->capacity = capacity;
	arr->count    = 0;
	arr->vsize    = vsize;

	arr->data = m_calloc(capacity, vsize);
}

void array_free(array_t *arr)
{
	m_free(arr->data, arr->capacity * arr->vsize);
}

static void array_resize(array_t *arr)
{
	int old_capacity = arr->capacity;

	arr->capacity *= ARRAY_RESIZE_FACTOR;
	arr->data = m_realloc(arr->data, arr->capacity * arr->vsize, old_capacity * arr->vsize);
}

void *array_add(array_t *arr, const void *value)
{
	if (arr->count + 1 > ARRAY_MAX_LOAD * arr->capacity) {
		array_resize(arr);
	}

	void *ptr = (void *)((uintptr_t)arr->data + arr->vsize * arr->count++);
	memcpy_s(ptr, arr->vsize, value, arr->vsize);
	return ptr;
}

void array_set(array_t *arr, int index, const void *value)
{
	memcpy_s((void *)((uintptr_t)arr->data + arr->vsize * index), arr->vsize, value, arr->vsize);
}

void *array_get(const array_t *arr, int index)
{
	return (void *)((uintptr_t)arr->data + arr->vsize * index);
}

void array_iterate(array_t *arr, array_callback callback, void *priv)
{
	for (int i = 0; i < arr->count; i++) {
		callback(i, (void *)((uintptr_t)arr->data + arr->vsize * i), priv);
	}
}

int array_index(const array_t *arr, const void *value)
{
	for (int i = 0; i < arr->count; i++) {
		if (memcmp((void *)((uintptr_t)arr->data + arr->vsize * i), value, arr->vsize) == 0) {
			return i;
		}
	}

	return -1;
}

int array_index_cb(const array_t *arr, const void *value, array_index_callback callback)
{
	for (int i = 0; i < arr->count; i++) {
		if (callback((void *)((uintptr_t)arr->data + arr->vsize * i), value)) {
			return i;
		}
	}

	return -1;
}
