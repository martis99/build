#include "t_mk_pgen.h"

#include "gen/mk/mk_pgen.h"

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

#define CONFIG              \
	"LDFLAGS :=\n"      \
	"CONFIG_FLAGS :=\n" \
	"\n"                \
	"RM += -r\n"        \
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

#define ASM_O                                                                      \
	"$(INTDIR)%.o: %.asm\n"                                                    \
	"\t@mkdir -p $(@D)\n"                                                      \
	"\t@nasm $(CONFIG_FLAGS) $(ASFLAGS) $(DEFINES) -felf$(BITS) -c -o $@ $<\n" \
	"\n"

#define ASM_SO                                                                       \
	"$(INTDIR_S)%.o: %.asm\n"                                                    \
	"\t@mkdir -p $(@D)\n"                                                        \
	"\t@nasm $(CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_S) -felf$(BITS) -c -o $@ $<\n" \
	"\n"

#define ASM_DO                                                                             \
	"$(INTDIR_D)%.o: %.asm\n"                                                          \
	"\t@mkdir -p $(@D)\n"                                                              \
	"\t@nasm $(CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_D) -fPIC -felf$(BITS) -c -o $@ $<\n" \
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

TEST(t_mk_pgen_init_free)
{
	START;

	mk_pgen_t gen = { 0 };
	EXPECT_EQ(mk_pgen_init(NULL), NULL);
	mem_oom(1);
	EXPECT_EQ(mk_pgen_init(&gen), NULL);
	mem_oom(0);
	EXPECT_EQ(mk_pgen_init(&gen), &gen);

	mk_pgen_free(NULL);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_config)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	EXPECT_EQ(mk_pgen_add_config(NULL, str_null()), MK_CONFIG_END);
	EXPECT_EQ(mk_pgen_add_config(&gen, str_null()), 0);
	EXPECT_EQ(mk_pgen_add_config(&gen, str_null()), 1);
	mem_oom(1);
	EXPECT_EQ(mk_pgen_add_config(&gen, str_null()), MK_CONFIG_END);
	mem_oom(0);
	EXPECT_EQ(mk_pgen_add_config(&gen, STRH("Debug")), 2);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_header)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	EXPECT_EQ(mk_pgen_add_header(NULL, str_null(), 0), MK_HEADER_END);
	EXPECT_EQ(mk_pgen_add_header(&gen, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(mk_pgen_add_header(&gen, str_null(), 0), MK_HEADER_END);
	mem_oom(0);
	EXPECT_EQ(mk_pgen_add_header(&gen, STRH("include/"), 0), 1);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_src)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	EXPECT_EQ(mk_pgen_add_src(NULL, str_null(), 0), MK_SRC_END);
	EXPECT_EQ(mk_pgen_add_src(&gen, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(mk_pgen_add_src(&gen, str_null(), 0), MK_SRC_END);
	mem_oom(0);
	EXPECT_EQ(mk_pgen_add_src(&gen, STRH("src/"), 0), 1);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_include)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_include(NULL, str_null());
	mk_pgen_add_include(&gen, STR("src/"));
	mk_pgen_add_include(&gen, STR("include/"));

	EXPECT_STRN(gen.includes.data, "-Isrc/ -Iinclude/", 17);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_flag)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_flag(NULL, str_null(), F_MK_EXT_C);
	mk_pgen_add_flag(&gen, STR("-Wall"), F_MK_EXT_C);
	mk_pgen_add_flag(&gen, STR("-Wextra"), F_MK_EXT_C);

	EXPECT_STRN(gen.flags[MK_EXT_C].data, "-Wall -Wextra", 13);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_define)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_define(NULL, str_null(), F_MK_INTDIR_OBJECT);
	mk_pgen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_OBJECT);
	mk_pgen_add_define(&gen, STR("UNICODE"), F_MK_INTDIR_OBJECT);

	EXPECT_STRN(gen.defines[MK_INTDIR_OBJECT].data, "-DDEBUG -DUNICODE", 13);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_ldflag)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_ldflag(NULL, str_null());
	mk_pgen_add_ldflag(&gen, STR("-lm"));
	mk_pgen_add_ldflag(&gen, STR("-lpthread"));

	EXPECT(str_eq(gen.ldflags, STR("-lm -lpthread")));

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_slib)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_slib(NULL, str_null());
	mk_pgen_add_slib(&gen, STR("a"));
	mk_pgen_add_slib(&gen, STR("b"));

	EXPECT_STRN(gen.ldflags.data, "-l:a.a -l:b.a", 11);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_dlib)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_dlib(NULL, str_null());
	mk_pgen_add_dlib(&gen, STR("a"));
	mk_pgen_add_dlib(&gen, STR("b"));

	EXPECT_STRN(gen.ldflags.data, "-l:a.so -l:b.so", 13);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_slib_dir)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_slib_dir(NULL, str_null());
	mk_pgen_add_slib_dir(&gen, STR("libs/a"));
	mk_pgen_add_slib_dir(&gen, STR("libs/b"));

	EXPECT_STRN(gen.ldflags.data, "-Llibs/a -Llibs/b", 17);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_dlib_dir)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_add_dlib_dir(NULL, str_null());
	mk_pgen_add_dlib_dir(&gen, STR("libs/a"));
	mk_pgen_add_dlib_dir(&gen, STR("libs/b"));

	EXPECT_STRN(gen.ldflags.data, "-Wl,-rpath,libs/a -Wl,-rpath,libs/b", 35);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_set_run)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_set_run(NULL, str_null(), F_MK_BUILD_EXE);
	mk_pgen_set_run(&gen, STRH("$(TARGET)"), F_MK_BUILD_EXE);

	EXPECT_STRN(gen.run[MK_BUILD_EXE].data, "$(TARGET)", 9);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_set_run_debug)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	mk_pgen_set_run_debug(NULL, str_null(), F_MK_BUILD_EXE);
	mk_pgen_set_run_debug(&gen, STRH("$(TARGET_DEBUG)"), F_MK_BUILD_EXE);

	EXPECT_STRN(gen.run_debug[MK_BUILD_EXE].data, "$(TARGET_DEBUG)", 15);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_add_file)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	EXPECT_EQ(mk_pgen_add_file(NULL, str_null(), MK_EXT_BIN), MK_FILE_END);
	EXPECT_EQ(mk_pgen_add_file(&gen, str_null(), MK_EXT_BIN), 0);
	EXPECT_EQ(mk_pgen_add_file(&gen, str_null(), MK_EXT_BIN), 1);
	mem_oom(1);
	EXPECT_EQ(mk_pgen_add_file(&gen, str_null(), MK_EXT_BIN), MK_FILE_END);
	mem_oom(0);
	EXPECT_EQ(mk_pgen_add_file(&gen, STRH("src/file.bin"), 0), 2);

	mk_pgen_free(&gen);

	END;
}
TEST(t_mk_pgen_add_require)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	EXPECT_EQ(mk_pgen_add_require(NULL, str_null()), MK_FILE_END);
	EXPECT_EQ(mk_pgen_add_require(&gen, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(mk_pgen_add_require(&gen, str_null()), MK_FILE_END);
	mem_oom(0);
	EXPECT_EQ(mk_pgen_add_require(&gen, STRH("g++")), 1);

	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_empty)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	char buf[128] = { 0 };
	EXPECT_EQ(mk_pgen(NULL, NULL), NULL);
	EXPECT_EQ(mk_pgen(&gen, &make), &make);
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_args)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;
	gen.args   = STRH("-D");

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_headers)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	mk_pgen_add_header(&gen, STRH("src/"), F_MK_EXT_H);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_includes)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);
	mk_pgen_add_include(&gen, STR("src/"));
	mk_pgen_add_include(&gen, STR("include/"));

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_flags)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);
	mk_pgen_add_flag(&gen, STR("-Wall"), F_MK_EXT_C);
	mk_pgen_add_flag(&gen, STR("-Wextra"), F_MK_EXT_C);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_ldflags)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);
	mk_pgen_add_ldflag(&gen, STR("-lm"));
	mk_pgen_add_ldflag(&gen, STR("-lpthread"));

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"LDFLAGS := -lm -lpthread\n"
			"CONFIG_FLAGS :=\n"
			"\n"
			"RM += -r\n"
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_coverage_exe)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.covdir = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/");

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_coverage_static)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");
	gen.covdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_coverage_shared)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");
	gen.covdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/cov/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_defines_exe)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_OBJECT);
	mk_pgen_add_define(&gen, STR("VERSION=1"), F_MK_INTDIR_OBJECT);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_defines_static)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_STATIC;

	mk_pgen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_STATIC);
	mk_pgen_add_define(&gen, STR("VERSION=1"), F_MK_INTDIR_STATIC);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_defines_shared)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_SHARED;

	mk_pgen_add_define(&gen, STR("DEBUG"), F_MK_INTDIR_SHARED);
	mk_pgen_add_define(&gen, STR("VERSION=1"), F_MK_INTDIR_SHARED);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_run)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen_set_run(&gen, STRH("run"), F_MK_BUILD_EXE);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_run_debug)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen_add_config(&gen, STRH("Debug"));

	mk_pgen_set_run_debug(&gen, STRH("run_debug"), F_MK_BUILD_EXE);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
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
			"run: check $(TARGET)\n"
			"ifeq ($(CONFIG), Debug)\n"
			"\trun_debug\n"
			"else\n"
			"\t@$(TARGET) $(ARGS)\n"
			"endif\n"
			"\n"
			"" CLEAN_EXE_C "");

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_run_run_debug)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen_add_config(&gen, STRH("Debug"));

	mk_pgen_set_run(&gen, STRH("run"), F_MK_BUILD_EXE);
	mk_pgen_set_run_debug(&gen, STRH("run_debug"), F_MK_BUILD_EXE);

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
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
			"run: check $(TARGET)\n"
			"ifeq ($(CONFIG), Debug)\n"
			"\trun_debug\n"
			"else\n"
			"\trun\n"
			"endif\n"
			"\n"
			"" CLEAN_EXE_C "");

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_artifact)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	gen.artifact[MK_BUILD_EXE] = STRH("test");

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_bin_obj)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_BIN;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"TARGET_BIN := $(OUTDIR)test.bin\n"
			"SRC_C := $(shell find src/ -name '*.c')\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"\n"
			"" CONFIG ""
			".PHONY: all check bin clean\n"
			"\n"
			"all: bin\n"
			"\n"
			"" C_CHECK ""
			"bin: check $(TARGET_BIN)\n"
			"\n"
			"$(TARGET_BIN): $(OBJ_C)\n"
			"\t@mkdir -p $(@D)\n"
			"\t@$(TLD) -Tlinker.ld --oformat binary -o $@ $^ $(LDFLAGS)\n"
			"\n"
			"" C_O ""
			"clean:\n"
			"\t@$(RM) $(TARGET_BIN) $(OBJ_C)\n"
			"\n");

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_bin_files)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_BIN;

	mk_pgen_add_file(&gen, STRH("file.bin"), MK_EXT_BIN);
	mk_pgen_add_file(&gen, STRH("file.elf"), MK_EXT_ELF);

	gen.size = STRH("1024");

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"TARGET_BIN := $(OUTDIR)test.bin\n"
			"\n"
			"RM += -r\n"
			"\n"
			".PHONY: all check bin clean\n"
			"\n"
			"all: bin\n"
			"\n"
			"check:\n"
			"\n"
			"bin: check $(TARGET_BIN)\n"
			"\n"
			"$(TARGET_BIN):\n"
			"\t@mkdir -p $(@D)\n"
			"\t@dd if=file.bin status=none >> $@\n"
			"\t@objcopy -O binary -j .text file.elf file.elf.bin\n"
			"\t@dd if=file.elf.bin status=none >> $@\n"
			"\t@dd if=/dev/zero bs=1 count=1024 status=none >> $@\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET_BIN)\n"
			"\n");

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_elf)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_ELF;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_fat12)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_FAT12;

	mk_pgen_add_file(&gen, STRH("file.bin"), MK_EXT_BIN);
	mk_pgen_add_file(&gen, STRH("file.elf"), MK_EXT_ELF);

	gen.size = STRH("1024");

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/\n"
			"TARGET_FAT12 := $(OUTDIR)test.img\n"
			"\n"
			"RM += -r\n"
			"\n"
			".PHONY: all check fat12 clean\n"
			"\n"
			"all: fat12\n"
			"\n"
			"check:\n"
			"ifeq (, $(shell which mcopy))\n"
			"\tsudo apt install mtools -y\n"
			"endif\n"
			"\n"
			"fat12: check $(TARGET_FAT12)\n"
			"\n"
			"$(TARGET_FAT12):\n"
			"\t@mkdir -p $(@D)\n"
			"\t@dd if=/dev/zero of=$@ bs=512 count=2880 status=none\n"
			"\t@mkfs.fat -F12 -n \"NBOS\" $@\n"
			"\t@dd if=$< of=$@ conv=notrunc status=none\n"
			"\t@mcopy -i $@ $(word 2,$^) \"::$(shell basename $(word 2,$^))\"\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET_FAT12)\n"
			"\n");

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_configs)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_config(&gen, STRH("Debug"));
	mk_pgen_add_config(&gen, STRH("Release"));

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_asm_exe)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_ASM);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" ASM_EXE_VARS ""
			"\n"
			"ifeq ($(PLATFORM), x86_64)\n"
			"BITS := 64\n"
			"else\n"
			"BITS := 32\n"
			"endif\n"
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_asm_static)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_ASM);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_STATIC;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" ASM_STATIC_VARS ""
			"ifeq ($(PLATFORM), x86_64)\n"
			"BITS := 64\n"
			"else\n"
			"BITS := 32\n"
			"endif\n"
			"\n"
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_asm_shared)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_ASM);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_SHARED;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" ASM_SHARED_VARS ""
			"ifeq ($(PLATFORM), x86_64)\n"
			"BITS := 64\n"
			"else\n"
			"BITS := 32\n"
			"endif\n"
			"\n"
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_c_exe)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_c_static)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_STATIC;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_c_shared)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_C);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_SHARED;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_cpp_exe)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_CPP);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_EXE;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_cpp_static)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_CPP);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_STATIC;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_cpp_shared)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.outdir		     = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/");
	gen.intdir[MK_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(PLATFORM)/test/int/");

	mk_pgen_add_src(&gen, STRH("src/"), F_MK_EXT_CPP);

	gen.name   = STRH("test");
	gen.builds = F_MK_BUILD_SHARED;

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

TEST(t_mk_pgen_url)
{
	START;

	mk_pgen_t gen = { 0 };
	mk_pgen_init(&gen);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	gen.url	    = STRH("http://ftp.gnu.org/gnu/gcc/gcc-13.1.0/");
	gen.name    = STRH("gcc-13.1.0");
	gen.format  = STRH("tar.gz");
	gen.config  = STRH("--disable-nls");
	gen.targets = STRH("all-gcc");
	gen.outdir  = STRH("$(SLNDIR)bin/$(PLATFORM)/test/");
	mk_pgen_add_require(&gen, STRH("g++"));

	mk_pgen(&gen, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(PLATFORM)/test/\n"
			"URL := http://ftp.gnu.org/gnu/gcc/gcc-13.1.0/\n"
			"NAME := gcc-13.1.0\n"
			"FORMAT := tar.gz\n"
			"FILE := $(NAME).$(FORMAT)\n"
			"DLDIR := $(SLNDIR)dl/$(FILE)/\n"
			"SRCDIR := $(SLNDIR)staging/$(NAME)/\n"
			"BUILDDIR := $(SLNDIR)build/$(PLATFORM)/$(NAME)/\n"
			"LOGDIR := $(SLNDIR)logs/$(PLATFORM)/$(NAME)/\n"
			"\n"
			".PHONY: all check compile clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"check:\n"
			"ifeq (, $(shell which curl))\n"
			"\tsudo apt install curl -y\n"
			"endif\n"
			"ifeq (, $(shell dpkg -l g++))\n"
			"\tsudo apt install g++ -y\n"
			"endif\n"
			"\n"
			"$(DLDIR):\n"
			"\t@mkdir -p $(@D)\n"
			"\t@cd $(@D) && curl -O $(URL)$(FILE)\n"
			"\n"
			"$(SRCDIR)done: $(DLDIR)\n"
			"\t@mkdir -p $(SLNDIR)staging\n"
			"\t@tar xf $(DLDIR) -C $(SLNDIR)staging\n"
			"\t@touch $(SRCDIR)done\n"
			"\n"
			"$(OUTDIR)$(NAME): $(SRCDIR)done\n"
			"\t@mkdir -p $(LOGDIR) $(BUILDDIR) $(OUTDIR)\n"
			"\t@cd $(BUILDDIR) && $(SRCDIR)configure --target=$(PLATFORM)-elf --prefix=$(OUTDIR) --disable-nls 2>&1 | tee $(LOGDIR)configure.log\n"
			"\t@cd $(BUILDDIR) && make all-gcc 2>&1 | tee $(LOGDIR)make.log\n"
			"\t@touch $(OUTDIR)$(NAME)\n"
			"\n"
			"compile: check $(OUTDIR)$(NAME)\n"
			"\n"
			"clean:\n"
			"\n");

	make_free(&make);
	mk_pgen_free(&gen);

	END;
}

STEST(t_mk_pgen)
{
	SSTART;
	RUN(t_mk_pgen_init_free);
	RUN(t_mk_pgen_add_config);
	RUN(t_mk_pgen_add_header);
	RUN(t_mk_pgen_add_src);
	RUN(t_mk_pgen_add_include);
	RUN(t_mk_pgen_add_flag);
	RUN(t_mk_pgen_add_define);
	RUN(t_mk_pgen_add_ldflag);
	RUN(t_mk_pgen_add_slib);
	RUN(t_mk_pgen_add_dlib);
	RUN(t_mk_pgen_add_slib_dir);
	RUN(t_mk_pgen_add_dlib_dir);
	RUN(t_mk_pgen_set_run);
	RUN(t_mk_pgen_set_run_debug);
	RUN(t_mk_pgen_add_file);
	RUN(t_mk_pgen_add_require);
	RUN(t_mk_pgen_empty);
	RUN(t_mk_pgen_args);
	RUN(t_mk_pgen_headers);
	RUN(t_mk_pgen_includes);
	RUN(t_mk_pgen_flags);
	RUN(t_mk_pgen_ldflags);
	RUN(t_mk_pgen_coverage_exe);
	RUN(t_mk_pgen_coverage_static);
	RUN(t_mk_pgen_coverage_shared);
	RUN(t_mk_pgen_defines_exe);
	RUN(t_mk_pgen_defines_static);
	RUN(t_mk_pgen_defines_shared);
	RUN(t_mk_pgen_run);
	RUN(t_mk_pgen_run_debug);
	RUN(t_mk_pgen_run_run_debug);
	RUN(t_mk_pgen_artifact);
	RUN(t_mk_pgen_bin_obj);
	RUN(t_mk_pgen_bin_files);
	RUN(t_mk_pgen_elf);
	RUN(t_mk_pgen_fat12);
	RUN(t_mk_pgen_configs);
	RUN(t_mk_pgen_asm_exe);
	RUN(t_mk_pgen_asm_static);
	RUN(t_mk_pgen_asm_shared);
	RUN(t_mk_pgen_c_exe);
	RUN(t_mk_pgen_c_static);
	RUN(t_mk_pgen_c_shared);
	RUN(t_mk_pgen_cpp_exe);
	RUN(t_mk_pgen_cpp_static);
	RUN(t_mk_pgen_cpp_shared);
	RUN(t_mk_pgen_url);
	SEND;
}
