#ifndef COMMON_H
#define COMMON_H

#include "gen/sln.h"

#define MAX_DATA_CNT 16

typedef int (*test_gen_fn)(const sln_t *sln, const path_t *path);

typedef struct test_gen_data_s {
	const char *data;
} test_gen_data_t;

typedef struct test_gen_file_s {
	char path[P_MAX_PATH];
	test_gen_data_t data[MAX_DATA_CNT];
} test_gen_file_t;

int test_gen(test_gen_fn fn, const test_gen_file_t *in, size_t in_size, const test_gen_file_t *out, size_t out_size);

static test_gen_file_t c_small_in[] = {
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
		.data = "#include <stdio.h>\n"
			"int main() {\n"
			"\tprintf(\"Test\n\");\n"
			"\treturn 0;\n"
			"}\n",
	},
};

static test_gen_file_t cpp_small_in[] = {
	{
		.path = "tmp/Solution.txt",
		.data = "NAME: test\n"
			"LANGS: CPP\n"
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
		.path = "tmp/test/src/main.cpp",
		.data = "#include <iostream>\n"
			"using namespace std;\n"
			"int main() {\n"
			"\tstd::cout << \"Test\" << std::endl;\n"
			"\treturn 0;\n"
			"}\n",
	},
};

static test_gen_file_t c_depends_in[] = {
	{
		.path = "tmp/Solution.txt",
		.data = "NAME: test\n"
			"LANGS: C\n"
			"DIRS: libtest, test\n"
			"CONFIGS: Debug\n"
			"PLATFORMS: x64\n",
	},
	{
		.path = "tmp/libtest/Project.txt",
		.data = "NAME: libtest\n"
			"TYPE: LIB\n"
			"SOURCE: src\n",
	},
	{
		.path = "tmp/libtest/src/main.c",
		.data = "#include <stdio.h>\n"
			"int ltest_print() {\n"
			"\tprintf(\"Test\n\");\n"
			"\treturn 0;\n"
			"}\n",
	},
	{
		.path = "tmp/test/Project.txt",
		.data = "NAME: test\n"
			"TYPE: EXE\n"
			"SOURCE: src\n"
			"DEPENDS: libtest\n",
	},
	{
		.path = "tmp/test/src/main.c",
		.data = "extern int ltest_print();\n"
			"int main() {\n"
			"\treturn ltest_print();\n"
			"}\n",
	},
};

static test_gen_file_t c_include_in[] = {
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
			"SOURCE: src\n"
			"INCLUDE: include\n",
	},
	{
		.path = "tmp/test/src/main.c",
		.data = "#include \"utils.h\"\n"
			"int main() {\n"
			"\treturn utils_print();\n"
			"}\n",
	},
	{
		.path = "tmp/test/src/utils.c",
		.data = "#include <stdio.h>\n"
			"int utils_print() {\n"
			"\tprintf(\"Test\n\");\n"
			"\treturn 0;\n"
			"}\n",
	},
	{
		.path = "tmp/test/include/utils.h",
		.data = "#ifndef UTILS_H\n"
			"#define UTILS_H\n"
			"int utils_print();\n"
			"#endif\n",
	},
};

#endif
