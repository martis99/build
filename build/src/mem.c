#include "mem.h"

#include <memory.h>
#include <stdlib.h>

static size_t *s_mem_usage;

void mem_init(size_t *mem_usage) {
	s_mem_usage = mem_usage;
}

void *m_realloc(void *memory, size_t new_size, size_t old_size) {
	*s_mem_usage -= old_size;
	*s_mem_usage += new_size;
	return realloc(memory, new_size);
}

void *m_calloc(size_t count, size_t size) {
	*s_mem_usage += count * size;
	return calloc(count, size);
}

void *m_malloc(size_t size) {
	*s_mem_usage += size;
	void *ptr = malloc(size);
	if (ptr == NULL) {
		return NULL;
	}
	return memset(ptr, 0, size);
}

void m_free(void *memory, size_t size) {
	*s_mem_usage -= size;
	free(memory);
}