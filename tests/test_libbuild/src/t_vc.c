#include "t_vc.h"

#include "gen/vc/vc_sln.h"

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

static const char *PROJ_C = "OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/\n"
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
			    "\n"
			    "RM += -r\n"
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

static const char *PROJ_CPP = "OUTDIR := $(SLNDIR)/bin/$(CONFIG)-$(PLATFORM)/test/\n"
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
			      "\n"
			      "RM += -r\n"
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

static const char *TASKS = "{\n"
			   "        \"version\": \"2.0.0\",\n"
			   "        \"tasks\": [\n"
			   "                {\n"
			   "                        \"label\": \"Build-test-Debug-x64\",\n"
			   "                        \"type\": \"shell\",\n"
			   "                        \"command\": \"make\",\n"
			   "                        \"args\": [\n"
			   "                                \"clean\",\n"
			   "                                \"test/compile\",\n"
			   "                                \"CONFIG=Debug\",\n"
			   "                                \"PLATFORM=x64\"\n"
			   "                        ],\n"
			   "                        \"group\": {\n"
			   "                                \"kind\": \"build\",\n"
			   "                                \"isDefault\": true\n"
			   "                        }\n"
			   "                },\n"
			   "                {\n"
			   "                        \"label\": \"Run-test-Debug-x64\",\n"
			   "                        \"type\": \"shell\",\n"
			   "                        \"command\": \"make\",\n"
			   "                        \"args\": [\n"
			   "                                \"clean\",\n"
			   "                                \"test/run\",\n"
			   "                                \"CONFIG=Debug\",\n"
			   "                                \"PLATFORM=x64\"\n"
			   "                        ],\n"
			   "                        \"group\": {\n"
			   "                                \"kind\": \"build\",\n"
			   "                                \"isDefault\": true\n"
			   "                        }\n"
			   "                }\n"
			   "        ]\n"
			   "}";

static const char *LAUNCH = "{\n"
			    "        \"version\": \"0.2.0\",\n"
			    "        \"configurations\": [\n"
			    "                {\n"
			    "                        \"name\": \"test-Debug-x64\",\n"
			    "                        \"type\": \"cppdbg\",\n"
			    "                        \"request\": \"launch\",\n"
			    "                        \"program\": \"${workspaceFolder}/bin/Debug-x64/test/test\",\n"
			    "                        \"args\": [],\n"
			    "                        \"preLaunchTask\": \"Build-test-Debug-x64\",\n"
			    "                        \"stopAtEntry\": false,\n"
			    "                        \"cwd\": \"${workspaceFolder}/test\",\n"
			    "                        \"environment\": [],\n"
			    "                        \"externalConsole\": false,\n"
			    "                        \"MIMode\": \"gdb\",\n"
			    "                        \"setupCommands\": [\n"
			    "                                {\n"
			    "                                        \"description\": \"Enable pretty-printing for gdb\",\n"
			    "                                        \"text\": \"-enable-pretty-printing\",\n"
			    "                                        \"ignoreFailures\": true\n"
			    "                                },\n"
			    "                                {\n"
			    "                                        \"description\": \"Set Disassembly Flavor to Intel\",\n"
			    "                                        \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			    "                                        \"ignoreFailures\": true\n"
			    "                                }\n"
			    "                        ]\n"
			    "                }\n"
			    "        ]\n"
			    "}";

static const char *LAUNCH_ARGS = "{\n"
				 "        \"version\": \"0.2.0\",\n"
				 "        \"configurations\": [\n"
				 "                {\n"
				 "                        \"name\": \"test-Debug-x64\",\n"
				 "                        \"type\": \"cppdbg\",\n"
				 "                        \"request\": \"launch\",\n"
				 "                        \"program\": \"${workspaceFolder}/bin/Debug-x64/test/test\",\n"
				 "                        \"args\": [\n"
				 "                                \"-D\"\n"
				 "                        ],\n"
				 "                        \"preLaunchTask\": \"Build-test-Debug-x64\",\n"
				 "                        \"stopAtEntry\": false,\n"
				 "                        \"cwd\": \"${workspaceFolder}/test\",\n"
				 "                        \"environment\": [],\n"
				 "                        \"externalConsole\": false,\n"
				 "                        \"MIMode\": \"gdb\",\n"
				 "                        \"setupCommands\": [\n"
				 "                                {\n"
				 "                                        \"description\": \"Enable pretty-printing for gdb\",\n"
				 "                                        \"text\": \"-enable-pretty-printing\",\n"
				 "                                        \"ignoreFailures\": true\n"
				 "                                },\n"
				 "                                {\n"
				 "                                        \"description\": \"Set Disassembly Flavor to Intel\",\n"
				 "                                        \"text\": \"-gdb-set disassembly-flavor intel\",\n"
				 "                                        \"ignoreFailures\": true\n"
				 "                                }\n"
				 "                        ]\n"
				 "                }\n"
				 "        ]\n"
				 "}";
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
			.data = PROJ_C,
		},
		{
			.path = "tmp/.vscode/tasks.json",
			.data = TASKS,
		},
		{
			.path = "tmp/.vscode/launch.json",
			.data = LAUNCH,
		},
	};

	const int ret = test_gen(vc_sln_gen, c_small_in, sizeof(c_small_in), out, sizeof(out));
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
			.data = PROJ_C,
		},
		{
			.path = "tmp/.vscode/tasks.json",
			.data = TASKS,
		},
		{
			.path = "tmp/.vscode/launch.json",
			.data = LAUNCH_ARGS,
		},
	};

	const int ret = test_gen(vc_sln_gen, c_args_in, sizeof(c_args_in), out, sizeof(out));
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
			.data = PROJ_CPP,
		},
		{
			.path = "tmp/.vscode/tasks.json",
			.data = TASKS,
		},
		{
			.path = "tmp/.vscode/launch.json",
			.data = LAUNCH,
		},
	};

	const int ret = test_gen(vc_sln_gen, cpp_small_in, sizeof(cpp_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

STEST(vc)
{
	SSTART;
	RUN(c_small);
	RUN(c_args);
	RUN(cpp_small);
	SEND;
}
//350