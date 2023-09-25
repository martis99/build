#include "t_mk.h"

#include "gen/mk/mk_sln.h"

#include "common.h"
#include "test.h"

static const char *SLN_TEST = "SLNDIR := $(CURDIR)\n"
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
			      ".PHONY: all check test test/clean test/compile test/run test/coverage clean\n"
			      "\n"
			      "all: test\n"
			      "\n"
			      "check:\n"
			      "ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
			      "\t$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
			      "endif\n"
			      "\n"
			      "test: check\n"
			      "\t@$(MAKE) -C test\n"
			      "\n"
			      "test/clean: check\n"
			      "\t@$(MAKE) -C test clean\n"
			      "\n"
			      "test/compile: check\n"
			      "\t@$(MAKE) -C test compile\n"
			      "\n"
			      "test/run: check test/compile\n"
			      "\t@$(MAKE) -C test run\n"
			      "\n"
			      "test/coverage: check\n"
			      "\t@$(MAKE) -C test coverage\n"
			      "\n"
			      "clean: check\n"
			      "	@$(MAKE) -C test clean\n";

static const char *SLN_LIBTEST = "SLNDIR := $(CURDIR)\n"
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
				 ".PHONY: all check libtest libtest/clean libtest/compile libtest/coverage test test/clean test/compile test/run test/coverage clean\n"
				 "\n"
				 "all: libtest test\n"
				 "\n"
				 "check:\n"
				 "ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
				 "\t$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
				 "endif\n"
				 "\n"
				 "libtest: check\n"
				 "\t@$(MAKE) -C libtest\n"
				 "\n"
				 "libtest/clean: check\n"
				 "\t@$(MAKE) -C libtest clean\n"
				 "\n"
				 "libtest/compile: check\n"
				 "\t@$(MAKE) -C libtest compile\n"
				 "\n"
				 "libtest/coverage: check\n"
				 "\t@$(MAKE) -C libtest coverage\n"
				 "\n"
				 "test: check libtest\n"
				 "\t@$(MAKE) -C test\n"
				 "\n"
				 "test/clean: check libtest/clean\n"
				 "\t@$(MAKE) -C test clean\n"
				 "\n"
				 "test/compile: check libtest/compile\n"
				 "\t@$(MAKE) -C test compile\n"
				 "\n"
				 "test/run: check test/compile\n"
				 "\t@$(MAKE) -C test run\n"
				 "\n"
				 "test/coverage: check libtest/coverage\n"
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
					 ".PHONY: all check check_coverage test compile coverage run clean\n"
					 "\n"
					 "all: test\n"
					 "\n"
					 "check:\n"
					 "ifeq (, $(shell which gcc))\n"
					 "\tsudo apt install gcc\n"
					 "endif\n"
					 "\n"
					 "check_coverage: check\n"
					 "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
					 "ifeq (, $(shell which lcov))\n"
					 "\tsudo apt install lcov\n"
					 "endif\n"
					 "\n"
					 "test: clean compile\n"
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
					 "\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) -c -o $@ $<\n"
					 "\n"
					 "run: $(TARGET)\n"
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
					  ".PHONY: all check check_coverage libtest compile coverage clean\n"
					  "\n"
					  "all: libtest\n"
					  "\n"
					  "check:\n"
					  "ifeq (, $(shell which gcc))\n"
					  "\tsudo apt install gcc\n"
					  "endif\n"
					  "\n"
					  "check_coverage: check\n"
					  "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
					  "ifeq (, $(shell which lcov))\n"
					  "\tsudo apt install lcov\n"
					  "endif\n"
					  "\n"
					  "libtest: clean compile\n"
					  "\n"
					  "compile: check $(TARGET)\n"
					  "\n"
					  "coverage: clean check_coverage $(TARGET)\n"
					  "\n"
					  "$(TARGET): $(OBJ_C)\n"
					  "\t@mkdir -p $(@D)\n"
					  "\t@ar rcs $@ $^\n"
					  "\n"
					  "$(INTDIR)%.o: %.c\n"
					  "\t@mkdir -p $(@D)\n"
					  "\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) -c -o $@ $<\n"
					  "\n"
					  "clean:\n"
					  "\t@$(RM) $(TARGET) $(OBJ_C) $(COV)\n";

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
					   ".PHONY: all check check_coverage test compile coverage run clean\n"
					   "\n"
					   "all: test\n"
					   "\n"
					   "check:\n"
					   "ifeq (, $(shell which gcc))\n"
					   "\tsudo apt install gcc\n"
					   "endif\n"
					   "\n"
					   "check_coverage: check\n"
					   "\t$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
					   "ifeq (, $(shell which lcov))\n"
					   "\tsudo apt install lcov\n"
					   "endif\n"
					   "\n"
					   "test: clean compile\n"
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
					   "\t@$(TCC) $(CONFIG_FLAGS) $(CXXFLAGS) -c -o $@ $<\n"
					   "\n"
					   "run: $(TARGET)\n"
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
				"OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
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
				"ifeq ($(PLATFORM), x86_64)\n"
				"BITS := 64\n"
				"else\n"
				"BITS := 32\n"
				"endif\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS)\n"
				"LDFLAGS +=\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},

		},
	};

	const int ret = test_gen(mk_sln_gen, c_small_in, sizeof(c_small_in), out, sizeof(out));
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
				"OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
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
				"ifeq ($(PLATFORM), x86_64)\n"
				"BITS := 64\n"
				"else\n"
				"BITS := 32\n"
				"endif\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS)\n"
				"LDFLAGS +=\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},

		},
	};

	const int ret = test_gen(mk_sln_gen, c_small_in, sizeof(c_small_in), out, sizeof(out));
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
				"OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
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
				"ifeq ($(PLATFORM), x86_64)\n"
				"BITS := 64\n"
				"else\n"
				"BITS := 32\n"
				"endif\n"
				"\n"
				"FLAGS := -Isrc -Iinclude\n"
				"CFLAGS += $(FLAGS)\n"
				"LDFLAGS +=\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},
		},
	};

	const int ret = test_gen(mk_sln_gen, c_include_in, sizeof(c_include_in), out, sizeof(out));
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
				"OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/libtest/\n"
				"INTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/libtest/int/\n"
				"REPDIR := $(OUTDIR)coverage-report/\n"
				"DEPS := $(shell find src -name '*.h')\n"
				"SRC_C := $(shell find src -name '*.c')\n"
				"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
				"LCOV := $(OUTDIR)lcov.info\n"
				"COV := $(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))\n"
				"COV += $(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))\n"
				"COV += $(LCOV) $(REPDIR)\n"
				"TARGET := $(OUTDIR)libtest.a\n"
				"\n"
				"ifeq ($(PLATFORM), x86_64)\n"
				"BITS := 64\n"
				"else\n"
				"BITS := 32\n"
				"endif\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS)\n"
				"\n",
				PROJ_LIBTEST_COMPILE,
			},
		},
		{
			.path = "tmp/test/Makefile",
			.data = {
				"OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
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
				"ifeq ($(PLATFORM), x86_64)\n"
				"BITS := 64\n"
				"else\n"
				"BITS := 32\n"
				"endif\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CFLAGS += $(FLAGS)\n"
				"LDFLAGS += -L$(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/libtest/ -l:libtest.a\n"
				"\n",
				PROJ_TEST_C_COMPILE,
			},
		},
	};

	const int ret = test_gen(mk_sln_gen, c_depends_in, sizeof(c_depends_in), out, sizeof(out));
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
				"OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/\n"
				"INTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
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
				"ifeq ($(PLATFORM), x86_64)\n"
				"BITS := 64\n"
				"else\n"
				"BITS := 32\n"
				"endif\n"
				"\n"
				"FLAGS := -Isrc\n"
				"CXXFLAGS += $(FLAGS)\n"
				"LDFLAGS += -lstdc++\n"
				"\n",
				PROJ_TEST_CPP_COMPILE,
			},
		},
	};

	const int ret = test_gen(mk_sln_gen, cpp_small_in, sizeof(cpp_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

STEST(mk)
{
	SSTART;
	RUN(c_small);
	RUN(c_args);
	RUN(c_include);
	RUN(c_depends);
	RUN(cpp_small);
	SEND;
}
