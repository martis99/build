#include "gen/pgc.h"

#include "mem.h"
#include "test.h"

TEST(t_pgc_init_free)
{
	START;

	pgc_t pgc = { 0 };
	EXPECT_EQ(pgc_init(NULL), NULL);
	mem_oom(1);
	EXPECT_EQ(pgc_init(&pgc), NULL);
	mem_oom(0);
	EXPECT_EQ(pgc_init(&pgc), &pgc);

	pgc_free(NULL);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_arch)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_arch(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_arch(&pgc, str_null()), 0);
	EXPECT_EQ(pgc_add_arch(&pgc, str_null()), 1);
	mem_oom(1);
	EXPECT_EQ(pgc_add_arch(&pgc, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_arch(&pgc, STRH("x86_64")), 2);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "ARCHS\n"
			"    x86_64\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_get_arch)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_arch(&pgc, STRH("x86_64"));

	EXPECT_EQ(pgc_get_arch(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_get_arch(&pgc, str_null()), PGC_END);
	EXPECT_EQ(pgc_get_arch(&pgc, STR("x86_64")), 0);

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_config)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_config(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_config(&pgc, str_null()), 0);
	EXPECT_EQ(pgc_add_config(&pgc, str_null()), 1);
	mem_oom(1);
	EXPECT_EQ(pgc_add_config(&pgc, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_config(&pgc, STRH("Debug")), 2);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "CONFIGS\n"
			"    Debug\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_get_config)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_config(&pgc, STRH("Debug"));

	EXPECT_EQ(pgc_get_config(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_get_config(&pgc, str_null()), PGC_END);
	EXPECT_EQ(pgc_get_config(&pgc, STR("Debug")), 0);

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_header)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_header(NULL, str_null(), 0), PGC_END);
	EXPECT_EQ(pgc_add_header(&pgc, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_header(&pgc, str_null(), 0), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_header(&pgc, STRH("include/"), 0), 1);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "HEADERS\n"
			"    include/ (0x0000)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_src)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_src(NULL, str_null(), 0), PGC_END);
	EXPECT_EQ(pgc_add_src(&pgc, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_src(&pgc, str_null(), 0), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_src(&pgc, STRH("src/"), 0), 1);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SRCS\n"
			"    src/ (0x0000)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_include)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_include(NULL, str_null(), PGC_SCOPE_PRIVATE), PGC_END);
	EXPECT_EQ(pgc_add_include(&pgc, str_null(), PGC_SCOPE_PRIVATE), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_include(&pgc, str_null(), PGC_SCOPE_PRIVATE), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_include(&pgc, STRH("include/"), PGC_SCOPE_PRIVATE), 1);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    include/ (PRIVATE)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_flag)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_flag(NULL, str_null(), F_PGC_SRC_C);
	pgc_add_flag(&pgc, STR("-Wall"), F_PGC_SRC_C);
	pgc_add_flag(&pgc, STR("-Wextra"), F_PGC_SRC_C);

	EXPECT_STRN(pgc.src[PGC_SRC_STR_FLAGS][PGC_SRC_C].data, "-Wall -Wextra", 13);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "FLAGS\n"
			"    C: -Wall -Wextra\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_define)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_define(NULL, str_null(), F_PGC_INTDIR_OBJECT);
	pgc_add_define(&pgc, STR("DEBUG"), F_PGC_INTDIR_OBJECT);
	pgc_add_define(&pgc, STR("UNICODE"), F_PGC_INTDIR_OBJECT);

	EXPECT_STRN(pgc.intdir[PGC_INTDIR_STR_DEFINES][PGC_INTDIR_OBJECT].data, "-DDEBUG -DUNICODE", 13);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEFINES\n"
			"    OBJECT: -DDEBUG -DUNICODE\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_ldflag)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_ldflag(NULL, str_null());
	pgc_add_ldflag(&pgc, STR("-lm"));
	pgc_add_ldflag(&pgc, STR("-lpthread"));

	EXPECT(str_eq(pgc.str[PGC_STR_LDFLAGS], STR("-lm -lpthread")));

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LDFLAGS: -lm -lpthread\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_lib)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_lib(NULL, str_null(), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), PGC_END);
	EXPECT_EQ(pgc_add_lib(&pgc, str_null(), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_lib(&pgc, str_null(), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), PGC_END);
	mem_oom(0);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    (STATIC, INT)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_lib_dir)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_lib(&pgc, STRH("libs/"), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), 0);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: libs/ (STATIC, INT)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_lib_name)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_lib(&pgc, str_null(), STRH("a"), PGC_LINK_STATIC, PGC_LIB_INT), 0);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    name: a (STATIC, INT)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_depend)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_depend(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_depend(&pgc, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_depend(&pgc, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_depend(&pgc, STRH("lib")), 1);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEPENDS\n"
			"    lib\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_set_cwd)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_set_cwd(NULL, str_null());
	pgc_set_cwd(&pgc, STRH("test"));

	EXPECT_STRN(pgc.str[PGC_STR_CWD].data, "test", 4);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "CWD: test\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_set_run)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_set_run(NULL, str_null(), F_PGC_BUILD_EXE);
	pgc_set_run(&pgc, STRH("$(TARGET)"), F_PGC_BUILD_EXE);

	EXPECT_STRN(pgc.target[PGC_TARGET_STR_RUN][PGC_BUILD_EXE].data, "$(TARGET)", 9);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RUN\n"
			"    EXE: $(TARGET)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_set_run_debug)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_set_run_debug(NULL, str_null(), F_PGC_BUILD_EXE);
	pgc_set_run_debug(&pgc, STRH("$(TARGET_DEBUG)"), F_PGC_BUILD_EXE);

	EXPECT_STRN(pgc.target[PGC_TARGET_STR_RUN_DBG][PGC_BUILD_EXE].data, "$(TARGET_DEBUG)", 15);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RUN_DBG\n"
			"    EXE: $(TARGET_DEBUG)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_file)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_file(NULL, str_null(), PGC_FILE_BIN), PGC_END);
	EXPECT_EQ(pgc_add_file(&pgc, str_null(), PGC_FILE_BIN), 0);
	EXPECT_EQ(pgc_add_file(&pgc, str_null(), PGC_FILE_BIN), 1);
	mem_oom(1);
	EXPECT_EQ(pgc_add_file(&pgc, str_null(), PGC_FILE_BIN), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_file(&pgc, STRH("src/file.bin"), 0), 2);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "FILES\n"
			"    src/file.bin (0x0000)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_require)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_require(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_require(&pgc, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_require(&pgc, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_require(&pgc, STRH("g++")), 1);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "REQUIRES\n"
			"    g++\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_add_copyfile)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_add_copyfile(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_copyfile(&pgc, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_copyfile(&pgc, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_copyfile(&pgc, STRH("lib.so")), 1);

	char buf[1024] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "COPYFILES\n"
			"    lib.so\n");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_print)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	EXPECT_EQ(pgc_print(NULL, PRINT_DST_NONE()), 0);

	char buf[2048] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "");

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_t pgcr = { 0 };

	EXPECT_EQ(pgc_replace_vars(NULL, NULL, NULL, NULL, 0), NULL);
	EXPECT_EQ(pgc_replace_vars(&pgc, NULL, NULL, NULL, 0), NULL);
	EXPECT_EQ(pgc_replace_vars(&pgc, &pgcr, NULL, NULL, 0), &pgcr);

	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_str)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc.str[PGC_STR_NAME] = STR("$(PROJNAME)");

	pgc_t pgcr = { 0 };
	str_t from = STR("$(PROJNAME)");
	str_t to   = STR("name");
	pgc_replace_vars(&pgc, &pgcr, &from, &to, 1);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "NAME: name\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_arr)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_arch(&pgc, STRH("$(ARCH)"));

	pgc_t pgcr = { 0 };
	str_t from = STR("$(ARCH)");
	str_t to   = STR("x86_64");
	mem_oom(1);
	EXPECT_EQ(pgc_replace_vars(&pgc, &pgcr, &from, &to, 1), NULL);
	mem_oom(0);

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_arr_str)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_arch(&pgc, STRH("$(ARCH)"));

	pgc_t pgcr = { 0 };
	str_t from = STR("$(ARCH)");
	str_t to   = STR("x86_64");
	mem_oom(1);
	EXPECT_EQ(pgc_replace_vars(&pgc, &pgcr, &from, &to, 1), NULL);
	mem_oom(0);
	pgc_replace_vars(&pgc, &pgcr, &from, &to, 1);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "ARCHS\n"
			"    x86_64\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_arr_str_flag)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_header(&pgc, STRH("$(DIR)"), 0);

	pgc_t pgcr = { 0 };
	str_t from = STR("$(DIR)");
	str_t to   = STR("include/");
	mem_oom(1);
	EXPECT_EQ(pgc_replace_vars(&pgc, &pgcr, &from, &to, 1), NULL);
	mem_oom(0);
	pgc_replace_vars(&pgc, &pgcr, &from, &to, 1);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "HEADERS\n"
			"    include/ (0x0000)\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_arr_include)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_include(&pgc, STRH("$(DIR)"), PGC_SCOPE_PRIVATE);

	pgc_t pgcr = { 0 };
	str_t from = STR("$(DIR)");
	str_t to   = STR("include/");

	pgc_replace_vars(&pgc, &pgcr, &from, &to, 1);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    include/ (PRIVATE)\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_arr_lib)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc_add_lib(&pgc, STRH("$(DIR)"), STRH("$(NAME)"), PGC_LINK_STATIC, PGC_LIB_INT);

	pgc_t pgcr   = { 0 };
	str_t from[] = {
		STR("$(DIR)"),
		STR("$(NAME)"),
	};
	str_t to[] = {
		STR("libs/"),
		STR("a"),
	};
	pgc_replace_vars(&pgc, &pgcr, from, to, 2);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: libs/ name: a (STATIC, INT)\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_src)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc.src[PGC_SRC_STR_FLAGS][PGC_SRC_C] = STR("-m$(BITS)");

	pgc_t pgcr = { 0 };
	str_t from = STR("$(BITS)");
	str_t to   = STR("64");
	pgc_replace_vars(&pgc, &pgcr, &from, &to, 1);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "FLAGS\n"
			"    C: -m64\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_intdir)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc.intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STR("$(INTDIR)");

	pgc_t pgcr = { 0 };
	str_t from = STR("$(INTDIR)");
	str_t to   = STR("bin");
	pgc_replace_vars(&pgc, &pgcr, &from, &to, 1);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INTDIR\n"
			"    OBJECT: bin\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_target)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc.target[PGC_TARGET_STR_RUN][PGC_BUILD_EXE] = STR("$(OUTDIR)$(NAME)");

	pgc_t pgcr   = { 0 };
	str_t from[] = {
		STR("$(OUTDIR)"),
		STR("$(NAME)"),
	};
	str_t to[] = {
		STR("bin/projects/test/"),
		STR("test"),
	};
	pgc_replace_vars(&pgc, &pgcr, from, to, 2);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RUN\n"
			"    EXE: bin/projects/test/test\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_replace_vars_slash)
{
	START;

	pgc_t pgc = { 0 };
	pgc_init(&pgc);

	pgc.target[PGC_TARGET_STR_RUN][PGC_BUILD_EXE] = STR("$(OUTDIR)$(NAME)");

	pgc_t pgcr   = { 0 };
	str_t from[] = {
		STR("$(OUTDIR)"),
		STR("$(NAME)"),
	};
	str_t to[] = {
		STR("bin\\projects\\test\\"),
		STR("test"),
	};
	pgc_replace_vars(&pgc, &pgcr, from, to, 2);

	char buf[1024] = { 0 };
	pgc_print(&pgcr, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RUN\n"
			"    EXE: bin/projects/test/test\n");

	pgc_free(&pgcr);
	pgc_free(&pgc);

	END;
}

STEST(t_pgc)
{
	SSTART;
	RUN(t_pgc_init_free);
	RUN(t_pgc_add_arch);
	RUN(t_pgc_get_arch);
	RUN(t_pgc_add_config);
	RUN(t_pgc_get_config);
	RUN(t_pgc_add_header);
	RUN(t_pgc_add_src);
	RUN(t_pgc_add_include);
	RUN(t_pgc_add_flag);
	RUN(t_pgc_add_define);
	RUN(t_pgc_add_ldflag);
	RUN(t_pgc_add_lib);
	RUN(t_pgc_add_lib_dir);
	RUN(t_pgc_add_lib_name);
	RUN(t_pgc_add_depend);
	RUN(t_pgc_set_cwd);
	RUN(t_pgc_set_run);
	RUN(t_pgc_set_run_debug);
	RUN(t_pgc_add_file);
	RUN(t_pgc_add_require);
	RUN(t_pgc_add_copyfile);
	RUN(t_pgc_replace_vars);
	RUN(t_pgc_replace_vars_str);
	RUN(t_pgc_replace_vars_arr);
	RUN(t_pgc_replace_vars_arr_str);
	RUN(t_pgc_replace_vars_arr_str_flag);
	RUN(t_pgc_replace_vars_arr_include);
	RUN(t_pgc_replace_vars_arr_lib);
	RUN(t_pgc_replace_vars_src);
	RUN(t_pgc_replace_vars_intdir);
	RUN(t_pgc_replace_vars_target);
	RUN(t_pgc_replace_vars_slash);
	RUN(t_pgc_print);
	SEND;
}
