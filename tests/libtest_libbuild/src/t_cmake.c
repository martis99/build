#include "gen/cm/cmake.h"

#include "mem.h"
#include "test.h"

TEST(t_cmake_init_free)
{
	START;

	cmake_t cmake = { 0 };
	EXPECT_EQ(cmake_init(NULL, 0, 0, 0), NULL);
	mem_oom(1);
	EXPECT_EQ(cmake_init(&cmake, 1, 0, 0), NULL);
	EXPECT_EQ(cmake_init(&cmake, 0, 1, 0), NULL);
	EXPECT_EQ(cmake_init(&cmake, 0, 0, 1), NULL);
	mem_oom(0);
	EXPECT_EQ(cmake_init(&cmake, 1, 1, 1), &cmake);

	cmake_free(NULL);
	cmake_free(&cmake);

	END;
}

TEST(t_cmake_cmd_add_str)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_cmd_add_str(NULL, 0, str_null()), CMAKE_END);
	EXPECT_EQ(cmake_cmd_add_str(&cmake, 0, str_null()), CMAKE_END);

	uint target  = cmake_add_custom_target(&cmake, STRH("test"));
	uint depends = cmake_add_depends(&cmake, target);

	EXPECT_EQ(cmake_cmd_add_str(&cmake, depends, STRH("test")), depends);
	mem_oom(1);
	EXPECT_EQ(cmake_cmd_add_str(&cmake, depends, str_null()), CMAKE_END);
	mem_oom(0);
	EXPECT_EQ(cmake_cmd_add_str(&cmake, depends, STRH("test")), depends);

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_file)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_file(NULL, CMAKE_FILE_GLOB_RECURSE, str_null()), CMAKE_END);
	uint sources = cmake_file(&cmake, CMAKE_FILE_GLOB_RECURSE, STRH("sources"));
	cmake_cmd_add_str(&cmake, sources, STRH("src"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE sources src)\n"
			"\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_exe)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_add_exe(NULL, str_null(), 0), CMAKE_END);
	EXPECT_EQ(cmake_add_exe(&cmake, STR("test"), 0), CMAKE_END);

	uint sources = cmake_file(&cmake, CMAKE_FILE_GLOB_RECURSE, STRH("sources"));
	cmake_cmd_add_str(&cmake, sources, STRH("src"));
	EXPECT_EQ(cmake_add_exe(&cmake, STRH("test"), sources), 1);
	mem_oom(1);
	EXPECT_EQ(cmake_add_exe(&cmake, STRH("test"), sources), CMAKE_END);
	mem_oom(0);

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE sources src)\n"
			"\n"
			"add_executable(test ${sources})\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_lib)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_add_lib(NULL, str_null(), CMAKE_LIB_STATIC, 0), CMAKE_END);
	EXPECT_EQ(cmake_add_lib(&cmake, STR("test"), CMAKE_LIB_STATIC, 0), CMAKE_END);

	uint sources = cmake_file(&cmake, CMAKE_FILE_GLOB_RECURSE, STRH("sources"));
	cmake_cmd_add_str(&cmake, sources, STRH("src"));
	EXPECT_EQ(cmake_add_lib(&cmake, STRH("test"), CMAKE_LIB_STATIC, sources), 1);

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE sources src)\n"
			"\n"
			"add_library(test STATIC ${sources})\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_custom_target)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_add_custom_target(NULL, str_null()), CMAKE_END);
	EXPECT_EQ(cmake_add_custom_target(&cmake, STRH("test")), 0);

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_custom_cmd_target_null)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_add_custom_cmd_target(NULL, 0, CMAKE_CMD_TARGET_PRE_BUILD, str_null()), CMAKE_END);
	EXPECT_EQ(cmake_add_custom_cmd_target(&cmake, 0, CMAKE_CMD_TARGET_PRE_BUILD, str_null()), CMAKE_END);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	EXPECT_EQ(cmake_add_custom_cmd_target(&cmake, target, CMAKE_CMD_TARGET_PRE_BUILD, str_null()), 1);

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"add_custom_command()\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_custom_cmd_target)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	EXPECT_EQ(cmake_add_custom_cmd_target(&cmake, target, CMAKE_CMD_TARGET_PRE_BUILD, STRH("command")), 1);

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"add_custom_command(TARGET test PRE_BUILD\n"
			"\tCOMMAND command\n"
			")\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_depends)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target  = cmake_add_custom_target(&cmake, STRH("test"));
	uint depends = cmake_add_depends(&cmake, target);

	cmake_cmd_add_str(&cmake, depends, str_null());
	cmake_cmd_add_str(&cmake, depends, STRH("dep1"));
	cmake_cmd_add_str(&cmake, depends, STRH("dep2"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"add_dependencies(test dep1 dep2)\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_target_cmd)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_target_cmd(NULL, CMAKE_CMD_TARGET_DEFINES, 0, CMAKE_SCOPE_PRIVATE), CMAKE_END);
	EXPECT_EQ(cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_DEFINES, 0, CMAKE_SCOPE_PRIVATE), CMAKE_END);
	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	mem_oom(1);
	EXPECT_EQ(cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_DEFINES, target, CMAKE_SCOPE_PRIVATE), CMAKE_END);
	mem_oom(0);
	EXPECT_EQ(cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_DEFINES, target, CMAKE_SCOPE_PRIVATE), 1);

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_target_defines)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target  = cmake_add_custom_target(&cmake, STRH("test"));
	uint depends = cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_DEFINES, target, CMAKE_SCOPE_PRIVATE);

	cmake_cmd_add_str(&cmake, depends, str_null());
	cmake_cmd_add_str(&cmake, depends, STRH("def1"));
	cmake_cmd_add_str(&cmake, depends, STRH("def2"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"target_compile_definitions(test PRIVATE def1 def2)\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_target_compile_opts)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	uint opts   = cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_COMPILE_OPTS, target, CMAKE_SCOPE_PRIVATE);

	cmake_cmd_add_str(&cmake, opts, str_null());
	cmake_cmd_add_str(&cmake, opts, STRH("opt1"));
	cmake_cmd_add_str(&cmake, opts, STRH("opt2"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"target_compile_options(test PRIVATE\n"
			"\topt1\n"
			"\topt2\n"
			")\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_target_include_dirs)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	uint opts   = cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_INCLUDE_DIRS, target, CMAKE_SCOPE_PRIVATE);

	cmake_cmd_add_str(&cmake, opts, str_null());
	cmake_cmd_add_str(&cmake, opts, STRH("include/"));
	cmake_cmd_add_str(&cmake, opts, STRH("src/"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"target_include_directories(test PRIVATE include/ src/)\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_target_link_dirs)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	uint dirs   = cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_LINK_DIRS, target, CMAKE_SCOPE_PRIVATE);

	cmake_cmd_add_str(&cmake, dirs, str_null());
	cmake_cmd_add_str(&cmake, dirs, STRH("lib1/"));
	cmake_cmd_add_str(&cmake, dirs, STRH("lib2/"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"target_link_directories(test PRIVATE lib1/ lib2/)\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_target_link_libs)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	uint libs   = cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_LINK_LIBS, target, CMAKE_SCOPE_PRIVATE);

	cmake_cmd_add_str(&cmake, libs, str_null());
	cmake_cmd_add_str(&cmake, libs, STRH("lib1"));
	cmake_cmd_add_str(&cmake, libs, STRH("lib2"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"target_link_libraries(test PRIVATE lib1 lib2)\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_target_link_opts)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	uint opts   = cmake_target_cmd(&cmake, CMAKE_CMD_TARGET_LINK_OPTS, target, CMAKE_SCOPE_PRIVATE);

	cmake_cmd_add_str(&cmake, opts, str_null());
	cmake_cmd_add_str(&cmake, opts, STRH("-static"));
	cmake_cmd_add_str(&cmake, opts, STRH("-nostartfiles"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"target_link_options(test PRIVATE -static -nostartfiles)\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_target_prop)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	uint props  = cmake_set_target_props(&cmake, target);

	EXPECT_EQ(cmake_add_target_prop(NULL, CMAKE_END, CMAKE_TARGET_PROP_ARC_OUT_DIR, str_null()), CMAKE_END);
	EXPECT_EQ(cmake_add_target_prop(&cmake, CMAKE_END, CMAKE_TARGET_PROP_ARC_OUT_DIR, str_null()), CMAKE_END);
	EXPECT_EQ(cmake_add_target_prop(&cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR, str_null()), props);
	mem_oom(1);
	EXPECT_EQ(cmake_add_target_prop(&cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR, str_null()), CMAKE_END);
	mem_oom(0);

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"set_target_properties(test PROPERTIES\n"
			")\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_add_target_props)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	uint target = cmake_add_custom_target(&cmake, STRH("test"));
	uint props  = cmake_set_target_props(&cmake, target);

	cmake_add_target_prop(&cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR, STRH("bin/arc"));
	cmake_add_target_prop(&cmake, props, CMAKE_TARGET_PROP_LIB_OUT_DIR, STRH("bin/lib"));

	char buf[256] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"set_target_properties(test PROPERTIES\n"
			"\tARCHIVE_OUTPUT_DIRECTORY bin/arc\n"
			"\tLIBRARY_OUTPUT_DIRECTORY bin/lib\n"
			")\n");

	cmake_free(&cmake);

	END;
}

TEST(t_cmake_print)
{
	START;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 1, 1, 1);

	EXPECT_EQ(cmake_print(NULL, PRINT_DST_NONE()), 0);
	char buf[256] = { 0 };
	EXPECT_EQ(cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_STR(buf, "");

	cmake_free(&cmake);

	END;
}

STEST(t_cmake)
{
	SSTART;
	RUN(t_cmake_init_free);
	RUN(t_cmake_cmd_add_str);
	RUN(t_cmake_file);
	RUN(t_cmake_add_exe);
	RUN(t_cmake_add_lib);
	RUN(t_cmake_add_custom_target);
	RUN(t_cmake_add_custom_cmd_target_null);
	RUN(t_cmake_add_custom_cmd_target);
	RUN(t_cmake_add_depends);
	RUN(t_cmake_target_cmd);
	RUN(t_cmake_target_defines);
	RUN(t_cmake_target_compile_opts);
	RUN(t_cmake_target_include_dirs);
	RUN(t_cmake_target_link_dirs);
	RUN(t_cmake_target_link_libs);
	RUN(t_cmake_target_link_opts);
	RUN(t_cmake_add_target_prop);
	RUN(t_cmake_add_target_props);
	RUN(t_cmake_print);
	SEND;
}
