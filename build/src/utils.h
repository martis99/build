#ifndef UTILS_H
#define UTILS_H

#include "utils_types.h"

#include <stdarg.h>
#include <stdio.h>

unsigned int cstr_len(const char *str);
int cstr_cmp(const char *str1, unsigned int str1_len, const char *str2, unsigned int str2_len);
int cstrn_cmp(const char *str1, unsigned int str1_len, const char *str2, unsigned int str2_len, unsigned int len);
void *cstr_cpy(char *dst, const char *src, unsigned int len);
int cstr_replace(const char *src, unsigned int src_len, char *dst, unsigned int dst_len, const char *from, unsigned int from_len, const char *to, unsigned int to_len);
int cstr_replaces(const char *src, unsigned int src_len, char *dst, unsigned int dst_len, const char *const *from, const char *const *to, unsigned int len);

int cstr_format_v(char *buf, size_t buf_len, const char *format, va_list args);
int cstr_format_f(char *buf, size_t buf_len, const char *format, ...);

FILE *file_open(const char *path, const char *mode, int exists);
FILE *file_open_v(const char *format, const char *mode, int exists, va_list args);
FILE *file_open_f(const char *format, const char *mode, int exists, ...);

size_t file_read(const char *path, int exists, char *data, size_t data_len);
size_t file_read_v(const char *format, int exists, char *data, size_t data_len, va_list args);
size_t file_read_f(const char *format, int exists, char *data, size_t data_len, ...);

int file_exists(const char *path);
int file_exists_v(const char *format, va_list args);
int file_exists_f(const char *format, ...);

int folder_exists(const char *path);
int folder_exists_v(const char *format, va_list args);
int folder_exists_f(const char *format, ...);
int folder_create(const char *path);

typedef int (*files_foreach_cb)(path_t *path, const char *folder, void *usr);
int files_foreach(const path_t *path, files_foreach_cb on_folder, files_foreach_cb on_file, void *usr);

int path_init(path_t *path, const char *dir, unsigned int len);
int path_child_s(path_t *path, const char *dir, unsigned int len, char s);
int path_child(path_t *path, const char *dir, unsigned int len);
int path_parent(path_t *path);
int path_set_len(path_t *path, unsigned int len);
int path_ends(const path_t *path, const char *str);
int path_calc_rel(const char *path, unsigned int path_len, const char *dest, unsigned int dest_len, path_t *out);
;

int pathv_path(pathv_t *pathv, const path_t *path);
int pathv_sub(pathv_t *pathv, const path_t *l, const path_t *r);
int pathv_folder(pathv_t *pathv, const path_t *path);

int read_char(prop_str_t *str, char c);
int read_name(prop_str_t *str);
int read_path(prop_str_t *str, prop_str_t *dst);
int read_upper(prop_str_t *str, prop_str_t *dst);
int read_printable(prop_str_t *str);

#endif
