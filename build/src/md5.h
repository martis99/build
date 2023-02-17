#ifndef MD5_H
#define MD5_H

#include <stddef.h>

int md5(const char *msg, size_t msg_len, unsigned char *buf, size_t buf_size, char *out, size_t out_size);

#endif
