#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <stddef.h>

typedef struct array_s {
	int capacity;
	int count;
	size_t vsize;
	void *data;
} array_t;


void array_init(array_t *arr, int capacity, size_t vsize);

void array_free(array_t *arr);

void *array_add(array_t *arr, const void *value);

void array_set(array_t *arr, int index, const void *value);

void *array_get(const array_t *arr, int index);

typedef void (*array_callback)(int index, void *value, void *usr);
void array_iterate(array_t *arr, array_callback callback, void *usr);

int array_index(const array_t *arr, const void *value);

typedef int (*array_index_callback)(const void *arr_value, const void *value);
int array_index_cb(const array_t *arr, const void *value, array_index_callback callback);

#endif