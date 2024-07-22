#include "gen/mk/pgc_gen_mk.h"

#include "mem.h"
#include "pgc_gen.h"
#include "test.h"

#define ASM_BIN_VARS                                                  \
	"SRC_ASM := $(shell find src/ -name '*.nasm')\n"              \
	"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"        \
	"OBJ_ASM := $(patsubst %.nasm, $(INTDIR)%.bin, $(SRC_ASM))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"            \
	"TARGET_BIN := $(OUTDIR)test.bin\n"

#define ASM_EXE_VARS                                                \
	"SRC_ASM := $(shell find src/ -name '*.nasm')\n"            \
	"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"      \
	"OBJ_ASM := $(patsubst %.nasm, $(INTDIR)%.o, $(SRC_ASM))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"          \
	"TARGET := $(OUTDIR)test\n"                                 \
	"ARGS :=\n"

#define ASM_STATIC_VARS                                                 \
	"SRC_ASM := $(shell find src/ -name '*.nasm')\n"                \
	"INTDIR_S := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"        \
	"OBJ_ASM_S := $(patsubst %.nasm, $(INTDIR_S)%.o, $(SRC_ASM))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"              \
	"TARGET_S := $(OUTDIR)test.a\n"                                 \
	"\n"

#define ASM_SHARED_VARS                                                 \
	"SRC_ASM := $(shell find src/ -name '*.nasm')\n"                \
	"INTDIR_D := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"        \
	"OBJ_ASM_D := $(patsubst %.nasm, $(INTDIR_D)%.o, $(SRC_ASM))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"              \
	"TARGET_D := $(OUTDIR)test.so\n"                                \
	"\n"

#define S_EXE_VARS                                             \
	"SRC_S := $(shell find src/ -name '*.S')\n"            \
	"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n" \
	"OBJ_S := $(patsubst %.S, $(INTDIR)%.o, $(SRC_S))\n"   \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"     \
	"TARGET := $(OUTDIR)test\n"                            \
	"ARGS :=\n"

#define S_STATIC_VARS                                            \
	"SRC_S := $(shell find src/ -name '*.S')\n"              \
	"INTDIR_S := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n" \
	"OBJ_S_S := $(patsubst %.S, $(INTDIR_S)%.o, $(SRC_S))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"TARGET_S := $(OUTDIR)test.a\n"                          \
	"\n"

#define S_SHARED_VARS                                            \
	"SRC_S := $(shell find src/ -name '*.S')\n"              \
	"INTDIR_D := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n" \
	"OBJ_S_D := $(patsubst %.S, $(INTDIR_D)%.o, $(SRC_S))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"TARGET_D := $(OUTDIR)test.so\n"                         \
	"\n"

#define C_EXE_VARS                                             \
	"SRC_C := $(shell find src/ -name '*.c')\n"            \
	"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n" \
	"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"   \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"     \
	"TARGET := $(OUTDIR)test\n"                            \
	"ARGS :=\n"

#define C_STATIC_VARS                                            \
	"SRC_C := $(shell find src/ -name '*.c')\n"              \
	"INTDIR_S := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n" \
	"OBJ_C_S := $(patsubst %.c, $(INTDIR_S)%.o, $(SRC_C))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"TARGET_S := $(OUTDIR)test.a\n"                          \
	"\n"

#define C_SHARED_VARS                                            \
	"SRC_C := $(shell find src/ -name '*.c')\n"              \
	"INTDIR_D := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n" \
	"OBJ_C_D := $(patsubst %.c, $(INTDIR_D)%.o, $(SRC_C))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"TARGET_D := $(OUTDIR)test.so\n"                         \
	"\n"

#define CPP_EXE_VARS                                               \
	"SRC_CPP := $(shell find src/ -name '*.cpp')\n"            \
	"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"     \
	"OBJ_CPP := $(patsubst %.cpp, $(INTDIR)%.o, $(SRC_CPP))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"         \
	"TARGET := $(OUTDIR)test\n"                                \
	"ARGS :=\n"

#define CPP_STATIC_VARS                                                \
	"SRC_CPP := $(shell find src/ -name '*.cpp')\n"                \
	"INTDIR_S := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"       \
	"OBJ_CPP_S := $(patsubst %.cpp, $(INTDIR_S)%.o, $(SRC_CPP))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"             \
	"TARGET_S := $(OUTDIR)test.a\n"                                \
	"\n"

#define CPP_SHARED_VARS                                                \
	"SRC_CPP := $(shell find src/ -name '*.cpp')\n"                \
	"INTDIR_D := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"       \
	"OBJ_CPP_D := $(patsubst %.cpp, $(INTDIR_D)%.o, $(SRC_CPP))\n" \
	"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"             \
	"TARGET_D := $(OUTDIR)test.so\n"                               \
	"\n"

#define CONFIG_BITS                \
	"ifeq ($(ARCH), x86_64)\n" \
	"BITS := 64\n"             \
	"endif\n"                  \
	"ifeq ($(ARCH), i386)\n"   \
	"BITS := 32\n"             \
	"endif\n"

#define CONFIG_NASM              \
	"LDFLAGS :=\n"           \
	"NASM_CONFIG_FLAGS :=\n" \
	"\n"                     \
	"RM += -r\n"             \
	"\n"                     \
	"" CONFIG_BITS ""        \
	"\n"

#define CONFIG                  \
	"LDFLAGS :=\n"          \
	"GCC_CONFIG_FLAGS :=\n" \
	"\n"                    \
	"RM += -r\n"            \
	"\n"                    \
	"" CONFIG_BITS ""       \
	"\n"

#define CONFIG_DEBUG                       \
	"ifeq ($(CONFIG), Debug)\n"        \
	"GCC_CONFIG_FLAGS += -ggdb3 -O0\n" \
	"endif\n"                          \
	"\n"

#define ASM_CHECK                          \
	"check:\n"                         \
	"ifeq (, $(shell which nasm))\n"   \
	"\tsudo apt-get install nasm -y\n" \
	"endif\n"                          \
	"\n"

#define S_CHECK                                                                     \
	"check:\n"                                                                  \
	"ifeq (, $(shell which gcc))\n"                                             \
	"\tsudo apt-get install gcc -y\n"                                           \
	"endif\n"                                                                   \
	"ifeq ($(ARCH), $(shell uname -m))\n"                                       \
	"else\n"                                                                    \
	"ifeq (, $(shell apt list --installed 2>/dev/null | grep gcc-multilib/))\n" \
	"\tsudo apt-get install gcc-multilib -y\n"                                  \
	"endif\n"                                                                   \
	"endif\n"                                                                   \
	"\n"

#define C_CHECK                                                                     \
	"check:\n"                                                                  \
	"ifeq (, $(shell which gcc))\n"                                             \
	"\tsudo apt-get install gcc -y\n"                                           \
	"endif\n"                                                                   \
	"ifeq ($(ARCH), $(shell uname -m))\n"                                       \
	"else\n"                                                                    \
	"ifeq (, $(shell apt list --installed 2>/dev/null | grep gcc-multilib/))\n" \
	"\tsudo apt-get install gcc-multilib -y\n"                                  \
	"endif\n"                                                                   \
	"endif\n"                                                                   \
	"\n"

#define CPP_CHECK                                                                   \
	"check:\n"                                                                  \
	"ifeq (, $(shell which g++))\n"                                             \
	"\tsudo apt-get install g++ -y\n"                                           \
	"endif\n"                                                                   \
	"ifeq ($(ARCH), $(shell uname -m))\n"                                       \
	"else\n"                                                                    \
	"ifeq (, $(shell apt list --installed 2>/dev/null | grep g++-multilib/))\n" \
	"\tsudo apt-get install g++-multilib -y\n"                                  \
	"endif\n"                                                                   \
	"endif\n"                                                                   \
	"\n"

#define TARGET_ASM_BIN                \
	"$(TARGET_BIN): $(OBJ_ASM)\n" \
	"\t@mkdir -p $(@D)\n"         \
	"\t@cat $(OBJ_ASM) > $@\n"    \
	"\n"

#define TARGET_ASM_EXE                                      \
	"$(TARGET): $(OBJ_ASM)\n"                           \
	"\t@mkdir -p $(@D)\n"                               \
	"\t@$(TCC) -m$(BITS) $(OBJ_ASM) $(LDFLAGS) -o $@\n" \
	"\n"

#define TARGET_ASM_S                  \
	"$(TARGET_S): $(OBJ_ASM_S)\n" \
	"\t@mkdir -p $(@D)\n"         \
	"\t@ar rcs $@ $(OBJ_ASM_S)\n" \
	"\n"

#define TARGET_ASM_D                                                  \
	"$(TARGET_D): $(OBJ_ASM_D)\n"                                 \
	"\t@mkdir -p $(@D)\n"                                         \
	"\t@$(TCC) -m$(BITS) -shared $(OBJ_ASM_D) $(LDFLAGS) -o $@\n" \
	"\n"

#define TARGET_S                                          \
	"$(TARGET): $(OBJ_S)\n"                           \
	"\t@mkdir -p $(@D)\n"                             \
	"\t@$(TCC) -m$(BITS) $(OBJ_S) $(LDFLAGS) -o $@\n" \
	"\n"

#define TARGET_S_S                  \
	"$(TARGET_S): $(OBJ_S_S)\n" \
	"\t@mkdir -p $(@D)\n"       \
	"\t@ar rcs $@ $(OBJ_S_S)\n" \
	"\n"

#define TARGET_S_D                                                  \
	"$(TARGET_D): $(OBJ_S_D)\n"                                 \
	"\t@mkdir -p $(@D)\n"                                       \
	"\t@$(TCC) -m$(BITS) -shared $(OBJ_S_D) $(LDFLAGS) -o $@\n" \
	"\n"

#define TARGET_C                                          \
	"$(TARGET): $(OBJ_C)\n"                           \
	"\t@mkdir -p $(@D)\n"                             \
	"\t@$(TCC) -m$(BITS) $(OBJ_C) $(LDFLAGS) -o $@\n" \
	"\n"

#define TARGET_C_S                  \
	"$(TARGET_S): $(OBJ_C_S)\n" \
	"\t@mkdir -p $(@D)\n"       \
	"\t@ar rcs $@ $(OBJ_C_S)\n" \
	"\n"

#define TARGET_C_D                                                  \
	"$(TARGET_D): $(OBJ_C_D)\n"                                 \
	"\t@mkdir -p $(@D)\n"                                       \
	"\t@$(TCC) -m$(BITS) -shared $(OBJ_C_D) $(LDFLAGS) -o $@\n" \
	"\n"

#define TARGET_CPP                                          \
	"$(TARGET): $(OBJ_CPP)\n"                           \
	"\t@mkdir -p $(@D)\n"                               \
	"\t@$(TCC) -m$(BITS) $(OBJ_CPP) $(LDFLAGS) -o $@\n" \
	"\n"

#define TARGET_CPP_S                  \
	"$(TARGET_S): $(OBJ_CPP_S)\n" \
	"\t@mkdir -p $(@D)\n"         \
	"\t@ar rcs $@ $(OBJ_CPP_S)\n" \
	"\n"

#define TARGET_CPP_D                                                  \
	"$(TARGET_D): $(OBJ_CPP_D)\n"                                 \
	"\t@mkdir -p $(@D)\n"                                         \
	"\t@$(TCC) -m$(BITS) -shared $(OBJ_CPP_D) $(LDFLAGS) -o $@\n" \
	"\n"

#define ASM_BIN                                                                           \
	"$(INTDIR)%.bin: %.nasm\n"                                                        \
	"\t@mkdir -p $(@D)\n"                                                             \
	"\t@nasm -fbin $(INCLUDES) $(NASM_CONFIG_FLAGS) $(ASFLAGS) $(DEFINES) $< -o $@\n" \
	"\n"

#define ASM_O                                                                                    \
	"$(INTDIR)%.o: %.nasm\n"                                                                 \
	"\t@mkdir -p $(@D)\n"                                                                    \
	"\t@nasm -felf$(BITS) $(INCLUDES) $(NASM_CONFIG_FLAGS) $(ASFLAGS) $(DEFINES) $< -o $@\n" \
	"\n"

#define ASM_SO                                                                                     \
	"$(INTDIR_S)%.o: %.nasm\n"                                                                 \
	"\t@mkdir -p $(@D)\n"                                                                      \
	"\t@nasm -felf$(BITS) $(INCLUDES) $(NASM_CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_S) $< -o $@\n" \
	"\n"

#define ASM_DO                                                                                     \
	"$(INTDIR_D)%.o: %.nasm\n"                                                                 \
	"\t@mkdir -p $(@D)\n"                                                                      \
	"\t@nasm -felf$(BITS) $(INCLUDES) $(NASM_CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_D) $< -o $@\n" \
	"\n"

#define S_O                                                                                       \
	"$(INTDIR)%.o: %.S\n"                                                                     \
	"\t@mkdir -p $(@D)\n"                                                                     \
	"\t@$(TCC) -m$(BITS) -c $(INCLUDES) $(GCC_CONFIG_FLAGS) $(ASFLAGS) $(DEFINES) $< -o $@\n" \
	"\n"

#define S_SO                                                                                        \
	"$(INTDIR_S)%.o: %.S\n"                                                                     \
	"\t@mkdir -p $(@D)\n"                                                                       \
	"\t@$(TCC) -m$(BITS) -c $(INCLUDES) $(GCC_CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_S) $< -o $@\n" \
	"\n"

#define S_DO                                                                                              \
	"$(INTDIR_D)%.o: %.S\n"                                                                           \
	"\t@mkdir -p $(@D)\n"                                                                             \
	"\t@$(TCC) -m$(BITS) -c -fPIC $(INCLUDES) $(GCC_CONFIG_FLAGS) $(ASFLAGS) $(DEFINES_D) $< -o $@\n" \
	"\n"

#define C_O                                                                                      \
	"$(INTDIR)%.o: %.c\n"                                                                    \
	"\t@mkdir -p $(@D)\n"                                                                    \
	"\t@$(TCC) -m$(BITS) -c $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CFLAGS) $(DEFINES) $< -o $@\n" \
	"\n"

#define C_SO                                                                                       \
	"$(INTDIR_S)%.o: %.c\n"                                                                    \
	"\t@mkdir -p $(@D)\n"                                                                      \
	"\t@$(TCC) -m$(BITS) -c $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CFLAGS) $(DEFINES_S) $< -o $@\n" \
	"\n"

#define C_DO                                                                                             \
	"$(INTDIR_D)%.o: %.c\n"                                                                          \
	"\t@mkdir -p $(@D)\n"                                                                            \
	"\t@$(TCC) -m$(BITS) -c -fPIC $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CFLAGS) $(DEFINES_D) $< -o $@\n" \
	"\n"

#define CPP_O                                                                                      \
	"$(INTDIR)%.o: %.cpp\n"                                                                    \
	"\t@mkdir -p $(@D)\n"                                                                      \
	"\t@$(TCC) -m$(BITS) -c $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CXXFLAGS) $(DEFINES) $< -o $@\n" \
	"\n"

#define CPP_SO                                                                                       \
	"$(INTDIR_S)%.o: %.cpp\n"                                                                    \
	"\t@mkdir -p $(@D)\n"                                                                        \
	"\t@$(TCC) -m$(BITS) -c $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CXXFLAGS) $(DEFINES_S) $< -o $@\n" \
	"\n"

#define CPP_DO                                                                                             \
	"$(INTDIR_D)%.o: %.cpp\n"                                                                          \
	"\t@mkdir -p $(@D)\n"                                                                              \
	"\t@$(TCC) -m$(BITS) -c -fPIC $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CXXFLAGS) $(DEFINES_D) $< -o $@\n" \
	"\n"

#define RUN_EXE                             \
	"run: check $(TARGET)\n"            \
	"\t@$(TARGET) $(ARGS)\n"            \
	"\n"                                \
	"debug: check $(TARGET)\n"          \
	"\t@gdb --args $(TARGET) $(ARGS)\n" \
	"\n"

#define CLEAN_BIN_ASM                         \
	"clean:\n"                            \
	"\t@$(RM) $(TARGET_BIN) $(OBJ_ASM)\n" \
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

#define CLEAN_EXE_S                     \
	"clean:\n"                      \
	"\t@$(RM) $(TARGET) $(OBJ_S)\n" \
	"\n"

#define CLEAN_EXE_S_S                       \
	"clean:\n"                          \
	"\t@$(RM) $(TARGET_S) $(OBJ_S_S)\n" \
	"\n"

#define CLEAN_EXE_S_D                       \
	"clean:\n"                          \
	"\t@$(RM) $(TARGET_D) $(OBJ_S_D)\n" \
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

TEST(t_pgc_gen_mk_empty)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_empty(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	char buf[128] = { 0 };
	EXPECT_EQ(pgc_gen_mk(NULL, NULL), NULL);
	EXPECT_EQ(pgc_gen_mk(&pgc, &make), &make);
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_args)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_args(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"TARGET := $(OUTDIR)test\n"
			"ARGS := -D\n"
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run debug clean\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_require)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_require(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RM += -r\n"
			"\n"
			".PHONY: all check clean\n"
			"\n"
			"all:\n"
			"\n"
			"check:\n"
			"ifeq (, $(shell apt list --installed 2>/dev/null | grep package/))\n"
			"\tsudo apt-get install package -y\n"
			"endif\n"
			"\n"
			"clean:\n"
			"\t@$(RM)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_lib_empty)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_lib_empty(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_headers)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_headers(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_includes)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_includes(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_flags)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_flags(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_ldflags)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_ldflags(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"LDFLAGS := -lm -lpthread\n"
			"GCC_CONFIG_FLAGS :=\n"
			"\n"
			"RM += -r\n"
			"\n"
			"" CONFIG_BITS ""
			"\n"
			".PHONY: all check compile run debug clean\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_libs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_libs(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"LDFLAGS := -Llibs/ -Llibs/ -l:lib.a -Llibs/ -l:lib.a -Llibs/ -Wl,-rpath,. -l:lib.so -Llibs/ -l:lib.so\n"
			"GCC_CONFIG_FLAGS :=\n"
			"\n"
			"RM += -r\n"
			"\n"
			"" CONFIG_BITS ""
			"\n"
			".PHONY: all check compile run debug clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"$(TARGET): $(OBJ_C) libs/lib.a libs/lib.a libs/lib.so libs/lib.so\n"
			"\t@mkdir -p $(@D)\n"
			"\t@$(TCC) -m$(BITS) $(OBJ_C) $(LDFLAGS) -o $@\n"
			"\n"
			"" C_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_C "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_depends)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_depends(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"LDFLAGS :=\n"
			"GCC_CONFIG_FLAGS :=\n"
			"\n"
			"RM += -r\n"
			"\n"
			"" CONFIG_BITS ""
			"\n"
			".PHONY: all check compile run debug clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" C_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"$(TARGET): $(OBJ_C)\n"
			"\t@mkdir -p $(@D)\n"
			"\t@$(TCC) -m$(BITS) $(OBJ_C) $(LDFLAGS) -o $@\n"
			"\n"
			"" C_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_C "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_coverage_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_coverage_exe(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"COVDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/cov/\n"
			"COV := $(patsubst %.c, $(COVDIR)%.gcno, $(SRC_C))\n"
			"COV += $(patsubst %.c, $(COVDIR)%.gcda, $(SRC_C))\n"
			"TARGET := $(OUTDIR)test\n"
			"ARGS :=\n"
			"REPDIR := $(COVDIR)coverage-report/\n"
			"LCOV := $(COVDIR)lcov.info\n"
			"COV += $(LCOV) $(REPDIR)\n"
			"\n"
			"" CONFIG ""
			"ifeq ($(COVERAGE), true)\n"
			"GCC_CONFIG_FLAGS += --coverage -fprofile-abs-path\n"
			"endif\n"
			"\n"
			".PHONY: all check compile run debug clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"check:\n"
			"ifeq (, $(shell which gcc))\n"
			"\tsudo apt-get install gcc -y\n"
			"endif\n"
			"ifeq ($(ARCH), $(shell uname -m))\n"
			"else\n"
			"ifeq (, $(shell apt list --installed 2>/dev/null | grep gcc-multilib/))\n"
			"\tsudo apt-get install gcc-multilib -y\n"
			"endif\n"
			"endif\n"
			"ifeq ($(COVERAGE), true)\n"
			"ifeq (, $(shell which lcov))\n"
			"\tsudo apt-get install lcov -y\n"
			"endif\n"
			"endif\n"
			"\n"
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_C ""
			"" C_O ""
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
			"debug: check $(TARGET)\n"
			"\t@gdb --args $(TARGET) $(ARGS)\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET) $(OBJ_C) $(COV)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_coverage_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_coverage_static(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR_S := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C_S := $(patsubst %.c, $(INTDIR_S)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"COVDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/cov/\n"
			"COV := $(patsubst %.c, $(COVDIR)%.gcno, $(SRC_C))\n"
			"COV += $(patsubst %.c, $(COVDIR)%.gcda, $(SRC_C))\n"
			"TARGET_S := $(OUTDIR)test.a\n"
			"\n"
			"" CONFIG ""
			"ifeq ($(COVERAGE), true)\n"
			"GCC_CONFIG_FLAGS += --coverage -fprofile-abs-path\n"
			"endif\n"
			"\n"
			".PHONY: all check static clean\n"
			"\n"
			"all: static\n"
			"\n"
			"" C_CHECK ""
			"static: check $(TARGET_S)\n"
			"\n"
			"" TARGET_C_S ""
			"" C_SO ""
			"clean:\n"
			"\t@$(RM) $(TARGET_S) $(OBJ_C_S) $(COV)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_coverage_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_coverage_shared(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR_D := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C_D := $(patsubst %.c, $(INTDIR_D)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"COVDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/cov/\n"
			"COV := $(patsubst %.c, $(COVDIR)%.gcno, $(SRC_C))\n"
			"COV += $(patsubst %.c, $(COVDIR)%.gcda, $(SRC_C))\n"
			"TARGET_D := $(OUTDIR)test.so\n"
			"\n"
			"" CONFIG ""
			"ifeq ($(COVERAGE), true)\n"
			"GCC_CONFIG_FLAGS += --coverage -fprofile-abs-path\n"
			"endif\n"
			"\n"
			".PHONY: all check shared clean\n"
			"\n"
			"all: shared\n"
			"\n"
			"" C_CHECK ""
			"shared: check $(TARGET_D)\n"
			"\n"
			"" TARGET_C_D ""
			"" C_DO ""
			"clean:\n"
			"\t@$(RM) $(TARGET_D) $(OBJ_C_D) $(COV)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_defines_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_defines_exe(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"DEFINES := -DDEBUG -DVERSION=1\n"
			"" CONFIG ""
			".PHONY: all check compile run debug clean\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_defines_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_defines_static(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_defines_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_defines_shared(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_copyfiles)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_copyfiles(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RM += -r\n"
			"\n"
			".PHONY: all check copyfiles static clean\n"
			"\n"
			"all: static\n"
			"\n"
			"check:\n"
			"\n"
			"copyfiles:\n"
			"\t@cp lib.so .\n"
			"\n"
			"static: check copyfiles\n"
			"\n"
			"clean:\n"
			"\t@$(RM)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_run)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_run(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run debug clean\n"
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
			"debug: check $(TARGET)\n"
			"\t@gdb --args run\n"
			"\n"
			"" CLEAN_EXE_C "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_run_debug)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_run_debug(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			"" CONFIG_DEBUG ""
			".PHONY: all check compile run debug clean\n"
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
			"debug: check $(TARGET)\n"
			"\t@gdb --args run_debug\n"
			"\n"
			"" CLEAN_EXE_C "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_run_run_debug)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_run_run_debug(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			"" CONFIG_DEBUG ""
			".PHONY: all check compile run debug clean\n"
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
			"debug: check $(TARGET)\n"
			"\t@gdb --args run_debug\n"
			"\n"
			"" CLEAN_EXE_C "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_artifact_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_artifact_exe(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run debug artifact clean\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_artifact_lib)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_artifact_lib(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_STATIC_VARS ""
			"" CONFIG ""
			".PHONY: all check static artifact_s artifact clean\n"
			"\n"
			"all: static\n"
			"\n"
			"" C_CHECK ""
			"static: check $(TARGET_S)\n"
			"\n"
			"" TARGET_C_S ""
			"" C_SO ""
			"artifact_s: check $(TARGET_S)\n"
			"\t@mkdir -p $(SLNDIR)tmp/artifact/\n"
			"\t@cp $(TARGET_S) $(SLNDIR)tmp/artifact/test\n"
			"\n"
			"artifact: artifact_s\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET_S) $(SLNDIR)tmp/artifact/test $(OBJ_C_S)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_bin_obj)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_bin_obj(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"TARGET_BIN := $(OUTDIR)test.bin\n"
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
			"\t@$(TLD) -Tlinker.ld --oformat binary $(OBJ_C) $(LDFLAGS) -o $@\n"
			"\n"
			"" C_O ""
			"clean:\n"
			"\t@$(RM) $(TARGET_BIN) $(OBJ_C)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_bin_files)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_bin_files(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
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
			"$(TARGET_BIN): file.bin file.elf\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_elf)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_elf(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRC_C := $(shell find src/ -name '*.c')\n"
			"INTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/\n"
			"OBJ_C := $(patsubst %.c, $(INTDIR)%.o, $(SRC_C))\n"
			"OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"TARGET_ELF := $(OUTDIR)test.elf\n"
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
			"\t@$(TCC) -m$(BITS) -shared -ffreestanding $(OBJ_C) $(LDFLAGS) -o $@\n"
			"\n"
			"" C_O ""
			"clean:\n"
			"\t@$(RM) $(TARGET_ELF) $(OBJ_C)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_fat12)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_fat12(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
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
			"\tsudo apt-get install mtools -y\n"
			"endif\n"
			"\n"
			"fat12: check $(TARGET_FAT12)\n"
			"\n"
			"$(TARGET_FAT12): file.bin file.elf\n"
			"\t@mkdir -p $(@D)\n"
			"\t@dd if=/dev/zero of=$@ bs=512 count=2880 status=none\n"
			"\t@mkfs.fat -F12 -n \"NBOS\" $@\n"
			"\t@mcopy -i $@ $(word 2,$^) \"::$(shell basename $(word 2,$^))\"\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET_FAT12)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_fat12_header)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_fat12_header(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
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
			"\tsudo apt-get install mtools -y\n"
			"endif\n"
			"\n"
			"fat12: check $(TARGET_FAT12)\n"
			"\n"
			"$(TARGET_FAT12): file.bin file.elf\n"
			"\t@mkdir -p $(@D)\n"
			"\t@dd if=/dev/zero of=$@ bs=512 count=2880 status=none\n"
			"\t@dd if=file.bin of=$@ conv=notrunc status=none\n"
			"\t@mcopy -i $@ $(word 2,$^) \"::$(shell basename $(word 2,$^))\"\n"
			"\n"
			"clean:\n"
			"\t@$(RM) $(TARGET_FAT12)\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_configs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_configs(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			"" CONFIG_DEBUG ""
			".PHONY: all check compile run debug clean\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_nasm_bin)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_bin(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" ASM_BIN_VARS ""
			"\n"
			"" CONFIG_NASM ""
			"ifeq ($(CONFIG), Debug)\n"
			"NASM_CONFIG_FLAGS += -g\n"
			"endif\n"
			"\n"
			".PHONY: all check bin clean\n"
			"\n"
			"all: bin\n"
			"\n"
			"" ASM_CHECK ""
			"bin: check $(TARGET_BIN)\n"
			"\n"
			"" TARGET_ASM_BIN ""
			"" ASM_BIN ""
			"" CLEAN_BIN_ASM "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_nasm_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_exe(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" ASM_EXE_VARS ""
			"\n"
			"" CONFIG_NASM ""
			"ifeq ($(CONFIG), Debug)\n"
			"NASM_CONFIG_FLAGS += -g -F dwarf\n"
			"endif\n"
			"\n"
			".PHONY: all check compile run debug clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" ASM_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_ASM_EXE ""
			"" ASM_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_ASM "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_nasm_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_static(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" ASM_STATIC_VARS ""
			"" CONFIG_NASM ""
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_nasm_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_shared(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" ASM_SHARED_VARS ""
			"" CONFIG_NASM ""
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_asm_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_asm_exe(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" S_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run debug clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"" S_CHECK ""
			"compile: check $(TARGET)\n"
			"\n"
			"" TARGET_S ""
			"" S_O ""
			"" RUN_EXE ""
			"" CLEAN_EXE_S "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_asm_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_asm_static(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" S_STATIC_VARS ""
			"" CONFIG ""
			".PHONY: all check static clean\n"
			"\n"
			"all: static\n"
			"\n"
			"" S_CHECK ""
			"static: check $(TARGET_S)\n"
			"\n"
			"" TARGET_S_S ""
			"" S_SO ""
			"" CLEAN_EXE_S_S "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_asm_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_asm_shared(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" S_SHARED_VARS ""
			"" CONFIG ""
			".PHONY: all check shared clean\n"
			"\n"
			"all: shared\n"
			"\n"
			"" S_CHECK ""
			"shared: check $(TARGET_D)\n"
			"\n"
			"" TARGET_S_D ""
			"" S_DO ""
			"" CLEAN_EXE_S_D "");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_c_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_exe(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" C_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run debug clean\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_c_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_static(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_c_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_shared(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_cpp_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cpp_exe(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[1024] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "" CPP_EXE_VARS ""
			"\n"
			"" CONFIG ""
			".PHONY: all check compile run debug clean\n"
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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_cpp_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cpp_static(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_cpp_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cpp_shared(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

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
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_mk_url)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_url(&pgc);

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	pgc_gen_mk(&pgc, &make);

	char buf[2048] = { 0 };
	make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR := $(SLNDIR)bin/$(ARCH)/test/\n"
			"URL := http://ftp.gnu.org/gnu/gcc/gcc-13.1.0/\n"
			"NAME := gcc-13.1.0\n"
			"FORMAT := tar.gz\n"
			"FILE := $(NAME).$(FORMAT)\n"
			"DLPATH := $(SLNDIR)tmp/dl/$(FILE)\n"
			"SRCDIR := $(SLNDIR)tmp/src/$(NAME)/\n"
			"BUILDDIR := $(SLNDIR)tmp/bin/$(ARCH)/$(NAME)/\n"
			"LOGDIR := $(SLNDIR)tmp/logs/$(ARCH)/$(NAME)/\n"
			"\n"
			".PHONY: all check compile clean\n"
			"\n"
			"all: compile\n"
			"\n"
			"check:\n"
			"ifeq (, $(shell which curl))\n"
			"\tsudo apt-get install curl -y\n"
			"endif\n"
			"ifeq (, $(shell apt list --installed 2>/dev/null | grep g++/))\n"
			"\tsudo apt-get install g++ -y\n"
			"endif\n"
			"\n"
			"$(DLPATH):\n"
			"\t@mkdir -p $(@D)\n"
			"\t@cd $(@D) && curl -s -O $(URL)$(FILE)\n"
			"\n"
			"$(SRCDIR)done: $(DLPATH)\n"
			"\t@mkdir -p $(SLNDIR)tmp/src\n"
			"\t@tar xf $(DLPATH) -C $(SLNDIR)tmp/src\n"
			"\t@touch $(SRCDIR)done\n"
			"\n"
			"$(OUTDIR)$(NAME): $(SRCDIR)done\n"
			"\t@mkdir -p $(LOGDIR) $(BUILDDIR) $(OUTDIR)\n"
			"\t@cd $(BUILDDIR) && $(SRCDIR)configure --target=$(ARCH)-elf --prefix=$(OUTDIR) --disable-nls > $(LOGDIR)configure.log 2>&1\n"
			"\t@cd $(BUILDDIR) && make all-gcc > $(LOGDIR)make.log 2>&1\n"
			"\t@touch $(OUTDIR)$(NAME)\n"
			"\n"
			"compile: check $(OUTDIR)$(NAME)\n"
			"\n"
			"clean:\n"
			"\n");

	make_free(&make);
	pgc_free(&pgc);

	END;
}

STEST(t_pgc_gen_mk)
{
	SSTART;
	RUN(t_pgc_gen_mk_empty);
	RUN(t_pgc_gen_mk_args);
	RUN(t_pgc_gen_mk_require);
	RUN(t_pgc_gen_mk_lib_empty);
	RUN(t_pgc_gen_mk_headers);
	RUN(t_pgc_gen_mk_includes);
	RUN(t_pgc_gen_mk_flags);
	RUN(t_pgc_gen_mk_ldflags);
	RUN(t_pgc_gen_mk_libs);
	RUN(t_pgc_gen_mk_depends);
	RUN(t_pgc_gen_mk_coverage_exe);
	RUN(t_pgc_gen_mk_coverage_static);
	RUN(t_pgc_gen_mk_coverage_shared);
	RUN(t_pgc_gen_mk_defines_exe);
	RUN(t_pgc_gen_mk_defines_static);
	RUN(t_pgc_gen_mk_defines_shared);
	RUN(t_pgc_gen_mk_copyfiles);
	RUN(t_pgc_gen_mk_run);
	RUN(t_pgc_gen_mk_run_debug);
	RUN(t_pgc_gen_mk_run_run_debug);
	RUN(t_pgc_gen_mk_artifact_exe);
	RUN(t_pgc_gen_mk_artifact_lib);
	RUN(t_pgc_gen_mk_bin_obj);
	RUN(t_pgc_gen_mk_bin_files);
	RUN(t_pgc_gen_mk_elf);
	RUN(t_pgc_gen_mk_fat12);
	RUN(t_pgc_gen_mk_fat12_header);
	RUN(t_pgc_gen_mk_configs);
	RUN(t_pgc_gen_mk_nasm_bin);
	RUN(t_pgc_gen_mk_nasm_exe);
	RUN(t_pgc_gen_mk_nasm_static);
	RUN(t_pgc_gen_mk_nasm_shared);
	RUN(t_pgc_gen_mk_asm_exe);
	RUN(t_pgc_gen_mk_asm_static);
	RUN(t_pgc_gen_mk_asm_shared);
	RUN(t_pgc_gen_mk_c_exe);
	RUN(t_pgc_gen_mk_c_static);
	RUN(t_pgc_gen_mk_c_shared);
	RUN(t_pgc_gen_mk_cpp_exe);
	RUN(t_pgc_gen_mk_cpp_static);
	RUN(t_pgc_gen_mk_cpp_shared);
	RUN(t_pgc_gen_mk_url);
	SEND;
}
