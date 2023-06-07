#include "t_mk.h"

#include "gen/mk/mk_sln.h"

#include "common.h"
#include "test.h"

TEST(simple)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/Makefile",
			.data = "SLNDIR=$(CURDIR)\n"
				"\n"
				"CONFIGS = Debug\n"
				"CONFIG = Debug\n"
				"\n"
				"SHOW = true\n"
				"\n"
				".PHONY: all check clean\n"
				"\n"
				"all: test\n"
				"\n"
				"check:\n"
				"ifeq ($(filter $(CONFIG),$(CONFIGS)),)\n"
				"	$(error Config '$(CONFIG)' not found. Configs: $(CONFIGS))\n"
				"endif\n"
				"\n"
				".PHONY: test\n"
				"test: check\n"
				"	@$(MAKE) -C test SLNDIR=$(SLNDIR) CONFIG=$(CONFIG)\n"
				"\n"
				".PHONY: test/clean\n"
				"test/clean: check\n"
				"	@$(MAKE) -C test clean SLNDIR=$(SLNDIR) CONFIG=$(CONFIG)\n"
				"\n"
				".PHONY: test/compile\n"
				"test/compile: check\n"
				"	@$(MAKE) -C test compile SLNDIR=$(SLNDIR) CONFIG=$(CONFIG)\n"
				"\n"
				".PHONY: test/coverage\n"
				"test/coverage: check\n"
				"	@$(MAKE) -C test coverage SLNDIR=$(SLNDIR) CONFIG=$(CONFIG)\n"
				"\n"
				"clean: check\n"
				"	@$(MAKE) -C test clean SLNDIR=$(SLNDIR) CONFIG=$(CONFIG)\n",
		},
		{
			.path = "tmp/test/Makefile",
			.data = "OUTDIR = $(SLNDIR)/bin/$(CONFIG)-x64/test/\n"
				"INTDIR = $(SLNDIR)/bin/$(CONFIG)-x64/test/int/\n"
				"REPDIR = $(OUTDIR)coverage-report/\n"
				"DEPS = $(shell find src -name '*.h')\n"
				"SRC_C = $(shell find src -name '*.c')\n"
				"OBJ_C = $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
				"LCOV = $(OUTDIR)lcov.info\n"
				"COV = $(patsubst %.c, $(INTDIR)%.gcno, $(SRC_C))\n"
				"COV += $(patsubst %.c, $(INTDIR)%.gcda, $(SRC_C))\n"
				"COV += $(LCOV) $(REPDIR)\n"
				"TARGET = $(OUTDIR)test\n"
				"\n"
				"FLAGS = -Isrc\n"
				"CFLAGS += $(FLAGS)\n"
				"LDFLAGS +=\n"
				"\n"
				"RM += -r\n"
				"\n"
				"CONFIG_FLAGS =\n"
				"\n"
				"SHOW = true\n"
				"\n"
				".PHONY: all check check_coverage test compile coverage clean\n"
				"\n"
				"all: test\n"
				"\n"
				"check:\n"
				"ifeq ($(CONFIG), Debug)\n"
				"	$(eval CONFIG_FLAGS += -ggdb3 -O0)\n"
				"endif\n"
				"\n"
				"check_coverage: check\n"
				"	$(eval CONFIG_FLAGS += --coverage -fprofile-abs-path)\n"
				"ifeq (, $(shell which lcov))\n"
				"	$(error \"lcov not found\")\n"
				"endif\n"
				"\n"
				"test: clean compile\n"
				"\n"
				"compile: check $(TARGET)\n"
				"\n"
				"coverage: clean check_coverage $(TARGET)\n"
				"	@$(TARGET) $(ARGS)\n"
				"	@lcov -q -c -d $(SLNDIR) -o $(LCOV)\n"
				"ifeq ($(SHOW), true)\n"
				"	@genhtml -q $(LCOV) -o $(REPDIR)\n"
				"	@open $(REPDIR)index.html\n"
				"endif\n"
				"\n"
				"$(TARGET): $(OBJ_C)\n"
				"	@mkdir -p $(@D)\n"
				"	@$(CC) $(CONFIG_FLAGS) -o $@ $^ $(LDFLAGS)\n"
				"\n"
				"$(INTDIR)%.o: %.c\n"
				"	@mkdir -p $(@D)\n"
				"	@$(CC) $(CONFIG_FLAGS) $(CFLAGS) -c -o $@ $<\n"
				"\n"
				"clean:\n"
				"	@$(RM) $(TARGET) $(OBJ_C) $(COV)\n",

		},
	};

	EXPECT_EQ(test_gen(mk_sln_gen, simple_in, sizeof(simple_in), out, sizeof(out)), 0);

	END;
}

STEST(mk)
{
	SSTART;
	RUN(simple);
	SEND;
}
