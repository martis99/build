#include "gen/proj_gen_pgc.h"

#include "gen/sln.h"

#include "test.h"

TEST(t_proj_gen_pgc_empty)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	pgc_init(&pgc);

	EXPECT_EQ(proj_gen_pgc(NULL, NULL, NULL), 1);
	EXPECT_EQ(proj_gen_pgc(&proj, NULL, NULL), 1);

	EXPECT_EQ(proj_gen_pgc(&proj, &sln_props, &pgc), 0);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_filename)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_FILENAME].flags	 = PROP_SET;
	proj.props[PROJ_PROP_FILENAME].value.val = STR("test");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "NAME: test\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_outdir)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_OUTDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_OUTDIR].value.val = STR("$(SLNDIR)");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR: $(SLNDIR)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_target_lib)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_OUTDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_OUTDIR].value.val = STR("$(SLNDIR)");
	proj.props[PROJ_PROP_NAME].flags       = PROP_SET;
	proj.props[PROJ_PROP_NAME].value.val   = STR("test");
	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_LIB;

	proj.name = proj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "NAME: test\n"
			"OUTDIR: $(SLNDIR)\n"
			"TARGET\n"
			"    STATIC: $(SLNDIR)test.a\n"
			"    SHARED: $(SLNDIR)test.so\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_target_exe)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_OUTDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_OUTDIR].value.val = STR("$(SLNDIR)");
	proj.props[PROJ_PROP_NAME].flags       = PROP_SET;
	proj.props[PROJ_PROP_NAME].value.val   = STR("test");
	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;

	proj.name = proj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "NAME: test\n"
			"OUTDIR: $(SLNDIR)\n"
			"COVDIR: $(SLNDIR)cov/\n"
			"TARGET\n"
			"    EXE: $(SLNDIR)test\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_intdir_lib)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_INTDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_INTDIR].value.val = STR("$(SLNDIR)");
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");
	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_LIB;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[512] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"INTDIR\n"
			"    STATIC: $(SLNDIR)static/\n"
			"    SHARED: $(SLNDIR)shared/\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_intdir_exe)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_INTDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_INTDIR].value.val = STR("$(SLNDIR)");
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");
	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[512] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"INTDIR\n"
			"    OBJECT: $(SLNDIR)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_intdir_fat12)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_FAT12;
	proj.props[PROJ_PROP_INTDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_INTDIR].value.val = STR("$(SLNDIR)");
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[512] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_arch)
{
	START;

	proj_t proj			 = { 0 };
	pgc_t pgc			 = { 0 };
	dict_t projects			 = { 0 };
	prop_t sln_props[__SLN_PROP_MAX] = { 0 };

	sln_props[SLN_PROP_ARCHS].flags = PROP_SET;
	arr_init(&sln_props[SLN_PROP_ARCHS].arr, 1, sizeof(prop_str_t));

	prop_str_t *arch = arr_get(&sln_props[SLN_PROP_ARCHS].arr, arr_add(&sln_props[SLN_PROP_ARCHS].arr));

	arch->val = STR("x86_64");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "ARCHS\n"
			"    x86_64\n");

	arr_free(&sln_props[SLN_PROP_ARCHS].arr);

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_config)
{
	START;

	proj_t proj			 = { 0 };
	pgc_t pgc			 = { 0 };
	dict_t projects			 = { 0 };
	prop_t sln_props[__SLN_PROP_MAX] = { 0 };

	sln_props[SLN_PROP_CONFIGS].flags = PROP_SET;
	arr_init(&sln_props[SLN_PROP_CONFIGS].arr, 1, sizeof(prop_str_t));

	prop_str_t *config = arr_get(&sln_props[SLN_PROP_CONFIGS].arr, arr_add(&sln_props[SLN_PROP_CONFIGS].arr));

	config->val = STR("Debug");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "CONFIGS\n"
			"    Debug\n");

	arr_free(&sln_props[SLN_PROP_CONFIGS].arr);

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_include_c)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_INCLUDE].flags	= PROP_SET;
	proj.props[PROJ_PROP_INCLUDE].value.val = STR("include/");
	proj.props[PROJ_PROP_LANGS].flags	= PROP_SET;
	proj.props[PROJ_PROP_LANGS].mask	= (1 << LANG_C);

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "HEADERS\n"
			"    include/ (H)\n"
			"INCLUDES\n"
			"    include/ (PUBLIC)\n"
			"FLAGS\n"
			"    -Wall -Wextra -Werror -pedantic (C)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_source_c)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");
	proj.props[PROJ_PROP_LANGS].flags      = PROP_SET;
	proj.props[PROJ_PROP_LANGS].mask       = (1 << LANG_C);

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "HEADERS\n"
			"    src/ (H)\n"
			"SRCS\n"
			"    src/ (C)\n"
			"INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"FLAGS\n"
			"    -Wall -Wextra -Werror -pedantic (C)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_artifact)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_ARTIFACT].flags	 = PROP_SET;
	proj.props[PROJ_PROP_ARTIFACT].value.val = STR("artifact");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "ARTIFACT\n"
			"    EXE: artifact\n"
			"    STATIC: artifact\n"
			"    SHARED: artifact\n"
			"    ELF: artifact\n"
			"    BIN: artifact\n"
			"    FAT12: artifact\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_args)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_ARGS].flags     = PROP_SET;
	proj.props[PROJ_PROP_ARGS].value.val = STR("args");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "ARGS: args\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_includes)
{
	START;

	proj_t proj	 = { 0 };
	proj_t iproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.includes, 1, sizeof(proj_t *));
	*(proj_t **)arr_get(&proj.includes, arr_add(&proj.includes)) = &iproj;

	pgc_init(&iproj.pgcr);
	pgc_add_include(&iproj.pgcr, STR("$(SLNDIR)iproj/src/"), PGC_SCOPE_PRIVATE);
	pgc_add_include(&iproj.pgcr, STR("$(SLNDIR)iproj/include/"), PGC_SCOPE_PUBLIC);

	iproj.rel_dir = (pathv_t){
		.path = "$(SLNDIR)iproj/",
		.len  = sizeof("$(SLNDIR)iproj/") - 1,
	};

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    $(SLNDIR)iproj/include/ (PRIVATE)\n");

	pgc_free(&iproj.pgcr);
	pgc_free(&pgc);

	arr_free(&proj.includes);

	END;
}

TEST(t_proj_gen_pgc_defines)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_DEFINES].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_DEFINES].arr, sizeof(prop_str_t), 1);
	prop_str_t *def = arr_get(&proj.props[PROJ_PROP_DEFINES].arr, arr_add(&proj.props[PROJ_PROP_DEFINES].arr));

	def->val = STR("DEBUG");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEFINES\n"
			"    DEBUG (OBJECT | STATIC | SHARED)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_DEFINES].arr);

	END;
}

TEST(t_proj_gen_pgc_flags)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_FLAGS].flags = PROP_SET;
	proj.props[PROJ_PROP_FLAGS].mask  = (1 << FLAG_STD_C99) | (1 << FLAG_FREESTANDING);

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "FLAGS\n"
			"    -std=c99 (C)\n"
			"    -ffreestanding (C)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_c_flags)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_LANGS].flags = PROP_SET;
	proj.props[PROJ_PROP_LANGS].mask  = (1 << LANG_C);

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "FLAGS\n"
			"    -Wall -Wextra -Werror -pedantic (C)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_cpp_flags)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_LANGS].flags = PROP_SET;
	proj.props[PROJ_PROP_LANGS].mask  = (1 << LANG_CPP);

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LDFLAGS\n"
			"    -lstdc++\n"
			"FLAGS\n"
			"    -Wall -Wextra -Werror -pedantic (CPP)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_link_static)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_STATIC,
	};

	dproj.props[PROJ_PROP_TYPE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask	= PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags	= PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val	= STR("test");
	dproj.props[PROJ_PROP_SOURCE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_link_shared)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	dproj.props[PROJ_PROP_TYPE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask	= PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags	= PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val	= STR("test");
	dproj.props[PROJ_PROP_SOURCE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEFINES\n"
			"    TEST_DLL (SHARED)\n");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_link_object)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	proj_t sproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	proj.props[PROJ_PROP_TYPE].flags = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	 = PROJ_TYPE_EXE;

	dproj.props[PROJ_PROP_TYPE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask	= PROJ_TYPE_EXE;
	dproj.props[PROJ_PROP_NAME].flags	= PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val	= STR("test");
	dproj.props[PROJ_PROP_SOURCE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEFINES\n"
			"    TEST_DLL (OBJECT)\n");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_build_shared)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_LIB;
	proj.props[PROJ_PROP_NAME].flags       = PROP_SET;
	proj.props[PROJ_PROP_NAME].value.val   = STR("test");
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	proj.name = proj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "NAME: test\n"
			"INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"DEFINES\n"
			"    TEST_BUILD_DLL (SHARED)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_depend_ext_static)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.depends, arr_add(&proj.depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_STATIC,
	};

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEPENDS\n"
			"    test_s\n");

	pgc_free(&pgc);

	arr_free(&proj.depends);

	END;
}

TEST(t_proj_gen_pgc_depend_ext_shared)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.depends, arr_add(&proj.depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEPENDS\n"
			"    test_d\n");

	pgc_free(&pgc);

	arr_free(&proj.depends);

	END;
}

TEST(t_proj_gen_pgc_depend_ext_exe)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.depends, arr_add(&proj.depends)) = (proj_dep_t){
		.proj = &dproj,
	};

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_EXE;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "DEPENDS\n"
			"    test\n");

	pgc_free(&pgc);

	arr_free(&proj.depends);

	END;
}

TEST(t_proj_gen_pgc_depend_exe_int_static)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.depends, arr_add(&proj.depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_STATIC,
	};

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.props[PROJ_PROP_TYPE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask	= PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags	= PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val	= STR("test");
	dproj.props[PROJ_PROP_SOURCE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n");

	pgc_free(&pgc);

	arr_free(&proj.depends);

	END;
}

TEST(t_proj_gen_pgc_depend_exe_int_shared)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.depends, arr_add(&proj.depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.props[PROJ_PROP_TYPE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask	= PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags	= PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val	= STR("test");
	dproj.props[PROJ_PROP_SOURCE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"DEPENDS\n"
			"    test_d\n");

	pgc_free(&pgc);

	arr_free(&proj.depends);

	END;
}

TEST(t_proj_gen_pgc_depend_exe_ext_static)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.depends, arr_add(&proj.depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_STATIC,
	};

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"DEPENDS\n"
			"    test_s\n");

	pgc_free(&pgc);

	arr_free(&proj.depends);

	END;
}

TEST(t_proj_gen_pgc_depend_exe_ext_shared)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.depends, arr_add(&proj.depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"DEPENDS\n"
			"    test_d\n");

	pgc_free(&pgc);

	arr_free(&proj.depends);

	END;
}

TEST(t_proj_gen_pgc_cflags)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_FLAGS].flags = PROP_SET;
	proj.props[PROJ_PROP_FLAGS].mask  = (1 << FLAG_WHOLEARCHIVE) | (1 << FLAG_ALLOWMULTIPLEDEFINITION);

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LDFLAGS\n"
			"    -Wl,--whole-archive\n"
			"    -Wl,--allow-multiple-definition\n"
			"    -Wl,--no-whole-archive\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_shared_lib)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	dproj.props[PROJ_PROP_TYPE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask	= PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags	= PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val	= STR("test");
	dproj.props[PROJ_PROP_SOURCE].flags	= PROP_SET;
	dproj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	dproj.pgcr.str[PGC_STR_OUTDIR] = STR("$(SLNDIR)");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, SHARED, INT)\n"
			"DEFINES\n"
			"    TEST_DLL (SHARED)\n"
			"COPYFILES\n"
			"    $(SLNDIR)test.so (OBJECT)\n");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_static_ext)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_STATIC,
	};

	dproj.props[PROJ_PROP_TYPE].flags    = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask     = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_LIB].flags     = PROP_SET;
	dproj.props[PROJ_PROP_LIB].value.val = STR("test");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, STATIC, EXT)\n");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_shared_ext)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");
	dproj.props[PROJ_PROP_DLIB].flags     = PROP_SET;
	dproj.props[PROJ_PROP_DLIB].value.val = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, SHARED, EXT)\n"
			"DEFINES\n"
			"    TEST_DLL (SHARED)\n");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_enclude)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_ENCLUDE].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_ENCLUDE].arr, 1, sizeof(prop_str_t));

	prop_str_t *enclude = arr_get(&proj.props[PROJ_PROP_ENCLUDE].arr, arr_add(&proj.props[PROJ_PROP_ENCLUDE].arr));

	enclude->val = STR("include/");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    include/ (PUBLIC)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_ENCLUDE].arr);

	END;
}

TEST(t_proj_gen_pgc_lib)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_LIB].flags	    = PROP_SET;
	proj.props[PROJ_PROP_LIB].value.val = STR("test");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, STATIC, EXT)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_exe_dlib)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	     = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_DLIB].flags     = PROP_SET;
	proj.props[PROJ_PROP_DLIB].value.val = STR("test");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, SHARED, EXT)\n"
			"COPYFILES\n"
			"    $(SLNDIR)test.so (OBJECT)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_lib_dlib)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	     = PROJ_TYPE_LIB;
	proj.props[PROJ_PROP_DLIB].flags     = PROP_SET;
	proj.props[PROJ_PROP_DLIB].value.val = STR("test");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, SHARED, EXT)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_exe_lib_lib)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_STATIC,
	};

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");
	dproj.props[PROJ_PROP_LIB].flags      = PROP_SET;
	dproj.props[PROJ_PROP_LIB].value.val  = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, STATIC, EXT)\n");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_exe_lib_dlib)
{
	START;

	proj_t proj	 = { 0 };
	proj_t dproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.all_depends, 1, sizeof(proj_dep_t));
	*(proj_dep_t *)arr_get(&proj.all_depends, arr_add(&proj.all_depends)) = (proj_dep_t){
		.proj	   = &dproj,
		.link_type = LINK_TYPE_SHARED,
	};

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_SOURCE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SOURCE].value.val = STR("src/");

	dproj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	dproj.props[PROJ_PROP_TYPE].mask      = PROJ_TYPE_LIB;
	dproj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	dproj.props[PROJ_PROP_NAME].value.val = STR("test");
	dproj.props[PROJ_PROP_DLIB].flags     = PROP_SET;
	dproj.props[PROJ_PROP_DLIB].value.val = STR("test");

	dproj.name = dproj.props[PROJ_PROP_NAME].value.val;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "INCLUDES\n"
			"    src/ (PRIVATE)\n"
			"LIBS\n"
			"    dir: $(SLNDIR) name: test (OBJECT | SHARED, SHARED, EXT)\n"
			"DEFINES\n"
			"    TEST_DLL (SHARED)\n"
			"COPYFILES\n"
			"    $(SLNDIR)test.so (OBJECT)\n");

	pgc_free(&pgc);

	arr_free(&proj.all_depends);

	END;
}

TEST(t_proj_gen_pgc_libdir)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags    = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	    = PROJ_TYPE_LIB;
	proj.props[PROJ_PROP_LIBDIRS].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_LIBDIRS].arr, 1, sizeof(prop_str_t));

	prop_str_t *libdir = arr_get(&proj.props[PROJ_PROP_LIBDIRS].arr, arr_add(&proj.props[PROJ_PROP_LIBDIRS].arr));

	libdir->val = STR("libs");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: libs (OBJECT | SHARED, UNKNOWN, EXT)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_LIBDIRS].arr);

	END;
}

TEST(t_proj_gen_pgc_libdir_rel)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags    = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	    = PROJ_TYPE_LIB;
	proj.props[PROJ_PROP_LIBDIRS].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_LIBDIRS].arr, 1, sizeof(prop_str_t));

	prop_str_t *libdir = arr_get(&proj.props[PROJ_PROP_LIBDIRS].arr, arr_add(&proj.props[PROJ_PROP_LIBDIRS].arr));

	libdir->val = STR("$(SLNDIR)libs");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LIBS\n"
			"    dir: $(SLNDIR)libs (OBJECT | SHARED, UNKNOWN, EXT)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_LIBDIRS].arr);

	END;
}

TEST(t_proj_gen_pgc_ldflags)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_FLAGS].flags = PROP_SET;
	proj.props[PROJ_PROP_FLAGS].mask  = (1 << FLAG_STATIC);

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "LDFLAGS\n"
			"    -static\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_require)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags    = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	    = PROJ_TYPE_LIB;
	proj.props[PROJ_PROP_REQUIRE].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_REQUIRE].arr, 1, sizeof(prop_str_t));

	prop_str_t *require = arr_get(&proj.props[PROJ_PROP_REQUIRE].arr, arr_add(&proj.props[PROJ_PROP_REQUIRE].arr));

	require->val = STR("gcc");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "REQUIRES\n"
			"    gcc\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_REQUIRE].arr);

	END;
}

TEST(t_proj_gen_pgc_copyfile_lib)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags      = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	      = PROJ_TYPE_LIB;
	proj.props[PROJ_PROP_COPYFILES].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_COPYFILES].arr, 1, sizeof(prop_str_t));

	prop_str_t *copyfile = arr_get(&proj.props[PROJ_PROP_COPYFILES].arr, arr_add(&proj.props[PROJ_PROP_COPYFILES].arr));

	copyfile->val = STR("file");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "COPYFILES\n"
			"    $(SLNDIR)file (STATIC)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_COPYFILES].arr);

	END;
}

TEST(t_proj_gen_pgc_copyfile_exe)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags      = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	      = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_COPYFILES].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_COPYFILES].arr, 1, sizeof(prop_str_t));

	prop_str_t *copyfile = arr_get(&proj.props[PROJ_PROP_COPYFILES].arr, arr_add(&proj.props[PROJ_PROP_COPYFILES].arr));

	copyfile->val = STR("file");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "COPYFILES\n"
			"    $(SLNDIR)file (OBJECT)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_COPYFILES].arr);

	END;
}

TEST(t_proj_gen_pgc_dcopyfile_lib)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_LIB;
	proj.props[PROJ_PROP_DCOPYFILES].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_DCOPYFILES].arr, 1, sizeof(prop_str_t));

	prop_str_t *copyfile = arr_get(&proj.props[PROJ_PROP_DCOPYFILES].arr, arr_add(&proj.props[PROJ_PROP_DCOPYFILES].arr));

	copyfile->val = STR("file");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "COPYFILES\n"
			"    $(SLNDIR)file (SHARED)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_DCOPYFILES].arr);

	END;
}

TEST(t_proj_gen_pgc_dcopyfile_exe)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags       = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	       = PROJ_TYPE_EXE;
	proj.props[PROJ_PROP_DCOPYFILES].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_DCOPYFILES].arr, 1, sizeof(prop_str_t));

	prop_str_t *copyfile = arr_get(&proj.props[PROJ_PROP_DCOPYFILES].arr, arr_add(&proj.props[PROJ_PROP_DCOPYFILES].arr));

	copyfile->val = STR("file");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "COPYFILES\n"
			"    $(SLNDIR)file (OBJECT)\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_DCOPYFILES].arr);

	END;
}

TEST(t_proj_gen_pgc_header)
{
	START;

	proj_t proj	 = { 0 };
	proj_t hproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	hproj.props[PROJ_PROP_TYPE].flags			= PROP_SET;
	hproj.props[PROJ_PROP_TYPE].mask			= PROJ_TYPE_BIN;
	hproj.pgcr.target[PGC_TARGET_STR_TARGET][PGC_BUILD_BIN] = STR("bin.bin");

	proj.header = &hproj;

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "HEADER: bin.bin\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_files)
{
	START;

	proj_t proj	 = { 0 };
	proj_t fproj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	arr_init(&proj.files, 1, sizeof(proj_t *));
	*(proj_t **)arr_get(&proj.files, arr_add(&proj.files)) = &fproj;

	fproj.props[PROJ_PROP_TYPE].flags			= PROP_SET;
	fproj.props[PROJ_PROP_TYPE].mask			= PROJ_TYPE_BIN;
	fproj.pgcr.target[PGC_TARGET_STR_TARGET][PGC_BUILD_BIN] = STR("file.bin");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "FILES\n"
			"    file.bin (BIN)\n");

	pgc_free(&pgc);

	arr_free(&proj.files);

	END;
}

TEST(t_proj_gen_pgc_wdir)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_WDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_WDIR].value.val = STR("$(PROJDIR)");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "CWD: $(PROJDIR)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_reldir)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.rel_dir = (pathv_t){
		.path = "$(PROJDIR)",
		.len  = sizeof("$(PROJDIR)") - 1,
	};

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "CWD: $(SLNDIR)$(PROJDIR)\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_size)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_SIZE].flags     = PROP_SET;
	proj.props[PROJ_PROP_SIZE].value.val = STR("1024");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "SIZE: 1024\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_run)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags    = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	    = PROJ_TYPE_BIN;
	proj.props[PROJ_PROP_RUN].flags	    = PROP_SET;
	proj.props[PROJ_PROP_RUN].value.val = STR("run");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RUN\n"
			"    BIN: run\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_drun)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_TYPE].flags     = PROP_SET;
	proj.props[PROJ_PROP_TYPE].mask	     = PROJ_TYPE_BIN;
	proj.props[PROJ_PROP_DRUN].flags     = PROP_SET;
	proj.props[PROJ_PROP_DRUN].value.val = STR("run");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "RUN_DBG\n"
			"    BIN: run\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_url_empty)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_URL].flags	    = PROP_SET;
	proj.props[PROJ_PROP_URL].value.val = STR("url");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "URL: url\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_url_name)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_URL].flags	     = PROP_SET;
	proj.props[PROJ_PROP_URL].value.val  = STR("url");
	proj.props[PROJ_PROP_NAME].flags     = PROP_SET;
	proj.props[PROJ_PROP_NAME].value.val = STR("test");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "NAME: test\n"
			"URL: url\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_url_format)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_URL].flags	       = PROP_SET;
	proj.props[PROJ_PROP_URL].value.val    = STR("url");
	proj.props[PROJ_PROP_FORMAT].flags     = PROP_SET;
	proj.props[PROJ_PROP_FORMAT].value.val = STR("zip");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "URL: url\n"
			"FORMAT: zip\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_url_outdir)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_URL].flags	       = PROP_SET;
	proj.props[PROJ_PROP_URL].value.val    = STR("url");
	proj.props[PROJ_PROP_OUTDIR].flags     = PROP_SET;
	proj.props[PROJ_PROP_OUTDIR].value.val = STR("$(SLNDIR)");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "OUTDIR: $(SLNDIR)\n"
			"URL: url\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_url_require)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_URL].flags	    = PROP_SET;
	proj.props[PROJ_PROP_URL].value.val = STR("url");
	proj.props[PROJ_PROP_REQUIRE].flags = PROP_SET;
	arr_init(&proj.props[PROJ_PROP_REQUIRE].arr, 1, sizeof(prop_str_t));

	prop_str_t *require = arr_get(&proj.props[PROJ_PROP_REQUIRE].arr, arr_add(&proj.props[PROJ_PROP_REQUIRE].arr));

	require->val = STR("gzip");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "URL: url\n"
			"REQUIRES\n"
			"    gzip\n");

	pgc_free(&pgc);

	arr_free(&proj.props[PROJ_PROP_REQUIRE].arr);

	END;
}

TEST(t_proj_gen_pgc_url_config)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_URL].flags	       = PROP_SET;
	proj.props[PROJ_PROP_URL].value.val    = STR("url");
	proj.props[PROJ_PROP_CONFIG].flags     = PROP_SET;
	proj.props[PROJ_PROP_CONFIG].value.val = STR("--enable-languages=c");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "URL: url\n"
			"CONFIG: --enable-languages=c\n");

	pgc_free(&pgc);

	END;
}

TEST(t_proj_gen_pgc_url_target)
{
	START;

	proj_t proj	 = { 0 };
	pgc_t pgc	 = { 0 };
	prop_t sln_props = { 0 };

	proj.props[PROJ_PROP_URL].flags	       = PROP_SET;
	proj.props[PROJ_PROP_URL].value.val    = STR("url");
	proj.props[PROJ_PROP_TARGET].flags     = PROP_SET;
	proj.props[PROJ_PROP_TARGET].value.val = STR("all");

	pgc_init(&pgc);

	proj_gen_pgc(&proj, &sln_props, &pgc);

	char buf[256] = { 0 };
	pgc_print(&pgc, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "URL: url\n"
			"TARGETS: all\n");

	pgc_free(&pgc);

	END;
}

STEST(t_proj_gen_pgc)
{
	SSTART;
	RUN(t_proj_gen_pgc_empty);
	RUN(t_proj_gen_pgc_filename);
	RUN(t_proj_gen_pgc_outdir);
	RUN(t_proj_gen_pgc_target_lib);
	RUN(t_proj_gen_pgc_target_exe);
	RUN(t_proj_gen_pgc_intdir_lib);
	RUN(t_proj_gen_pgc_intdir_exe);
	RUN(t_proj_gen_pgc_intdir_fat12);
	RUN(t_proj_gen_pgc_arch);
	RUN(t_proj_gen_pgc_config);
	RUN(t_proj_gen_pgc_include_c);
	RUN(t_proj_gen_pgc_source_c);
	RUN(t_proj_gen_pgc_artifact);
	RUN(t_proj_gen_pgc_args);
	RUN(t_proj_gen_pgc_includes);
	RUN(t_proj_gen_pgc_defines);
	RUN(t_proj_gen_pgc_flags);
	RUN(t_proj_gen_pgc_c_flags);
	RUN(t_proj_gen_pgc_cpp_flags);
	RUN(t_proj_gen_pgc_link_static);
	RUN(t_proj_gen_pgc_link_shared);
	RUN(t_proj_gen_pgc_link_object);
	RUN(t_proj_gen_pgc_build_shared);
	RUN(t_proj_gen_pgc_depend_ext_static);
	RUN(t_proj_gen_pgc_depend_ext_shared);
	RUN(t_proj_gen_pgc_depend_ext_exe);
	RUN(t_proj_gen_pgc_depend_exe_int_static);
	RUN(t_proj_gen_pgc_depend_exe_int_shared);
	RUN(t_proj_gen_pgc_depend_exe_ext_static);
	RUN(t_proj_gen_pgc_depend_exe_ext_shared);
	RUN(t_proj_gen_pgc_cflags);
	RUN(t_proj_gen_pgc_shared_lib);
	RUN(t_proj_gen_pgc_static_ext);
	RUN(t_proj_gen_pgc_shared_ext);
	RUN(t_proj_gen_pgc_enclude);
	RUN(t_proj_gen_pgc_lib);
	RUN(t_proj_gen_pgc_exe_dlib);
	RUN(t_proj_gen_pgc_lib_dlib);
	RUN(t_proj_gen_pgc_exe_lib_lib);
	RUN(t_proj_gen_pgc_exe_lib_dlib);
	RUN(t_proj_gen_pgc_libdir);
	RUN(t_proj_gen_pgc_libdir_rel);
	RUN(t_proj_gen_pgc_ldflags);
	RUN(t_proj_gen_pgc_require);
	RUN(t_proj_gen_pgc_copyfile_lib);
	RUN(t_proj_gen_pgc_copyfile_exe);
	RUN(t_proj_gen_pgc_dcopyfile_lib);
	RUN(t_proj_gen_pgc_dcopyfile_exe);
	RUN(t_proj_gen_pgc_header);
	RUN(t_proj_gen_pgc_files);
	RUN(t_proj_gen_pgc_wdir);
	RUN(t_proj_gen_pgc_reldir);
	RUN(t_proj_gen_pgc_size);
	RUN(t_proj_gen_pgc_run);
	RUN(t_proj_gen_pgc_drun);
	RUN(t_proj_gen_pgc_url_empty);
	RUN(t_proj_gen_pgc_url_name);
	RUN(t_proj_gen_pgc_url_format);
	RUN(t_proj_gen_pgc_url_outdir);
	RUN(t_proj_gen_pgc_url_require);
	RUN(t_proj_gen_pgc_url_config);
	RUN(t_proj_gen_pgc_url_target);
	SEND;
}
