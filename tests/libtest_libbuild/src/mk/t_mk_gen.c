#include "t_mk_gen.h"

#include "gen/mk/mk_gen.h"

#include "mem.h"
#include "test.h"

#define EXE_VARS                                                   \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"     \
	"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n" \
	"TARGET := $(OUTDIR)test\n"                                \
	"ARGS :=\n"

#define STATIC_VARS                                                  \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"       \
	"INTDIR_S := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n" \
	"TARGET_S := $(OUTDIR)test.a\n"

#define SHARED_VARS                                                  \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"       \
	"INTDIR_D := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n" \
	"TARGET_D := $(OUTDIR)test.so\n"

#define ASM_EXE_VARS                                    \
	EXE_VARS                                        \
	"SRC_ASM := $(shell find src/ -name '*.asm')\n" \
	"OBJ_ASM := $(patsubst %.asm, $(INTDIR)%.o, $(SRC_ASM))\n"

#define ASM_STATIC_VARS                                                \
	STATIC_VARS                                                    \
	"SRC_ASM := $(shell find src/ -name '*.asm')\n"                \
	"OBJ_ASM_S := $(patsubst %.asm, $(INTDIR_S)%.o, $(SRC_ASM))\n" \
	"\n"

#define ASM_SHARED_VARS                                                \
	SHARED_VARS                                                    \
	"SRC_ASM := $(shell find src/ -name '*.asm')\n"                \
	"OBJ_ASM_D := $(patsubst %.asm, $(INTDIR_D)%.o, $(SRC_ASM))\n" \
	"\n"

#define C_EXE_VARS                                  \
	EXE_VARS                                    \
	"SRC_C := $(shell find src/ -name '*.c')\n" \
	"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"

#define C_STATIC_VARS                                            \
	STATIC_VARS                                              \
	"SRC_C := $(shell find src/ -name '*.c')\n"              \
	"OBJ_C_S := $(patsubst %.c, $(INTDIR_S)%.o, $(SRC_C))\n" \
	"\n"

#define C_SHARED_VARS                                            \
	SHARED_VARS                                              \
	"SRC_C := $(shell find src/ -name '*.c')\n"              \
	"OBJ_C_D := $(patsubst %.c, $(INTDIR_D)%.o, $(SRC_C))\n" \
	"\n"

#define CPP_EXE_VARS                                    \
	EXE_VARS                                        \
	"SRC_CPP := $(shell find src/ -name '*.cpp')\n" \
	"OBJ_CPP := $(patsubst %.cpp, $(INTDIR)%.o, $(SRC_CPP))\n"

#define CPP_STATIC_VARS                                                \
	STATIC_VARS                                                    \
	"SRC_CPP := $(shell find src/ -name '*.cpp')\n"                \
	"OBJ_CPP_S := $(patsubst %.cpp, $(INTDIR_S)%.o, $(SRC_CPP))\n" \
	"\n"

#define CPP_SHARED_VARS                                                \
	SHARED_VARS                                                    \
	"SRC_CPP := $(shell find src/ -name '*.cpp')\n"                \
	"OBJ_CPP_D := $(patsubst %.cpp, $(INTDIR_D)%.o, $(SRC_CPP))\n" \
	"\n"

#define CONFIG                         \
	"LDFLAGS :=\n"                 \
	"CONFIG_FLAGS :=\n"            \
	"\n"                           \
	"RM += -r\n"                   \
	"\n"                           \
	"ifeq ($(CONFIG), Debug)\n"    \
	"CONFIG_FLAGS += -ggdb3 -O0\n" \
	"endif\n"                      \
	"\n"

#define ASM_CHECK                        \
	"check:\n"                       \
	"ifeq (, $(shell which nasm))\n" \
	"\tsudo apt install nasm -y\n"   \
	"endif\n"                        \
	"\n"

#define C_CHECK                         \
	"check:\n"                      \
	"ifeq (, $(shell which gcc))\n" \
	"\tsudo apt install gcc -y\n"   \
	"endif\n"                       \
	"\n"

#define CPP_CHECK                       \
	"check:\n"                      \
	"ifeq (, $(shell which gcc))\n" \
	"\tsudo apt install gcc -y\n"   \
	"endif\n"                       \
	"\n"

#define TARGET_ASM                        \
	"$(TARGET): $(OBJ_ASM)\n"         \
	"\t@mkdir -p $(@D)\n"             \
	"\t@$(TCC) -o $@ $^ $(LDFLAGS)\n" \
	"\n"

#define TARGET_ASM_S                  \
	"$(TARGET_S): $(OBJ_ASM_S)\n" \
	"\t@mkdir -p $(@D)\n"         \
	"\t@ar rcs $@ $^\n"           \
	"\n"

#define TARGET_ASM_D                              \
	"$(TARGET_D): $(OBJ_ASM_D)\n"             \
	"\t@mkdir -p $(@D)\n"                     \
	"\t@$(TCC) -shared -o $@ $^ $(LDFLAGS)\n" \
	"\n"

#define TARGET_C                          \
	"$(TARGET): $(OBJ_C)\n"           \
	"\t@mkdir -p $(@D)\n"             \
	"\t@$(TCC) -o $@ $^ $(LDFLAGS)\n" \
	"\n"

#define TARGET_C_S                  \
	"$(TARGET_S): $(OBJ_C_S)\n" \
	"\t@mkdir -p $(@D)\n"       \
	"\t@ar rcs $@ $^\n"         \
	"\n"

#define TARGET_C_D                                \
	"$(TARGET_D): $(OBJ_C_D)\n"               \
	"\t@mkdir -p $(@D)\n"                     \
	"\t@$(TCC) -shared -o $@ $^ $(LDFLAGS)\n" \
	"\n"

#define TARGET_CPP                        \
	"$(TARGET): $(OBJ_CPP)\n"         \
	"\t@mkdir -p $(@D)\n"             \
	"\t@$(TCC) -o $@ $^ $(LDFLAGS)\n" \
	"\n"

#define TARGET_CPP_S                  \
	"$(TARGET_S): $(OBJ_CPP_S)\n" \
	"\t@mkdir -p $(@D)\n"         \
	"\t@ar rcs $@ $^\n"           \
	"\n"

#define TARGET_CPP_D                              \
	"$(TARGET_D): $(OBJ_CPP_D)\n"             \
	"\t@mkdir -p $(@D)\n"                     \
	"\t@$(TCC) -shared -o $@ $^ $(LDFLAGS)\n" \
	"\n"

#define ASM_O                                                         \
	"$(INTDIR)%.o: %.asm\n"                                       \
	"\t@mkdir -p $(@D)\n"                                         \
	"\t@nasm $(CONFIG_FLAGS) $(ASFLAGS) $(DEFINES) -c -o $@ $<\n" \
	"\n"

#define ASM_SO                                                          \
	"$(INTDIR_S)%.o: %.asm\n"                                       \
	"\t@mkdir -p $(@D)\n"                                           \
	"\t@nasm $(CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_S) -c -o $@ $<\n" \
	"\n"

#define ASM_DO                                                                \
	"$(INTDIR_D)%.o: %.asm\n"                                             \
	"\t@mkdir -p $(@D)\n"                                                 \
	"\t@nasm $(CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_D) -fPIC -c -o $@ $<\n" \
	"\n"

#define C_O                                                            \
	"$(INTDIR)%.o: %.c\n"                                          \
	"\t@mkdir -p $(@D)\n"                                          \
	"\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) $(DEFINES) -c -o $@ $<\n" \
	"\n"

#define C_SO                                                             \
	"$(INTDIR_S)%.o: %.c\n"                                          \
	"\t@mkdir -p $(@D)\n"                                            \
	"\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) $(DEFINES_S) -c -o $@ $<\n" \
	"\n"

#define C_DO                                                                   \
	"$(INTDIR_D)%.o: %.c\n"                                                \
	"\t@mkdir -p $(@D)\n"                                                  \
	"\t@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) $(DEFINES_D) -fPIC -c -o $@ $<\n" \
	"\n"

#define CPP_O                                                            \
	"$(INTDIR)%.o: %.cpp\n"                                          \
	"\t@mkdir -p $(@D)\n"                                            \
	"\t@$(TCC) $(CONFIG_FLAGS) $(CXXFLAGS) $(DEFINES) -c -o $@ $<\n" \
	"\n"

#define CPP_SO                                                             \
	"$(INTDIR_S)%.o: %.cpp\n"                                          \
	"\t@mkdir -p $(@D)\n"                                              \
	"\t@$(TCC) $(CONFIG_FLAGS) $(CXXFLAGS) $(DEFINES_S) -c -o $@ $<\n" \
	"\n"

#define CPP_DO                                                                   \
	"$(INTDIR_D)%.o: %.cpp\n"                                                \
	"\t@mkdir -p $(@D)\n"                                                    \
	"\t@$(TCC) $(CONFIG_FLAGS) $(CXXFLAGS) $(DEFINES_D) -fPIC -c -o $@ $<\n" \
	"\n"

#define RUN_EXE                  \
	"run: check $(TARGET)\n" \
	"\t@$(TARGET) $(ARGS)\n" \
	"\n"

#define CLEAN_EXE_ASM                     \
	"clean:\n"                        \
	"\t@$(RM) $(TARGET) $(OBJ_ASM)\n" \
	"\n"

#define CLEAN_EXE_ASM_S                       \
	"clean:\n"                            \
	"\t@$(RM) $(TARGET_S) $(OBJ_ASM_S)\n" \
	"\n"

#define CLEAN_EXE_ASM_D                       \
	"clean:\n"                            \
	"\t@$(RM) $(TARGET_D) $(OBJ_ASM_D)\n" \
	"\n"

#define CLEAN_EXE_C                     \
	"clean:\n"                      \
	"\t@$(RM) $(TARGET) $(OBJ_C)\n" \
	"\n"

#define CLEAN_EXE_C_S                       \
	"clean:\n"                          \
	"\t@$(RM) $(TARGET_S) $(OBJ_C_S)\n" \
	"\n"

#define CLEAN_EXE_C_D                       \
	"clean:\n"                          \
	"\t@$(RM) $(TARGET_D) $(OBJ_C_D)\n" \
	"\n"

#define CLEAN_EXE_CPP                     \
	"clean:\n"                        \
	"\t@$(RM) $(TARGET) $(OBJ_CPP)\n" \
	"\n"

#define CLEAN_EXE_CPP_S                       \
	"clean:\n"                            \
	"\t@$(RM) $(TARGET_S) $(OBJ_CPP_S)\n" \
	"\n"

#define CLEAN_EXE_CPP_D                       \
	"clean:\n"                            \
	"\t@$(RM) $(TARGET_D) $(OBJ_CPP_D)\n" \
	"\n"

TEST(t_mk_gen_init_free)
{
	START;

	mk_gen_t gen = { 0 };
	EXPECT_EQ(mk_gen_init(NULL), NULL);
	EXPECT_EQ(mk_gen_init(&gen), &gen);

	mk_gen_free(NULL);
	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_add_header)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	EXPECT_EQ(mk_gen_add_header(NULL, str_null(), 0), MK_HEADER_END);
	EXPECT_EQ(mk_gen_add_header(&gen, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(mk_gen_add_header(&gen, str_null(), 0), MK_HEADER_END);
	mem_oom(0);
	EXPECT_EQ(mk_gen_add_header(&gen, STRH("include/"), 0), 1);

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_add_src)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	EXPECT_EQ(mk_gen_add_src(NULL, str_null(), 0), MK_SRC_END);
	EXPECT_EQ(mk_gen_add_src(&gen, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(mk_gen_add_src(&gen, str_null(), 0), MK_SRC_END);
	mem_oom(0);
	EXPECT_EQ(mk_gen_add_src(&gen, STRH("src/"), 0), 1);

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_add_include)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	mk_gen_add_include(NULL, str_null());
	mk_gen_add_include(&gen, STR("src/"));

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_add_flag)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	mk_gen_add_flag(NULL, str_null(), F_MK_EXT_C);
	mk_gen_add_flag(&gen, STR("-Wall"), F_MK_EXT_C);
	mk_gen_add_flag(&gen, STR("-Wextra"), F_MK_EXT_C);

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_add_define)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	mk_gen_add_define(NULL, str_null(), F_MK_INTDIR_OBJECT);
	mk_gen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_OBJECT);

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_add_ldflag)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	mk_gen_add_ldflag(NULL, str_null());
	mk_gen_add_ldflag(&gen, STR("-lm"));
	mk_gen_add_ldflag(&gen, STR("-lpthread"));

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_set_run)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	mk_gen_set_run(NULL, str_null(), F_MK_TARGET_EXE);
	mk_gen_set_run(&gen, STRH("$(TARGET)"), F_MK_TARGET_EXE);

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_set_run_debug)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	mk_gen_set_run_debug(NULL, str_null(), F_MK_TARGET_EXE);
	mk_gen_set_run_debug(&gen, STRH("$(TARGET)"), F_MK_TARGET_EXE);

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_empty)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	char buf[128] = { 0 };
	EXPECT_EQ(mk_gen_print(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0)), 65);
	EXPECT_STR(buf, "RM += -r\n"
			"\n"
			".PHONY: all check clean\n"
			"\n"
			"all:\n"
			"\n"
			"check:\n"
			"\n"
			"clean:\n"
			"\t@$(RM)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_args)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_EXE] = STRH("test");
	gen.args		  = STRH("-D");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"TARGET := $(OUTDIR)test\n"
			"ARGS := -D\n"
			"SRC_C := $(shell find src/ -name '*.c')\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_C "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_headers)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	mk_gen_add_header(&gen, STRH("src/"), F_MK_EXT_H);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "HEADERS := $(shell find src/ -name '*.h')\n"
			"\n"
			"RM += -r\n"
			"\n"
			".PHONY: all check clean\n"
			"\n"
			"all:\n"
			"\n"
			"check:\n"
			"\n"
			"clean:\n"
			"\t@$(RM)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_includes)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);
	mk_gen_add_include(&gen, STR("src/"));
	mk_gen_add_include(&gen, STR("include/"));

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"SRC_C := $(shell find src/ -name '*.c')\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"\n"
			"INCLUDES := -Isrc/ -Iinclude/\n"
			"" CONFIG ""
			".PHONY: all check clean\n"
			"\n"
			"all:\n"
			"\n"
			"" C_CHECK ""
			"" C_O ""
			"clean:\n"
			"\t@$(RM) $(OBJ_C)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_flags)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);
	mk_gen_add_flag(&gen, STR("-Wall"), F_MK_EXT_C);
	mk_gen_add_flag(&gen, STR("-Wextra"), F_MK_EXT_C);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"SRC_C := $(shell find src/ -name '*.c')\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"\n"
			"CFLAGS := -Wall -Wextra\n"
			"" CONFIG ""
			".PHONY: all check clean\n"
			"\n"
			"all:\n"
			"\n"
			"" C_CHECK ""
			"" C_O ""
			"clean:\n"
			"\t@$(RM) $(OBJ_C)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_ldflags)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	gen.target[MK_TARGET_EXE] = STRH("test");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);
	mk_gen_add_ldflag(&gen, STR("-lm"));
	mk_gen_add_ldflag(&gen, STR("-lpthread"));

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"LDFLAGS := -lm -lpthread\n"
			"CONFIG_FLAGS :=\n"
			"\n"
			"RM += -r\n"
			"\n"
			"ifeq ($(CONFIG), Debug)\n"
			"CONFIG_FLAGS += -ggdb3 -O0\n"
			"endif\n"
			"\n"
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_C "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_coverage_exe)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.covdir = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/");

	gen.target[MK_TARGET_EXE] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"COVDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/\n"
			"TARGET := $(OUTDIR)test\n"
			"ARGS :=\n"
			"REPDIR := $(COVDIR)coverage-report/\n"
			"LCOV := $(COVDIR)lcov.info\n"
			"COV := $(LCOV) $(REPDIR)\n"
			"\n"
			"RM += -r\n"
			"\n"
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"check:\n"
			"ifeq ($(COVERAGE), true)\n"
			"ifeq (, $(shell which lcov))\n"
			"\tsudo apt install lcov -y\n"
			"endif\n"
			"endif\n"
			"\n"
			"compile: check $(TARGET)\n"
			"\n"
			"run: check $(TARGET)\n"
			"\t@$(TARGET) $(ARGS)\n"
			"ifeq ($(COVERAGE), true)\n"
			"\t@lcov -q -c -d $(SLNDIR) -o $(LCOV)\n"
			"ifeq ($(SHOW), true)\n"
			"\t@genhtml -q $(LCOV) -o $(REPDIR)\n"
			"\t@open $(REPDIR)index.html\n"
			"endif\n"
			"endif\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET) $(COV)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_coverage_static)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");
	gen.covdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "INTDIR_S := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"COVDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/\n"
			"SRC_C := $(shell find src/ -name '*.c')\n"
			"OBJ_C_S := $(patsubst %.c, $(INTDIR_S)%.o, $(SRC_C))\n"
			"COV := $(patsubst %.c, $(COVDIR)%.gcno, $(SRC_C))\n"
			"COV += $(patsubst %.c, $(COVDIR)%.gcda, $(SRC_C))\n"
			"\n"
			"" CONFIG ""
			"ifeq ($(COVERAGE), true)\n"
			"CONFIG_FLAGS += --coverage -fprofile-abs-path\n"
			"endif\n"
			"\n"
			".PHONY: all check clean\n"
			"\n"
			"all:\n"
			"\n"
			"" C_CHECK ""
			"" C_SO ""
			"clean:\n"
			"\t@$(RM) $(OBJ_C_S) $(COV)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_coverage_shared)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");
	gen.covdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "INTDIR_D := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"COVDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/\n"
			"SRC_C := $(shell find src/ -name '*.c')\n"
			"OBJ_C_D := $(patsubst %.c, $(INTDIR_D)%.o, $(SRC_C))\n"
			"COV := $(patsubst %.c, $(COVDIR)%.gcno, $(SRC_C))\n"
			"COV += $(patsubst %.c, $(COVDIR)%.gcda, $(SRC_C))\n"
			"\n"
			"" CONFIG ""
			"ifeq ($(COVERAGE), true)\n"
			"CONFIG_FLAGS += --coverage -fprofile-abs-path\n"
			"endif\n"
			"\n"
			".PHONY: all check clean\n"
			"\n"
			"all:\n"
			"\n"
			"" C_CHECK ""
			"" C_DO ""
			"clean:\n"
			"\t@$(RM) $(OBJ_C_D) $(COV)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_defines_exe)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_EXE] = STRH("test");

	mk_gen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_OBJECT);
	mk_gen_add_define(&gen, STR("VERSION=1"), F_MK_INTDIR_OBJECT);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"DEFINES := -DDEBUG -DVERSION=1\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_C "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_defines_static)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_STATIC] = STRH("test");

	mk_gen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_STATIC);
	mk_gen_add_define(&gen, STR("VERSION=1"), F_MK_INTDIR_STATIC);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_STATIC_VARS ""
			"DEFINES_S := -DDEBUG -DVERSION=1\n"
			"" CONFIG ""
			".PHONY: all check static clean\n"
			"\n"
			"all: static\n"
			"\n"
			"" C_CHECK ""
			"static: check $(TARGET_S)\n"
			"\n"
			"" TARGET_C_S ""
			"" C_SO ""
			"" CLEAN_EXE_C_S "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_defines_shared)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_SHARED] = STRH("test");

	mk_gen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_SHARED);
	mk_gen_add_define(&gen, STR("VERSION=1"), F_MK_INTDIR_SHARED);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_SHARED_VARS ""
			"DEFINES_D := -DDEBUG -DVERSION=1\n"
			"" CONFIG ""
			".PHONY: all check shared clean\n"
			"\n"
			"all: shared\n"
			"\n"
			"" C_CHECK ""
			"shared: check $(TARGET_D)\n"
			"\n"
			"" TARGET_C_D ""
			"" C_DO ""
			"" CLEAN_EXE_C_D "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_run)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_EXE] = STRH("test");

	mk_gen_set_run(&gen, STRH("run"), F_MK_TARGET_EXE);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"run: check $(TARGET)\n"
			"\trun\n"
			"\n"
			"" CLEAN_EXE_C "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_run_debug)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_EXE] = STRH("test");

	mk_gen_set_run_debug(&gen, STRH("run_debug"), F_MK_TARGET_EXE);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"run: check $(TARGET)\n"
			"ifeq ($(CONFIG), Debug)\n"
			"\trun_debug\n"
			"else\n"
			"\t@$(TARGET) $(ARGS)\n"
			"endif\n"
			"\n"
			"" CLEAN_EXE_C "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_run_run_debug)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_EXE] = STRH("test");

	mk_gen_set_run(&gen, STRH("run"), F_MK_TARGET_EXE);
	mk_gen_set_run_debug(&gen, STRH("run_debug"), F_MK_TARGET_EXE);

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"run: check $(TARGET)\n"
			"ifeq ($(CONFIG), Debug)\n"
			"\trun_debug\n"
			"else\n"
			"\trun\n"
			"endif\n"
			"\n"
			"" CLEAN_EXE_C "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_artifact)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_EXE] = STRH("test");

	gen.artifact[MK_TARGET_EXE] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run artifact clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"" RUN_EXE ""
			"artifact: check $(TARGET)\n"
			"\t@mkdir -p $(SLNDIR)tmp/artifact/\n"
			"\t@cp $(TARGET) $(SLNDIR)tmp/artifact/test\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET) $(SLNDIR)tmp/artifact/test $(OBJ_C)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_elf)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_ELF] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"TARGET_ELF := $(OUTDIR)test.elf\n"
			"SRC_C := $(shell find src/ -name '*.c')\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"\n"
			"" CONFIG ""
			".PHONY: all check elf clean\n"
			"\n"
			"all: elf\n"
			"\n"
			"" C_CHECK ""
			"elf: check $(TARGET_ELF)\n"
			"\n"
			"$(TARGET_ELF): $(OBJ_C)\n"
			"\t@mkdir -p $(@D)\n"
			"\t@$(TLD) -Tlinker.ld -o $@ $^ $(LDFLAGS)\n"
			"\n"
			"" C_O ""
			"clean:\n"
			"\t@$(RM) $(TARGET_ELF) $(OBJ_C)\n"
			"\n");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_asm_exe)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_ASM);

	gen.target[MK_TARGET_EXE] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" ASM_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" ASM_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_ASM ""
			"" ASM_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_ASM "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_asm_static)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_ASM);

	gen.target[MK_TARGET_STATIC] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" ASM_STATIC_VARS ""
			"" CONFIG ""
			".PHONY: all check static clean\n"
			"\n"
			"all: static\n"
			"\n"
			"" ASM_CHECK ""
			"static: check $(TARGET_S)\n"
			"\n"
			"" TARGET_ASM_S ""
			"" ASM_SO ""
			"" CLEAN_EXE_ASM_S "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_asm_shared)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_ASM);

	gen.target[MK_TARGET_SHARED] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" ASM_SHARED_VARS ""
			"" CONFIG ""
			".PHONY: all check shared clean\n"
			"\n"
			"all: shared\n"
			"\n"
			"" ASM_CHECK ""
			"shared: check $(TARGET_D)\n"
			"\n"
			"" TARGET_ASM_D ""
			"" ASM_DO ""
			"" CLEAN_EXE_ASM_D "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_c_exe)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_EXE] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_C "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_c_static)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_STATIC] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_STATIC_VARS ""
			"" CONFIG ""
			".PHONY: all check static clean\n"
			"\n"
			"all: static\n"
			"\n"
			"" C_CHECK ""
			"static: check $(TARGET_S)\n"
			"\n"
			"" TARGET_C_S ""
			"" C_SO ""
			"" CLEAN_EXE_C_S "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_c_shared)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.target[MK_TARGET_SHARED] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" C_SHARED_VARS ""
			"" CONFIG ""
			".PHONY: all check shared clean\n"
			"\n"
			"all: shared\n"
			"\n"
			"" C_CHECK ""
			"shared: check $(TARGET_D)\n"
			"\n"
			"" TARGET_C_D ""
			"" C_DO ""
			"" CLEAN_EXE_C_D "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_cpp_exe)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_CPP);

	gen.target[MK_TARGET_EXE] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" CPP_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" CPP_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_CPP ""
			"" CPP_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_CPP "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_cpp_static)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_CPP);

	gen.target[MK_TARGET_STATIC] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" CPP_STATIC_VARS ""
			"" CONFIG ""
			".PHONY: all check static clean\n"
			"\n"
			"all: static\n"
			"\n"
			"" CPP_CHECK ""
			"static: check $(TARGET_S)\n"
			"\n"
			"" TARGET_CPP_S ""
			"" CPP_SO ""
			"" CLEAN_EXE_CPP_S "");

	mk_gen_free(&gen);

	END;
}

TEST(t_mk_gen_cpp_shared)
{
	START;

	mk_gen_t gen = { 0 };
	mk_gen_init(&gen);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_gen_add_src(&gen, STRH("src/"), F_MK_EXT_CPP);

	gen.target[MK_TARGET_SHARED] = STRH("test");

	char buf[1024] = { 0 };
	mk_gen_print(&gen, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "" CPP_SHARED_VARS ""
			"" CONFIG ""
			".PHONY: all check shared clean\n"
			"\n"
			"all: shared\n"
			"\n"
			"" CPP_CHECK ""
			"shared: check $(TARGET_D)\n"
			"\n"
			"" TARGET_CPP_D ""
			"" CPP_DO ""
			"" CLEAN_EXE_CPP_D "");

	mk_gen_free(&gen);

	END;
}

STEST(t_mk_gen)
{
	SSTART;
	RUN(t_mk_gen_init_free);
	RUN(t_mk_gen_add_header);
	RUN(t_mk_gen_add_src);
	RUN(t_mk_gen_add_include);
	RUN(t_mk_gen_add_flag);
	RUN(t_mk_gen_add_define);
	RUN(t_mk_gen_add_ldflag);
	RUN(t_mk_gen_set_run);
	RUN(t_mk_gen_set_run_debug);
	RUN(t_mk_gen_empty);
	RUN(t_mk_gen_args);
	RUN(t_mk_gen_headers);
	RUN(t_mk_gen_includes);
	RUN(t_mk_gen_flags);
	RUN(t_mk_gen_ldflags);
	RUN(t_mk_gen_coverage_exe);
	RUN(t_mk_gen_coverage_static);
	RUN(t_mk_gen_coverage_shared);
	RUN(t_mk_gen_defines_exe);
	RUN(t_mk_gen_defines_static);
	RUN(t_mk_gen_defines_shared);
	RUN(t_mk_gen_run);
	RUN(t_mk_gen_run_debug);
	RUN(t_mk_gen_run_run_debug);
	RUN(t_mk_gen_artifact);
	RUN(t_mk_gen_elf);
	RUN(t_mk_gen_asm_exe);
	RUN(t_mk_gen_asm_static);
	RUN(t_mk_gen_asm_shared);
	RUN(t_mk_gen_c_exe);
	RUN(t_mk_gen_c_static);
	RUN(t_mk_gen_c_shared);
	RUN(t_mk_gen_cpp_exe);
	RUN(t_mk_gen_cpp_static);
	RUN(t_mk_gen_cpp_shared);
	SEND;
}
