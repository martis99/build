#ifndef MEM_H
#define MEM_H

#include <stddef.h>

void mem_init(size_t *mem_usage);
void *m_realloc(void *memory, size_t new_size, size_t old_size);
void *m_calloc(size_t count, size_t size);
void *m_malloc(size_t size);
void *m_cpy(void *dst, size_t dst_size, const void *src, size_t src_size);
int m_cmp(const void *l, const void *r, size_t size);
void m_free(void *memory, size_t size);

#endif
