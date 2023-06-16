#include "common.h"

#include "cstr.h"
#include "file.h"
#include "print.h"

static void delete_folder(const char *path)
{
	char *end = cstr_rchr(path, '/');
	if (end == NULL) {
		return;
	}

	*end = '\0';
	if (folder_exists(path)) {
		folder_delete(path);
	}
	delete_folder(path);
	*end = '/';
}

static int get_diff(const char *str1, size_t str1_len, const char *str2, size_t str2_len, uint *line, uint *col, uint *at)
{
	int l = 1;
	int c = 1;
	int i = 0;

	while (i < str1_len && i < str2_len) {
		if (str1[i] != str2[i]) {
			*line = l;
			*col  = c;
			*at   = i;
			return 1;
		}

		if (str1[i] == '\n') {
			l++;
			c = 1;
		}

		i++;
		c++;
	}

	return 0;
}

static void print(const char *str, uint len)
{
	for (uint i = 0; i < len; i++) {
		char c = str[i];
		switch (c) {
		case '\r':
			printf("\\r");
			break;
		case '\n':
			printf("\\n");
			break;
		default:
			printf("%c", c);
			break;
		}
	}
}

int test_gen(test_gen_fn fn, const test_gen_file_t *in, size_t in_size, const test_gen_file_t *out, size_t out_size)
{
	const size_t in_cnt  = in_size / sizeof(test_gen_file_t);
	const size_t out_cnt = out_size / sizeof(test_gen_file_t);

	for (size_t i = 0; i < in_cnt; i++) {
		char *end = (char *)in[i].path;

		while (end = cstr_chr(end, '/')) {
			*end = '\0';
			if (!folder_exists(in[i].path)) {
				folder_create(in[i].path);
			}
			*end = '/';
			end++;
		}

		FILE *f = file_open(in[i].path, "w");
		p_fprintf(f, "%s", in[i].data);
		file_close(f);
	}

	path_t sln_dir = { 0 };
	if (path_init(&sln_dir, CSTR("tmp"))) {
		return 1;
	}

	sln_t sln = { 0 };

	if (sln_read(&sln, &sln_dir)) {
		return 1;
	}

	sln_print(&sln);

	path_t build_dir = { 0 };

	if (path_init(&build_dir, CSTR("tmp"))) {
		return 1;
	}

	if (fn(&sln, &build_dir)) {
		return 1;
	}

	sln_free(&sln);

	char act[1024 * 8] = { 0 };

	for (size_t i = 0; i < out_cnt; i++) {
		size_t len = file_read_t(out[i].path, CSTR(act));
		if (len == -1) {
			printf("Failed to read '%s' file\n", out[i].path);
		}
		uint line = -1;
		uint col  = -1;
		uint at	  = -1;
		if (get_diff(act, len, out[i].data, cstr_len(out[i].data), &line, &col, &at)) {
			const uint plen = (unsigned long long)at + 10 > len ? 0 : 10;
			printf("%s:%d:%d '", out[i].path, line, col);
			print(&out[i].data[at], plen);
			printf("' != '");
			print(&act[at], plen);
			printf("'\n");

			return 1;
			break;
		}
	}

	for (size_t i = 0; i < out_cnt; i++) {
		file_delete(out[i].path);
	}

	for (size_t i = 0; i < in_cnt; i++) {
		file_delete(in[i].path);
	}

	for (size_t i = 0; i < out_cnt; i++) {
		delete_folder(out[i].path);
	}

	for (size_t i = 0; i < in_cnt; i++) {
		delete_folder(in[i].path);
	}

	return 0;
}