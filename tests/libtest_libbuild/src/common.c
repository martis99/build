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

static int get_diff(const char *act, size_t act_len, const test_gen_data_t exps[MAX_DATA_CNT], uint *p_line, const char **act_ptr, const char **exp_ptr)
{
	int line       = 1;
	size_t act_pos = 0;
	const char *exp;

	*act_ptr = &act[act_pos];

	for (int i = 0; i < MAX_DATA_CNT && (exp = exps[i].data); i++) {
		size_t exp_len = cstr_len(exp);
		size_t exp_pos = 0;

		*exp_ptr = &exp[exp_pos];

		while (act_pos < act_len && exp_pos < exp_len) {
			if (act[act_pos] != exp[exp_pos]) {
				*p_line = line;
				return 1;
			}

			if (act[act_pos] == '\n') {
				line++;
				*act_ptr = &act[act_pos + 1];
				*exp_ptr = &exp[exp_pos + 1];
			}

			act_pos++;
			exp_pos++;
		}
	}

	return 0;
}

static void print(const char *str)
{
	const uint len = (uint)(cstr_chr(str, '\n') - str);

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

int test_gen(test_gen_fn gen_fn, test_free_fn free_fn, const test_gen_file_t *in, size_t in_size, const test_gen_file_t *out, size_t out_size)
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
		for (int j = 0; j < MAX_DATA_CNT && in[i].data[j].data; j++) {
			c_fprintf(f, "%s", in[i].data[j].data);
		}
		file_close(f);
	}

	path_t sln_dir = { 0 };
	if (path_init(&sln_dir, CSTR("tmp")) == NULL) {
		return 1;
	}

	sln_t sln = { 0 };

	if (sln_read(&sln, &sln_dir)) {
		return 1;
	}

	sln_print(&sln);

	path_t build_dir = { 0 };

	if (path_init(&build_dir, CSTR("tmp")) == NULL) {
		return 1;
	}

	if (gen_fn(&sln, &build_dir)) {
		return 1;
	}

	if (free_fn) {
		free_fn(&sln);
	}

	sln_free(&sln);

	int ret = 0;

	char act[1024 * 64] = { 0 };

	for (size_t i = 0; i < out_cnt; i++) {
		const test_gen_file_t *file = &out[i];

		size_t act_len = file_read_t(file->path, CSTR(act));
		if (act_len <= 0) {
			printf("Failed to read '%s' file\n", file->path);
			ret = 1;
			continue;
		}

		uint line = -1;

		const char *act_ptr = NULL;
		const char *exp_ptr = NULL;

		if (get_diff(act, act_len, file->data, &line, &act_ptr, &exp_ptr)) {
			printf("%s:%d '", file->path, line);
			print(act_ptr);
			printf("' != '");
			print(exp_ptr);
			printf("'\n");
			ret = 1;
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

	return ret;
}
