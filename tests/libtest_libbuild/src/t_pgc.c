#include "gen/pgc.h"

#include "mem.h"
#include "test.h"

TEST(t_pgc_init_free)
{
	START;

	pgc_t gen = { 0 };
	EXPECT_EQ(pgc_init(NULL), NULL);
	mem_oom(1);
	EXPECT_EQ(pgc_init(&gen), NULL);
	mem_oom(0);
	EXPECT_EQ(pgc_init(&gen), &gen);

	pgc_free(NULL);
	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_config)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_config(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_config(&gen, str_null()), 0);
	EXPECT_EQ(pgc_add_config(&gen, str_null()), 1);
	mem_oom(1);
	EXPECT_EQ(pgc_add_config(&gen, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_config(&gen, STRH("Debug")), 2);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_header)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_header(NULL, str_null(), 0), PGC_END);
	EXPECT_EQ(pgc_add_header(&gen, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_header(&gen, str_null(), 0), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_header(&gen, STRH("include/"), 0), 1);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_src)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_src(NULL, str_null(), 0), PGC_END);
	EXPECT_EQ(pgc_add_src(&gen, str_null(), 0), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_src(&gen, str_null(), 0), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_src(&gen, STRH("src/"), 0), 1);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_include)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_include(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_include(&gen, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_include(&gen, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_include(&gen, STRH("src/")), 1);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_flag)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	pgc_add_flag(NULL, str_null(), F_PGC_SRC_C);
	pgc_add_flag(&gen, STR("-Wall"), F_PGC_SRC_C);
	pgc_add_flag(&gen, STR("-Wextra"), F_PGC_SRC_C);

	EXPECT_STRN(gen.src[PGC_SRC_STR_FLAGS][PGC_SRC_C].data, "-Wall -Wextra", 13);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_define)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	pgc_add_define(NULL, str_null(), F_PGC_INTDIR_OBJECT);
	pgc_add_define(&gen, STR("DEBUG"), F_PGC_INTDIR_OBJECT);
	pgc_add_define(&gen, STR("UNICODE"), F_PGC_INTDIR_OBJECT);

	EXPECT_STRN(gen.intdir[PGC_INTDIR_STR_DEFINES][PGC_INTDIR_OBJECT].data, "-DDEBUG -DUNICODE", 13);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_ldflag)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	pgc_add_ldflag(NULL, str_null());
	pgc_add_ldflag(&gen, STR("-lm"));
	pgc_add_ldflag(&gen, STR("-lpthread"));

	EXPECT(str_eq(gen.str[PGC_STR_LDFLAGS], STR("-lm -lpthread")));

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_lib)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_lib(NULL, str_null(), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), PGC_END);
	EXPECT_EQ(pgc_add_lib(&gen, str_null(), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_lib(&gen, str_null(), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_lib(&gen, STR("libs/"), str_null(), PGC_LINK_STATIC, PGC_LIB_INT), 1);
	EXPECT_EQ(pgc_add_lib(&gen, str_null(), STR("a"), PGC_LINK_STATIC, PGC_LIB_INT), 2);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_depend)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_depend(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_depend(&gen, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_depend(&gen, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_depend(&gen, STRH("lib")), 1);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_set_run)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	pgc_set_run(NULL, str_null(), F_PGC_BUILD_EXE);
	pgc_set_run(&gen, STRH("$(TARGET)"), F_PGC_BUILD_EXE);

	EXPECT_STRN(gen.target[PGC_TARGET_STR_RUN][PGC_BUILD_EXE].data, "$(TARGET)", 9);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_set_run_debug)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	pgc_set_run_debug(NULL, str_null(), F_PGC_BUILD_EXE);
	pgc_set_run_debug(&gen, STRH("$(TARGET_DEBUG)"), F_PGC_BUILD_EXE);

	EXPECT_STRN(gen.target[PGC_TARGET_STR_RUN_DBG][PGC_BUILD_EXE].data, "$(TARGET_DEBUG)", 15);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_file)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_file(NULL, str_null(), PGC_FILE_BIN), PGC_END);
	EXPECT_EQ(pgc_add_file(&gen, str_null(), PGC_FILE_BIN), 0);
	EXPECT_EQ(pgc_add_file(&gen, str_null(), PGC_FILE_BIN), 1);
	mem_oom(1);
	EXPECT_EQ(pgc_add_file(&gen, str_null(), PGC_FILE_BIN), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_file(&gen, STRH("src/file.bin"), 0), 2);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_require)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_require(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_require(&gen, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_require(&gen, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_require(&gen, STRH("g++")), 1);

	pgc_free(&gen);

	END;
}

TEST(t_pgc_add_copyfile)
{
	START;

	pgc_t gen = { 0 };
	pgc_init(&gen);

	EXPECT_EQ(pgc_add_copyfile(NULL, str_null()), PGC_END);
	EXPECT_EQ(pgc_add_copyfile(&gen, str_null()), 0);
	mem_oom(1);
	EXPECT_EQ(pgc_add_copyfile(&gen, str_null()), PGC_END);
	mem_oom(0);
	EXPECT_EQ(pgc_add_copyfile(&gen, STRH("lib.so")), 1);

	pgc_free(&gen);

	END;
}

STEST(t_pgc)
{
	SSTART;
	RUN(t_pgc_init_free);
	RUN(t_pgc_add_config);
	RUN(t_pgc_add_header);
	RUN(t_pgc_add_src);
	RUN(t_pgc_add_include);
	RUN(t_pgc_add_flag);
	RUN(t_pgc_add_define);
	RUN(t_pgc_add_ldflag);
	RUN(t_pgc_add_lib);
	RUN(t_pgc_add_depend);
	RUN(t_pgc_set_run);
	RUN(t_pgc_set_run_debug);
	RUN(t_pgc_add_file);
	RUN(t_pgc_add_require);
	RUN(t_pgc_add_copyfile);
	SEND;
}
