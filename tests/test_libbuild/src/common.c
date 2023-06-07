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

int test_gen(test_gen_fn fn, const test_gen_file_t *in, size_t in_size, const test_gen_file_t *out, size_t out_size)
{
	int ret = 0;

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

		FILE *f = file_open(in[i].path, "wb");
		p_fprintf(f, "%s", in[i].data);
		file_close(f);
	}

	path_t sln_dir = { 0 };
	if (path_init(&sln_dir, CSTR("tmp"))) {
		ret = 1;
	}

	sln_t sln = { 0 };

	if (sln_read(&sln, &sln_dir)) {
		ret = 1;
	}

	sln_print(&sln);

	path_t build_dir = { 0 };

	if (path_init(&build_dir, CSTR("tmp"))) {
		ret = 1;
	}

	if (fn(&sln, &build_dir)) {
		ret = 1;
	}

	sln_free(&sln);

	char act[1024 * 8] = { 0 };

	for (size_t i = 0; i < out_cnt; i++) {
		size_t len = file_read_t(out[i].path, CSTR(act));

		if (!cstr_cmp(act, len, out[i].data, cstr_len(out[i].data))) {
			printf("Actual:\n%s", act);
			printf("Expected:\n%s", out[i].data);
			ret = 1;
		}

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

	return ret;
}