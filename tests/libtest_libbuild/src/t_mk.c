#include "gen/mk/mk_sln.h"

#include "common.h"
#include "mk_vc.h"
#include "test.h"

static const char *SLN_LIBTEST =
	"SLNDIR := $(CURDIR)/\n"
	"TLD := $(LD)\n"
	"TCC := $(CC)\n"
	"\n"
	"export\n"
	"\n"
	"CONFIGS := Debug\n"
	"CONFIG := Debug\n"
	"\n"
	"SHOW := true\n"
	"\n"
	".PHONY: all check libtest/clean libtest/static libtest/dynamic libtest/coverage_static libtest/coverage_dynamic test/clean test/compile test/run test/coverage clean\n"
	"\n"
	"all: libtest/static libtest/dynamic test/compile\n"
	"\n"
	"check:\n"
	"ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
	"\t$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
	"endif\n"
	"\n"
	"libtest/clean: check\n"
	"\t@$(MAKE) -C libtest clean\n"
	"\n"
	"libtest/static: check\n"
	"\t@$(MAKE) -C libtest static\n"
	"\n"
	"libtest/dynamic: check\n"
	"\t@$(MAKE) -C libtest dynamic\n"
	"\n"
	"libtest/coverage_static: check\n"
	"\t@$(MAKE) -C libtest coverage_static\n"
	"\n"
	"libtest/coverage_dynamic: check\n"
	"\t@$(MAKE) -C libtest coverage_dynamic\n"
	"\n"
	"test/clean: check libtest/clean\n"
	"\t@$(MAKE) -C test clean\n"
	"\n"
	"test/compile: check libtest/static\n"
	"\t@$(MAKE) -C test compile\n"
	"\n"
	"test/run: check test/compile\n"
	"\t@$(MAKE) -C test run\n"
	"\n"
	"test/coverage: check libtest/coverage_static\n"
	"\t@$(MAKE) -C test coverage\n"
	"\n"
	"clean: check\n"
	"\t@$(MAKE) -C libtest clean\n"
	"\t@$(MAKE) -C test clean\n";

static const char *PROJ_TEST_C_COMPILE = "RM += -r\n"
					 "\n"
					 "CONFIG_FLAGS :=\n"
					 "\n"
					 "ifeq ($(CONFIG), Debug)\n"
					 "CONFIG_FLAGS += -ggdb3 -O0\n"
					 "endif\n"
					 "\n"
					 "SHOW := true\n"
					 "\n"
					 ".PHONY: all check check_coverage compile coverage run clean\n"
					 "\n"
					 "all: compile\n"
					 "\n"
					 "check:\n"
					 "ifeq (, $(shell which gcc))\n"
					 "\tsudo apt install gcc -y\n"
					 "endif\n"
					 "\n"
					 "check_coverage: check\n"
					 "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
					 "ifeq (, $(shell which lcov))\n"
					 "\tsudo apt install lcov -y\n"
					 "endif\n"
					 "\n"
					 "compile: check $(TARGET)\n"
					 "\n"
					 "coverage: clean check_coverage $(TARGET)\n"
					 "\t@$(TARGET) $(ARGS)\n"
					 "\t@lcov -q -c -d $(SLNDIR) -o $(LCOV)\n"
					 "ifeq ($(SHOW), true)\n"
					 "\t@genhtml -q $(LCOV) -o $(REPDIR)\n"
					 "\t@open $(REPDIR)index.html\n"
					 "endif\n"
					 "\n"
					 "$(TARGET): $(OBJ_C)\n"
					 "\t@mkdir -p $(@D)\n"
					 "\t@$(TCC) $(CONFIG_FLAGS) -o $@ $^ $(LDFLAGS)\n"
					 "\n"
					 "$(INTDIR)%.o: %.c\n"
					 "\t@mkdir -p $(@D)\n"
					 "\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) $(DEFINES_STATIC) -c -o $@ $<\n"
					 "\n"
					 "run: check $(TARGET)\n"
					 "\t@$(TARGET) $(ARGS)\n"
					 "\n"
					 "clean:\n"
					 "\t@$(RM) $(TARGET) $(OBJ_C) $(COV)\n";

static const char *PROJ_LIBTEST_COMPILE = "RM += -r\n"
					  "\n"
					  "CONFIG_FLAGS :=\n"
					  "\n"
					  "ifeq ($(CONFIG), Debug)\n"
					  "CONFIG_FLAGS += -ggdb3 -O0\n"
					  "endif\n"
					  "\n"
					  ".PHONY: all check check_coverage static dynamic coverage_static coverage_dynamic clean\n"
					  "\n"
					  "all: static dynamic\n"
					  "\n"
					  "check:\n"
					  "ifeq (, $(shell which gcc))\n"
					  "\tsudo apt install gcc -y\n"
					  "endif\n"
					  "\n"
					  "check_coverage: check\n"
					  "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
					  "\n"
					  "static: check $(TARGET_STATIC)\n"
					  "\n"
					  "dynamic: check $(TARGET_DYNAMIC)\n"
					  "\n"
					  "coverage_static: clean check_coverage $(TARGET_STATIC)\n"
					  "\n"
					  "coverage_dynamic: clean check_coverage $(TARGET_DYNAMIC)\n"
					  "\n"
					  "$(TARGET_STATIC): $(OBJ_C)\n"
					  "\t@mkdir -p $(@D)\n"
					  "\t@ar rcs $@ $^\n"
					  "\n"
					  "$(TARGET_DYNAMIC): $(OBJ_D_C)\n"
					  "\t@mkdir -p $(@D)\n"
					  "\t@$(TCC) $^ -shared -o $@ $(LDFLAGS)\n"
					  "\n"
					  "$(INTDIR)%.o: %.c\n"
					  "\t@mkdir -p $(@D)\n"
					  "\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) $(DEFINES_STATIC) -c -o $@ $<\n"
					  "\n"
					  "$(INTDIR)%.d.o: %.c\n"
					  "\t@mkdir -p $(@D)\n"
					  "\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) $(DEFINES_DYNAMIC) -fPIC -c -o $@ $<\n"
					  "\n"
					  "clean:\n"
					  "\t@$(RM) $(TARGET_STATIC) $(TARGET_DYNAMIC) $(OBJ_C) $(OBJ_D_C) $(COV)\n";

static const char *PROJ_TEST_CPP_COMPILE = "RM += -r\n"
					   "\n"
					   "CONFIG_FLAGS :=\n"
					   "\n"
					   "ifeq ($(CONFIG), Debug)\n"
					   "CONFIG_FLAGS += -ggdb3 -O0\n"
					   "endif\n"
					   "\n"
					   "SHOW := true\n"
					   "\n"
					   ".PHONY: all check check_coverage compile coverage run clean\n"
					   "\n"
					   "all: compile\n"
					   "\n"
					   "check:\n"
					   "ifeq (, $(shell which gcc))\n"
					   "\tsudo apt install gcc -y\n"
					   "endif\n"
					   "\n"
					   "check_coverage: check\n"
					   "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
					   "ifeq (, $(shell which lcov))\n"
					   "\tsudo apt install lcov -y\n"
					   "endif\n"
					   "\n"
					   "compile: check $(TARGET)\n"
					   "\n"
					   "coverage: clean check_coverage $(TARGET)\n"
					   "\t@$(TARGET) $(ARGS)\n"
					   "\t@lcov -q -c -d $(SLNDIR) -o $(LCOV)\n"
					   "ifeq ($(SHOW), true)\n"
					   "\t@genhtml -q $(LCOV) -o $(REPDIR)\n"
					   "\t@open $(REPDIR)index.html\n"
					   "endif\n"
					   "\n"
					   "$(TARGET): $(OBJ_CPP)\n"
					   "\t@mkdir -p $(@D)\n"
					   "\t@$(TCC) $(CONFIG_FLAGS) -o $@ $^ $(LDFLAGS)\n"
					   "\n"
					   "$(INTDIR)%.o: %.cpp\n"
					   "\t@mkdir -p $(@D)\n"
					   "\t@$(TCC) $(CONFIG_FLAGS) $(CXXFLAGS) $(DEFINES_STATIC) -c -o $@ $<\n"
					   "\n"
					   "run: check $(TARGET)\n"
					   "\t@$(TARGET) $(ARGS)\n"
					   "\n"
					   "clean:\n"
					   "\t@$(RM) $(TARGET) $(OBJ_CPP) $(COV)\n";

TEST(c_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/Makefile",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/Makefile",
			.data = {
				"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
				"REPDIR := $(OUTDIR)coverage-report/\n"
				"DEPS := $(shell find src -name '*.h')\n"
				"SRC_C := $(shell find src -name '*.c')\n"
				"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
				"LCOV := $(OUTDIR)lcov.info\n"
				"COV := $(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))\n"
				"COV += $(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))\n"
				"COV += $(LCOV) $(REPDIR)\n"
				"TARGET := $(OUTDIR)test\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS) -Wall -Wextra -Werror -pedantic\n"
				"DEFINES_STATIC :=\n"
				"LDFLAGS +=\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},

		},
	};

	const int ret = test_gen(mk_sln_gen, mk_sln_free, c_small_in, sizeof(c_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_args)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/Makefile",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/Makefile",
			.data = {
				"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
				"REPDIR := $(OUTDIR)coverage-report/\n"
				"DEPS := $(shell find src -name '*.h')\n"
				"SRC_C := $(shell find src -name '*.c')\n"
				"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
				"LCOV := $(OUTDIR)lcov.info\n"
				"COV := $(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))\n"
				"COV += $(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))\n"
				"COV += $(LCOV) $(REPDIR)\n"
				"TARGET := $(OUTDIR)test\n"
				"ARGS := -D\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS) -Wall -Wextra -Werror -pedantic\n"
				"DEFINES_STATIC :=\n"
				"LDFLAGS +=\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},

		},
	};

	const int ret = test_gen(mk_sln_gen, mk_sln_free, c_args_in, sizeof(c_args_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_include)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/Makefile",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/Makefile",
			.data = {
				"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
				"REPDIR := $(OUTDIR)coverage-report/\n"
				"DEPS := $(shell find include -name '*.h')\n"
				"DEPS += $(shell find src -name '*.h')\n"
				"SRC_C := $(shell find src -name '*.c')\n"
				"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
				"LCOV := $(OUTDIR)lcov.info\n"
				"COV := $(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))\n"
				"COV += $(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))\n"
				"COV += $(LCOV) $(REPDIR)\n"
				"TARGET := $(OUTDIR)test\n"
				"\n"
				"FLAGS := -Isrc -Iinclude\n"
				"CFLAGS += $(FLAGS) -Wall -Wextra -Werror -pedantic\n"
				"DEFINES_STATIC :=\n"
				"LDFLAGS +=\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},
		},
	};

	const int ret = test_gen(mk_sln_gen, mk_sln_free, c_include_in, sizeof(c_include_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_depends)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/Makefile",
			.data = SLN_LIBTEST,
		},
		{
			.path = "tmp/libtest/Makefile",
			.data = {
				"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/libtest/\n"
				"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/libtest/int/\n"
				"DEPS := $(shell find src -name '*.h')\n"
				"SRC_C := $(shell find src -name '*.c')\n"
				"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
				"OBJ_D_C := $(patsubst %.c, $(INTDIR)%.d.o, $(SRC_C))\n"
				"LCOV := $(OUTDIR)lcov.info\n"
				"COV := $(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))\n"
				"COV += $(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))\n"
				"COV += $(LCOV)\n"
				"TARGET_STATIC := $(OUTDIR)libtest.a\n"
				"TARGET_DYNAMIC := $(OUTDIR)libtest.so\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS) -Wall -Wextra -Werror -pedantic\n"
				"DEFINES_STATIC :=\n"
				"DEFINES_DYNAMIC := -DLIBTEST_BUILD_DLL\n"
				"LDFLAGS +=\n"
				"\n",
				PROJ_LIBTEST_COMPILE,
			},
		},
		{
			.path = "tmp/test/Makefile",
			.data = {
				"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
				"REPDIR := $(OUTDIR)coverage-report/\n"
				"DEPS := $(shell find src -name '*.h')\n"
				"SRC_C := $(shell find src -name '*.c')\n"
				"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
				"LCOV := $(OUTDIR)lcov.info\n"
				"COV := $(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))\n"
				"COV += $(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))\n"
				"COV += $(LCOV) $(REPDIR)\n"
				"TARGET := $(OUTDIR)test\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS) -Wall -Wextra -Werror -pedantic\n"
				"DEFINES_STATIC :=\n"
				"LDFLAGS += -L$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/libtest/ -l:libtest.a\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},
		},
	};

	const int ret = test_gen(mk_sln_gen, mk_sln_free, c_depends_in, sizeof(c_depends_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(cpp_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/Makefile",
			.data = SLN_TEST,
		},
		{
			.path = "tmp/test/Makefile",
			.data = {
				"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
				"REPDIR := $(OUTDIR)coverage-report/\n"
				"DEPS := $(shell find src -name '*.h')\n"
				"DEPS += $(shell find src -name '*.hpp')\n"
				"SRC_CPP := $(shell find src -name '*.cpp')\n"
				"OBJ_CPP := $(patsubst %.cpp, $(INTDIR)%.o, $(SRC_CPP))\n"
				"LCOV := $(OUTDIR)lcov.info\n"
				"COV := $(patsubst %.cpp, $(INTDIR)%.gcno, $(SRC_CPP))\n"
				"COV += $(patsubst %.cpp, $(INTDIR)%.gcda, $(SRC_CPP))\n"
				"COV += $(LCOV) $(REPDIR)\n"
				"TARGET := $(OUTDIR)test\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CXXFLAGS += $(FLAGS) -Wall -Wextra -Werror -pedantic\n"
				"DEFINES_STATIC :=\n"
				"LDFLAGS += -lstdc++\n"
				"\n",
				PROJ_TEST_CPP_COMPILE,
			},
		},
	};

	const int ret = test_gen(mk_sln_gen, mk_sln_free, cpp_small_in, sizeof(cpp_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(os)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/Makefile",
			.data = OS_SLN,
		},
		{
			.path = "tmp/os/boot/bin/Makefile",
			.data = OS_PROJ_BOOT_BIN,
		},
		{
			.path = "tmp/toolchain/gcc/Makefile",
			.data = OS_PROJ_GCC,
		},
		{
			.path = "tmp/os/kernel/bin/Makefile",
			.data = OS_PROJ_KERNEL_BIN,
		},
		{
			.path = "tmp/os/kernel/elf/Makefile",
			.data = OS_PROJ_KERNEL_ELF,
		},
		{
			.path = "tmp/os/image/disk/Makefile",
			.data = OS_PROJ_IMAGE_DISK,
		},
		{
			.path = "tmp/os/image/floppy/Makefile",
			.data = OS_PROJ_IMAGE_FLOPPY,
		},

	};

	const int ret = test_gen(mk_sln_gen, mk_sln_free, os_in, sizeof(os_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

STEST(t_mk)
{
	SSTART;
	/*RUN(pgc_gen_mk_init_free);
	RUN(pgc_gen_mk_empty);
	RUN(c_small);
	RUN(c_args);
	RUN(c_include);
	RUN(c_depends);
	RUN(cpp_small);
	RUN(os);*/
	SEND;
}
