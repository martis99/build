#include "gen/cm/pgc_gen_cm.h"

#include "mem.h"
#include "pgc_gen.h"
#include "test.h"

#define TARGET_PROPERTIES                                                   \
	"set_target_properties(test PROPERTIES\n"                           \
	"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tBUILD_RPATH \".\"\n"                                             \
	"\tOUTPUT_NAME test\n"                                              \
	"\tPREFIX \"\"\n"                                                   \
	")\n"

#define TARGET_PROPERTIES_DEBUG                                                   \
	"set_target_properties(test PROPERTIES\n"                                 \
	"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tARCHIVE_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tLIBRARY_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tRUNTIME_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tBUILD_RPATH \".\"\n"                                                   \
	"\tOUTPUT_NAME test\n"                                                    \
	"\tPREFIX \"\"\n"                                                         \
	")\n"

#define TARGET_PROPERTIES_RELEASE                                                   \
	"set_target_properties(test PROPERTIES\n"                                   \
	"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"         \
	"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"         \
	"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"         \
	"\tARCHIVE_OUTPUT_DIRECTORY_RELEASE $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tLIBRARY_OUTPUT_DIRECTORY_RELEASE $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tRUNTIME_OUTPUT_DIRECTORY_RELEASE $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tBUILD_RPATH \".\"\n"                                                     \
	"\tOUTPUT_NAME test\n"                                                      \
	"\tPREFIX \"\"\n"                                                           \
	")\n"

#define TARGET_PROPERTIES_S                                                 \
	"set_target_properties(test_s PROPERTIES\n"                         \
	"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tBUILD_RPATH \".\"\n"                                             \
	"\tOUTPUT_NAME test\n"                                              \
	"\tPREFIX \"\"\n"                                                   \
	")\n"

#define TARGET_PROPERTIES_S_DEBUG                                                 \
	"set_target_properties(test_s PROPERTIES\n"                               \
	"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tARCHIVE_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tLIBRARY_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tRUNTIME_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tBUILD_RPATH \".\"\n"                                                   \
	"\tOUTPUT_NAME test\n"                                                    \
	"\tPREFIX \"\"\n"                                                         \
	")\n"

#define TARGET_PROPERTIES_D                                                 \
	"set_target_properties(test_d PROPERTIES\n"                         \
	"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tBUILD_RPATH \".\"\n"                                             \
	"\tOUTPUT_NAME test\n"                                              \
	"\tPREFIX \"\"\n"                                                   \
	")\n"

#define TARGET_PROPERTIES_D_DEBUG                                                 \
	"set_target_properties(test_d PROPERTIES\n"                               \
	"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"       \
	"\tARCHIVE_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tLIBRARY_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tRUNTIME_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n" \
	"\tBUILD_RPATH \".\"\n"                                                   \
	"\tOUTPUT_NAME test\n"                                                    \
	"\tPREFIX \"\"\n"                                                         \
	")\n"

TEST(t_pgc_gen_cm_empty)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_empty(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	char buf[128] = { 0 };
	EXPECT_EQ(pgc_gen_cm(NULL, NULL), NULL);
	EXPECT_EQ(pgc_gen_cm(&pgc, &cmake), &cmake);
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_args)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_args(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_cwd)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cwd(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_require)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_require(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_lib_empty)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_lib_empty(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test_s)\n"
			"" TARGET_PROPERTIES_S ""
			"add_custom_target(test_d)\n"
			"" TARGET_PROPERTIES_D "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_headers)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_headers(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.h)\n"
			"\n");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_includes)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_includes(&pgc);

	pgc.builds = F_PGC_BUILD_EXE;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"target_include_directories(test PRIVATE src/ include/)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_flags)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_flags(&pgc);

	pgc.builds = F_PGC_BUILD_EXE;

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"target_compile_options(test PRIVATE\n"
			"\t$<$<COMPILE_LANGUAGE:C>:-Wall -Wextra>\n"
			")\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_ldflags)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_ldflags(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"target_link_options(test PRIVATE -lm -lpthread)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_libs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_libs(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"target_link_directories(test PRIVATE libs/ libs/ libs/ libs/ libs/)\n"
			"target_link_libraries(test PRIVATE lib_s -l:lib.a -l:lib.so -l:lib.so)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_depends)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_depends(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"add_dependencies(test dep)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_coverage_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_coverage_exe(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_coverage_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_coverage_static(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test_s STATIC ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_S "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_coverage_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_coverage_shared(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test_d SHARED ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_D "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_defines_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_defines_exe(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"target_compile_definitions(test PRIVATE -DDEBUG -DVERSION=1)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_defines_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_defines_static(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test_s STATIC ${test_SOURCE})\n"
			"target_compile_definitions(test_s PRIVATE -DDEBUG -DVERSION=1)\n"
			"" TARGET_PROPERTIES_S "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_defines_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_defines_shared(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test_d SHARED ${test_SOURCE})\n"
			"target_compile_definitions(test_d PRIVATE -DDEBUG -DVERSION=1)\n"
			"" TARGET_PROPERTIES_D "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_copyfiles)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_copyfiles(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test_s)\n"
			"add_custom_command(TARGET test_s PRE_BUILD\n"
			"\tCOMMAND ${CMAKE_COMMAND} -E copy_if_different lib.so ${CMAKE_CURRENT_SOURCE_DIR}\n"
			")\n");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_run)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_run(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_RELEASE "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_run_debug)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_run_debug(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_run_run_debug)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_run_run_debug(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_artifact_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_artifact_exe(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_artifact_lib)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_artifact_lib(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test_s STATIC ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_S "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_bin_obj)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_bin_obj(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_bin_files)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_bin_files(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_bin_run)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_bin_run(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_elf)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_elf(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test STATIC ${test_SOURCE})\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_fat12)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_fat12(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_fat12_header)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_fat12_header(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(test)\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_archs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_archs(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_configs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_configs(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"set_target_properties(test PROPERTIES\n"
			"\tARCHIVE_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tLIBRARY_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tRUNTIME_OUTPUT_DIRECTORY $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tARCHIVE_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tLIBRARY_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tRUNTIME_OUTPUT_DIRECTORY_DEBUG $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tARCHIVE_OUTPUT_DIRECTORY_RELEASE $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tLIBRARY_OUTPUT_DIRECTORY_RELEASE $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tRUNTIME_OUTPUT_DIRECTORY_RELEASE $(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/\n"
			"\tBUILD_RPATH \".\"\n"
			"\tOUTPUT_NAME test\n"
			"\tPREFIX \"\"\n"
			")\n");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_nasm_bin)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_bin(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.nasm)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_nasm_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_exe(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.nasm)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_nasm_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_static(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.nasm)\n"
			"\n"
			"add_library(test_s STATIC ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_S "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_nasm_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_nasm_shared(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.nasm)\n"
			"\n"
			"add_library(test_d SHARED ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_D "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_asm_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_asm_exe(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.S)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_asm_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_asm_static(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.S)\n"
			"\n"
			"add_library(test_s STATIC ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_S "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_asm_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_asm_shared(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.S)\n"
			"\n"
			"add_library(test_d SHARED ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_D "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_c_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_exe(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_c_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_static(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test_s STATIC ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_S_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_c_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_shared(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.c)\n"
			"\n"
			"add_library(test_d SHARED ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_D_DEBUG "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_cpp_exe)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cpp_exe(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.cpp)\n"
			"\n"
			"add_executable(test ${test_SOURCE})\n"
			"" TARGET_PROPERTIES "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_cpp_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cpp_static(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.cpp)\n"
			"\n"
			"add_library(test_s STATIC ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_S "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_cpp_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cpp_shared(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "file(GLOB_RECURSE test_SOURCE src/*.cpp)\n"
			"\n"
			"add_library(test_d SHARED ${test_SOURCE})\n"
			"" TARGET_PROPERTIES_D "");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_cm_url)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_url(&pgc);

	cmake_t cmake = { 0 };
	cmake_init(&cmake, 8, 8, 8);

	pgc_gen_cm(&pgc, &cmake);

	char buf[2048] = { 0 };
	cmake_print(&cmake, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "add_custom_target(gcc-13.1.0)\n");

	cmake_free(&cmake);
	pgc_free(&pgc);

	END;
}

STEST(t_pgc_gen_cm)
{
	SSTART;
	RUN(t_pgc_gen_cm_empty);
	RUN(t_pgc_gen_cm_args);
	RUN(t_pgc_gen_cm_cwd);
	RUN(t_pgc_gen_cm_require);
	RUN(t_pgc_gen_cm_lib_empty);
	RUN(t_pgc_gen_cm_headers);
	RUN(t_pgc_gen_cm_includes);
	RUN(t_pgc_gen_cm_flags);
	RUN(t_pgc_gen_cm_ldflags);
	RUN(t_pgc_gen_cm_libs);
	RUN(t_pgc_gen_cm_depends);
	RUN(t_pgc_gen_cm_coverage_exe);
	RUN(t_pgc_gen_cm_coverage_static);
	RUN(t_pgc_gen_cm_coverage_shared);
	RUN(t_pgc_gen_cm_defines_exe);
	RUN(t_pgc_gen_cm_defines_static);
	RUN(t_pgc_gen_cm_defines_shared);
	RUN(t_pgc_gen_cm_copyfiles);
	RUN(t_pgc_gen_cm_run);
	RUN(t_pgc_gen_cm_run_debug);
	RUN(t_pgc_gen_cm_run_run_debug);
	RUN(t_pgc_gen_cm_artifact_exe);
	RUN(t_pgc_gen_cm_artifact_lib);
	RUN(t_pgc_gen_cm_bin_obj);
	RUN(t_pgc_gen_cm_bin_files);
	RUN(t_pgc_gen_cm_bin_run);
	RUN(t_pgc_gen_cm_elf);
	RUN(t_pgc_gen_cm_fat12);
	RUN(t_pgc_gen_cm_fat12_header);
	RUN(t_pgc_gen_cm_archs);
	RUN(t_pgc_gen_cm_configs);
	RUN(t_pgc_gen_cm_nasm_bin);
	RUN(t_pgc_gen_cm_nasm_exe);
	RUN(t_pgc_gen_cm_nasm_static);
	RUN(t_pgc_gen_cm_nasm_shared);
	RUN(t_pgc_gen_cm_asm_exe);
	RUN(t_pgc_gen_cm_asm_static);
	RUN(t_pgc_gen_cm_asm_shared);
	RUN(t_pgc_gen_cm_c_exe);
	RUN(t_pgc_gen_cm_c_static);
	RUN(t_pgc_gen_cm_c_shared);
	RUN(t_pgc_gen_cm_cpp_exe);
	RUN(t_pgc_gen_cm_cpp_static);
	RUN(t_pgc_gen_cm_cpp_shared);
	RUN(t_pgc_gen_cm_url);
	SEND;
}
