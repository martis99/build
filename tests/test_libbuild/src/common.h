#ifndef COMMON_H
#define COMMON_H

#include "gen/sln.h"

typedef int (*test_gen_fn)(const sln_t *sln, const path_t *path);

typedef struct test_gen_file_s {
	char path[P_MAX_PATH];
	const char *data;
} test_gen_file_t;

int test_gen(test_gen_fn fn, const test_gen_file_t *in, size_t in_size, const test_gen_file_t *out, size_t out_size);

static test_gen_file_t simple_in[] = {
	{
		.path = "tmp/Solution.txt",
		.data = "NAME: test\n"
			"LANGS: C\n"
			"DIRS: test\n"
			"CONFIGS: Debug\n"
			"PLATFORMS: x64\n",
	},
	{
		.path = "tmp/test/Project.txt",
		.data = "NAME: test\n"
			"TYPE: EXE\n"
			"SOURCE: src\n",
	},
	{
		.path = "tmp/test/src/main.c",
		.data = "int main() {\n"
			"\treturn 0;\n"
			"}\n",
	},
};

#endif
