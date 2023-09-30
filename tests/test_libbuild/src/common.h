#ifndef COMMON_H
#define COMMON_H

#include "gen/sln.h"

#define MAX_DATA_CNT 16

typedef int (*test_gen_fn)(sln_t *sln, const path_t *path);
typedef void (*test_free_fn)(sln_t *sln);

typedef struct test_gen_data_s {
	const char *data;
} test_gen_data_t;

typedef struct test_gen_file_s {
	char path[P_MAX_PATH];
	test_gen_data_t data[MAX_DATA_CNT];
} test_gen_file_t;

int test_gen(test_gen_fn gen_fn, test_free_fn free_fn, const test_gen_file_t *in, size_t in_size, const test_gen_file_t *out, size_t out_size);

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

static test_gen_file_t c_args_in[] = {
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
			"ARGS: -D\n",
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

static test_gen_file_t os_in[] = {
	{
		.path = "tmp/Solution.txt",
		.data = "NAME: OS\n"
			"DIRS: os, toolchain\n"
			"PLATFORMS: x86_64, i386\n"
			"CONFIGS: Debug, Release\n",
	},
	{
		.path = "tmp/toolchain/gcc/Project.txt",
		.data = "NAME: gcc-13.1.0\n"
			"URL: http://ftp.gnu.org/gnu/gcc/gcc-13.1.0/\n"
			"FORMAT: tar.gz\n"
			"CONFIG: --disable-nls --disable-libssp --enable-languages=c --without-headers\n"
			"TARGET: all-gcc all-target-libgcc install-gcc install-target-libgcc \n"
			"OUTDIR: $(SLN_DIR)/bin/toolchain/$(PLATFORM)/gcc\n"
			"REQUIRE: g++, libgmp-dev, libmpfr-dev, libmpc-dev, texinfo\n"
			"EXPORT: TCC = $(OUTDIR)/bin/$(PLATFORM)-elf-gcc\n",
	},
	{
		.path = "tmp/os/boot/bin/Project.txt",
		.data = "NAME: boot-bin\n"
			"TYPE: BIN\n"
			"LANGS: ASM\n"
			"SOURCE: ../src\n"
			"FILENAME: boot\n"
			"DEFINES: ARCH=$(PLATFORM), FORMAT=BIN\n",
	},
	{
		.path = "tmp/os/boot/src/boot.asm",
		.data = "[org 0x7c00]\n",
	},
	{
		.path = "tmp/os/kernel/bin/Project.txt",
		.data = "NAME: kernel-bin\n"
			"TYPE: BIN\n"
			"LANGS: ASM, C\n"
			"SOURCE: ../src\n"
			"FILENAME: kernel\n"
			"CFLAGS: FREESTANDING\n"
			"CCFLAGS: -m$(BITS)\n"
			"DEPENDS: gcc-13.1.0\n"
			"DEFINES: ARCH=$(PLATFORM)\n",
	},
	{
		.path = "tmp/os/kernel/elf/Project.txt",
		.data = "NAME: kernel-elf\n"
			"TYPE: ELF\n"
			"LANGS: ASM, C\n"
			"SOURCE: ../src\n"
			"FILENAME: kernel\n"
			"CFLAGS: FREESTANDING\n"
			"CCFLAGS: -m$(BITS)\n"
			"DEPENDS: gcc-13.1.0\n"
			"DEFINES: ARCH=$(PLATFORM)\n",
	},
	{
		.path = "tmp/os/kernel/src/main.c",
		.data = "void _start()\n"
			"{\n"
			"}\n",
	},
	{
		.path = "tmp/os/image/disk/Project.txt",
		.data = "NAME: image-disk\n"
			"TYPE: BIN\n"
			"DEPENDS: boot-bin, kernel-bin, kernel-elf\n"
			"FILES: boot-bin, kernel-bin\n"
			"FILENAME: disk\n"
			"REQUIRE: qemu-system-x86\n"
			"RUN: qemu-system-$(PLATFORM) -hda $(TARGET)\n"
			"DRUN: qemu-system-$(PLATFORM) -s -S -hda $(TARGET)\n"
			"PROGRAM: kernel-elf\n",
	},
	{
		.path = "tmp/os/image/floppy/Project.txt",
		.data = "NAME: image-floppy\n"
			"TYPE: FAT12\n"
			"DEPENDS: boot-bin, kernel-bin, kernel-elf\n"
			"FILES: boot-bin, kernel-bin\n"
			"FILENAME: floppy\n"
			"REQUIRE: qemu-system-x86\n"
			"RUN: qemu-system-$(PLATFORM) -fda $(TARGET)\n"
			"DRUN: qemu-system-$(PLATFORM) -s -S -fda $(TARGET)\n"
			"PROGRAM: kernel-elf\n",
	},
};

#endif
