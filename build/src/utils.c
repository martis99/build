#include "utils.h"

#include "defines.h"
#include "mem.h"
#include "print.h"

#if defined(B_LINUX)
	#include <dirent.h>
	#include <sys/stat.h>
	#include <errno.h>
#endif

#include <string.h>

unsigned int cstr_len(const char *str)
{
	unsigned int len = 0;
	while (str[len++])
		;
	return len - 1;
}

int cstr_cmp(const char *str1, unsigned int str1_len, const char *str2, unsigned int str2_len)
{
	return str1_len == str2_len && m_cmp(str1, str2, str1_len * sizeof(char)) == 0;
}

int cstrn_cmp(const char *str1, unsigned int str1_len, const char *str2, unsigned int str2_len, unsigned int len)
{
	return str1_len >= len && str2_len >= len && m_cmp(str1, str2, len * sizeof(char)) == 0;
}

void *cstr_cpy(char *dst, unsigned int dst_len, const char *src, unsigned int src_len)
{
	return m_cpy(dst, dst_len, src, src_len * sizeof(char));
}

int cstr_replace(const char *src, unsigned int src_len, char *dst, unsigned int dst_len, const char *from, unsigned int from_len, const char *to, unsigned int to_len)
{
	src_len	 = src_len == 0 ? cstr_len(src) : src_len;
	from_len = from_len == 0 ? cstr_len(from) : from_len;
	to_len	 = to_len == 0 ? cstr_len(to) : to_len;

	unsigned int dst_i = 0;
	for (unsigned int i = 0; i < src_len; i++) {
		if (src_len - i > from_len && cstr_cmp(&src[i], from_len, from, from_len)) {
			cstr_cpy(&dst[dst_i], dst_len, to, to_len);
			i += from_len - 1;
			dst_i += to_len;
		} else {
			dst[dst_i++] = src[i];
		}
	}

	return dst_i;
}

int cstr_replaces(const char *src, unsigned int src_len, char *dst, unsigned int dst_len, const char *const *from, const char *const *to, unsigned int len)
{
	src_len = src_len == 0 ? cstr_len(src) : src_len;

	unsigned int dst_i = 0;
	for (unsigned int i = 0; i < src_len; i++) {
		int found = 0;

		for (unsigned int j = 0; j < len; j++) {
			if (!from[j] || !to[j]) {
				continue;
			}

			unsigned int from_len = cstr_len(from[j]);
			unsigned int to_len   = cstr_len(to[j]);

			if (src_len - i >= from_len && cstr_cmp(&src[i], from_len, from[j], from_len)) {
				cstr_cpy(&dst[dst_i], dst_len, to[j], to_len);
				i += from_len - 1;
				dst_i += to_len;
				found = 1;
				break;
			}
		}

		if (!found) {
			dst[dst_i++] = src[i];
		}
	}

	return dst_i;
}

int cstr_format_v(char *buf, size_t buf_len, const char *format, va_list args)
{
	if (buf == NULL) {
		ERR("%s", "buffer is null");
		return -1;
	}

	if (format == NULL) {
		ERR("%s", "format is null");
		return -1;
	}

	if (buf_len <= 0) {
		ERR("%s", "buffer length is less or equal to zero");
		return -1;
	}

	int len = p_vsnprintf(buf, buf_len, format, args);
	if (len < 0) {
		ERR("%s", "failed to format");
		return -1;
	}

	if (len >= buf_len) {
		ERR("string too long: %s", buf);
		return -1;
	}

	return len;
}

int cstr_format_f(char *buf, size_t buf_len, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int len = cstr_format_v(buf, buf_len, format, args);
	va_end(args);
	return len;
}

FILE *file_open(const char *path, const char *mode, int exists)
{
	if (path == NULL) {
		ERR("%s", "path is null");
		return NULL;
	}

	if (mode == NULL) {
		ERR("%s", "mode is null");
		return NULL;
	}

	DBG("opening file: %s", path);

	FILE *file = NULL;

#if defined(B_WIN)
	fopen_s(&file, path, mode);
#else
	file = fopen(path, mode);
#endif

	if (file == NULL) {
		if (exists) {
			ERR("failed to open file: %s", path);
		} else {
			DBG("file not exists: %s", path);
		}
		return NULL;
	}

	DBG("file opened: %s", path);

	return file;
}

FILE *file_open_v(const char *format, const char *mode, int exists, va_list args)
{
	char path[B_MAX_PATH] = { 0 };

	if (cstr_format_v(path, sizeof(path) / sizeof(char), format, args) == -1) {
		return NULL;
	}

	return file_open(path, mode, exists);
}

FILE *file_open_f(const char *format, const char *mode, int exists, ...)
{
	va_list args;
	va_start(args, exists);
	FILE *file = file_open_v(format, mode, exists, args);
	va_end(args);
	return file;
}

size_t file_read(const char *path, int exists, char *data, size_t data_len)
{
	if (data == NULL) {
		ERR("%s", "data is null");
		return -1;
	}

	FILE *file = file_open(path, "r", exists);
	if (file == NULL) {
		return -1;
	}

	fseek(file, 0L, SEEK_END);
	size_t len = ftell(file);
	fseek(file, 0L, SEEK_SET);

	DBG("file length: %s %zu/%zu", path, len, data_len);

	if (len >= data_len) {
		ERR("file too large: %s", path);
		fclose(file);
		DBG("file closed: %s", path);
		return -1;
	}

	DBG("reading file: %s", path);

#if defined(B_WIN)
	fread_s(data, data_len, sizeof(char), len, file);
#else
	fread(data, sizeof(char), len, file);
#endif

	unsigned int dst = 0;
	for (unsigned int i = 0; i < len; i++) {
		data[dst] = data[i];
		if (data[i] != '\r') {
			dst++;
		}
	}
	len = dst;

	fclose(file);
	DBG("file closed: %s", path);
	return len;
}

size_t file_read_v(const char *format, int exists, char *data, size_t data_len, va_list args)
{
	char path[B_MAX_PATH] = { 0 };

	if (cstr_format_v(path, sizeof(path) / sizeof(char), format, args) == -1) {
		return -1;
	}

	return file_read(path, exists, data, data_len);
}

size_t file_read_f(const char *format, int exists, char *data, size_t data_len, ...)
{
	va_list args;
	va_start(args, data_len);
	size_t len = file_read_v(format, exists, data, data_len, args);
	va_end(args);
	return len;
}

int file_exists(const char *path)
{
#if defined(B_WIN)
	int dwAttrib = GetFileAttributesA(path);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat buffer;
	if (stat(path, &buffer)) {
		return 0;
	} else {
		return 1;
	}
#endif
}

int file_exists_v(const char *format, va_list args)
{
	char path[B_MAX_PATH] = { 0 };

	if (cstr_format_v(path, sizeof(path) / sizeof(char), format, args) == -1) {
		return 0;
	}

	return file_exists(path);
}

int file_exists_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int exists = file_exists_v(format, args);
	va_end(args);
	return exists;
}

int folder_exists(const char *path)
{
#if defined(B_WIN)
	int dwAttrib = GetFileAttributesA(path);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	DIR *dir = opendir(path);
	if (dir) {
		closedir(dir);
		return 1;
	} else {
		return 0;
	}
#endif
}

int folder_exists_v(const char *format, va_list args)
{
	char path[B_MAX_PATH] = { 0 };

	if (cstr_format_v(path, sizeof(path) / sizeof(char), format, args) == -1) {
		return 0;
	}

	return folder_exists(path);
}

int folder_exists_f(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int exists = folder_exists_v(format, args);
	va_end(args);
	return exists;
}

int folder_create(const char *path)
{
#if defined(B_WIN)
	return CreateDirectoryA(path, NULL) || ERROR_ALREADY_EXISTS == GetLastError();
#else
	struct stat st = { 0 };
	if (stat(path, &st) == -1) {
		mkdir(path, 0700);
	}
	return 1;
#endif
}

int files_foreach(const path_t *path, files_foreach_cb on_folder, files_foreach_cb on_file, void *priv)
{
#if defined(B_WIN)
	WIN32_FIND_DATA file = { 0 };
	HANDLE find	     = NULL;

	path_t child_path = *path;
	path_child(&child_path, "*.*", 3);

	if ((find = FindFirstFileA(child_path.path, &file)) == INVALID_HANDLE_VALUE) {
		ERR("folder not found: %s\n", child_path.path);
		return 1;
	}
	child_path.len = path->len;

	do {
		if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
			continue;
		}

		files_foreach_cb cb = file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? on_folder : on_file;
		if (cb) {
			if (path_child(&child_path, file.cFileName, cstr_len(file.cFileName))) {
				child_path.len = path->len;
				continue;
			}

			int ret = cb(&child_path, file.cFileName, priv);

			child_path.len = path->len;
			if (ret < 0) {
				return ret;
			}
		}
	} while (FindNextFileA(find, &file));

	FindClose(find);

#else

	DIR *dir = opendir(path->path);
	if (!dir) {
		ERR("folder not found: %.*s\n", path->len, path->path);
		return 1;
	}

	path_t child_path = *path;
	struct dirent *dp;

	while ((dp = readdir(dir))) {
		struct stat stbuf = { 0 };

		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			goto next;
		}

		if (path_child(&child_path, dp->d_name, cstr_len(dp->d_name))) {
			goto next;
		}

		if (stat(child_path.path, &stbuf)) {
			ERR("failed to check file: %.*s\n", child_path.len, child_path.path);
			goto next;
		}

		files_foreach_cb cb = folder_exists(child_path.path) ? on_folder : on_file;
		if (cb) {
			int ret = cb(&child_path, dp->d_name, priv);

			child_path.len = path->len;
			if (ret < 0) {
				return ret;
			}
		}
next:
		child_path.len = path->len;
	}

#endif

	return 0;
}

int path_init(path_t *path, const char *dir, unsigned int len)
{
	if (len + 1 > B_MAX_PATH) {
		ERR("%s", "path too long");
		return 1;
	}

	m_cpy(path->path, len, dir, len);
	path->len = (unsigned int)len;

	path->path[path->len] = '\0';

	return 0;
}

int path_child_s(path_t *path, const char *dir, unsigned int len, char s)
{
	if (path->len + len + 2 > B_MAX_PATH) {
		ERR("%s", "path too long");
		return 1;
	}

	if (s != 0) {
		path->path[path->len++] = s;
	}
	m_cpy(path->path + path->len, len, dir, len);
	path->len += (unsigned int)len;

	path->path[path->len] = '\0';

	return 0;
}

int path_child(path_t *path, const char *dir, unsigned int len)
{
	char c;
#if defined(B_WIN)
	c = '\\';
#else
	c = '/';
#endif
	return path_child_s(path, dir, len, c);
}

int path_parent(path_t *path)
{
	unsigned int len = path->len;

	while (len > 0 && path->path[len] != '\\' && path->path[len] != '/')
		len--;

	if (len == 0) {
		return 1;
	}

	path->len = len;

	path->path[path->len] = '\0';

	return 0;
}

int path_set_len(path_t *path, unsigned int len)
{
	path->len	      = len;
	path->path[path->len] = '\0';
	return 0;
}

int path_ends(const path_t *path, const char *str, unsigned int len)
{
	return path->len > len && m_cmp(path->path + path->len - len, str, (size_t)len) == 0;
}

int path_calc_rel(const char *path, unsigned int path_len, const char *dest, unsigned int dest_len, path_t *out)
{
	int dif	       = 0;
	int last_slash = 0;

	for (unsigned int i = 0; i < path_len; i++) {
		if (dif == 0 && (path[i] == '\\' || path[i] == '/')) {
			last_slash = i;
		}

		if (dif && (path[i] == '\\' || path[i] == '/')) {
			dif++;
		}

		if (dif == 0 && (i > dest_len || path[i] != dest[i])) {
			dif = 1;
		}
	}

	out->len = 0;
	for (int i = 0; i < dif; i++) {
		out->path[out->len++] = '.';
		out->path[out->len++] = '.';
		out->path[out->len++] = '\\';
	}

	if (out->len) {
		out->len--;
	}

	path_child(out, &dest[last_slash + 1], dest_len - last_slash - 1);
	return 0;
}

int pathv_path(pathv_t *pathv, const path_t *path)
{
	*pathv = (pathv_t){ .path = path->path, .len = path->len };
	return 0;
}

int pathv_sub(pathv_t *pathv, const path_t *l, const path_t *r)
{
	*pathv = (pathv_t){
		.path = l->path + r->len + 1,
		.len  = l->len - r->len - 1,
	};
	return 0;
}

int pathv_folder(pathv_t *pathv, const path_t *path)
{
	unsigned int len = path->len;

	while (len > 0 && path->path[len] != '\\' && path->path[len] != '/')
		len--;

	*pathv = (pathv_t){
		.path = path->path + len + 1,
		.len  = path->len - len - 1,
	};

	return 0;
}

int read_char(prop_str_t *str, char c)
{
	if (str->cur >= str->len) {
		return 0;
	}

	unsigned int start = str->cur;
	if (str->data[str->cur] == c) {
		str->cur++;
	}

	return str->cur - start;
}

int read_name(prop_str_t *str)
{
	if (str->cur >= str->len) {
		return 0;
	}

	unsigned int start = str->cur;
	char c		   = str->data[str->cur];
	while (str->cur < str->len && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) {
		c = str->data[++str->cur];
	}

	return str->cur - start;
}

int read_path(prop_str_t *str, prop_str_t *dst)
{
	if (str->cur >= str->len) {
		return 0;
	}

	unsigned int start = str->cur;
	char c		   = str->data[str->cur];
	while (str->cur < str->len && (c == '$' || c == '(' || c == ')' || c == '-' || c == '/' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || c == '\\' ||
				       c == '_' || (c >= 'a' && c <= 'z'))) {
		c = str->data[++str->cur];
	}

	if (dst != NULL) {
		*dst = (prop_str_t){
			.path	    = str->path,
			.data	    = &str->data[start],
			.start	    = start,
			.len	    = str->cur - start,
			.line	    = str->line,
			.line_start = str->line_start,
		};
	}

	return str->cur - start;
}

int read_upper(prop_str_t *str, prop_str_t *dst)
{
	if (str->cur >= str->len) {
		return 0;
	}

	unsigned int start = str->cur;
	char c		   = str->data[str->cur];
	while (str->cur < str->len && (c >= 'A' && c <= 'Z')) {
		c = str->data[++str->cur];
	}

	if (dst != NULL) {
		*dst = (prop_str_t){
			.path	    = str->path,
			.data	    = &str->data[start],
			.start	    = start,
			.len	    = str->cur - start,
			.line	    = str->line,
			.line_start = str->line_start,
		};
	}

	return str->cur - start;
}

int read_printable(prop_str_t *str)
{
	if (str->cur >= str->len) {
		return 0;
	}

	unsigned int start = str->cur;
	char c		   = str->data[str->cur];
	while (str->cur < str->len && c >= ' ' && c <= '~') {
		c = str->data[++str->cur];
	}

	return str->cur - start;
}

void convert_slash(char *dst, unsigned int dst_len, const char *src, size_t len)
{
	m_cpy(dst, dst_len, src, len);
	for (int i = 0; i < len; i++) {
		if (dst[i] == '\\') {
			dst[i] = '/';
		}
	}
}
