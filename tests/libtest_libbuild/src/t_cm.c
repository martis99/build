#include "t_cm.h"

#include "gen/cm/cm_sln.h"

#include "common.h"
#include "test.h"

static const char *SLN_TEST_C = "cmake_minimum_required(VERSION 3.16)\n"
				"\n"
				"project(\"test\" LANGUAGES C)\n"
				"\n"
				"set(CMAKE_CONFIGURATION_TYPES \"Debug\" CACHE STRING \"\" FORCE)\n"
				"\n"
				"set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n"
				"set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)\n"
				"\n"
				"add_subdirectory(test)\n";

static const char *SLN_LIBTEST_C = "cmake_minimum_required(VERSION 3.16)\n"
				   "\n"
				   "project(\"test\" LANGUAGES C)\n"
				   "\n"
				   "set(CMAKE_CONFIGURATION_TYPES \"Debug\" CACHE STRING \"\" FORCE)\n"
				   "\n"
				   "set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n"
				   "set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)\n"
				   "\n"
				   "add_subdirectory(libtest)\n"
				   "add_subdirectory(test)\n";

static const char *SLN_TEST_CPP = "cmake_minimum_required(VERSION 3.16)\n"
				  "\n"
				  "project(\"test\" LANGUAGES CXX)\n"
				  "\n"
				  "set(CMAKE_CONFIGURATION_TYPES \"Debug\" CACHE STRING \"\" FORCE)\n"
				  "\n"
				  "set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n"
				  "set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)\n"
				  "\n"
				  "add_subdirectory(test)\n";

#define PROJ_PROPS(_name)                                                                                                          \
	"set_target_properties(" _name "\n"                                                                                        \
	"    PROPERTIES\n"                                                                                                         \
	"    FOLDER \"" _name "\"\n"                                                                                               \
	"    ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/" _name "/\"\n"   \
	"    ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/" _name "/\"\n" \
	"    LIBRARY_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/" _name "/\"\n"   \
	"    LIBRARY_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/" _name "/\"\n" \
	"    RUNTIME_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/" _name "/\"\n"   \
	"    RUNTIME_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/" _name "/\"\n" \
	")\n"                                                                                                                      \
	"set_property(TARGET " _name " PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/" _name "\")\n"

TEST(c_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/CMakeLists.txt",
			.data = SLN_TEST_C,
		},
		{
			.path = "tmp/test/CMakeLists.txt",
			.data = {
				"file(GLOB_RECURSE test_SOURCE src/*.c src/*.h)\n"
				"\n"
				"add_executable(test ${test_SOURCE})\n"
				"include_directories(src)\n",
				PROJ_PROPS("test"),
			},
		},
	};

	const int ret = test_gen(cm_sln_gen, NULL, c_small_in, sizeof(c_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_args)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/CMakeLists.txt",
			.data = SLN_TEST_C,
		},
		{
			.path = "tmp/test/CMakeLists.txt",
			.data = {
				"file(GLOB_RECURSE test_SOURCE src/*.c src/*.h)\n"
				"\n"
				"add_executable(test ${test_SOURCE})\n"
				"include_directories(src)\n",
				PROJ_PROPS("test"),
			},
		},
	};

	const int ret = test_gen(cm_sln_gen, NULL, c_small_in, sizeof(c_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_include)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/CMakeLists.txt",
			.data = SLN_TEST_C,
		},
		{
			.path = "tmp/test/CMakeLists.txt",
			.data = {
				"file(GLOB_RECURSE test_SOURCE src/*.c src/*.h include/*.h)\n"
				"\n"
				"add_executable(test ${test_SOURCE})\n"
				"include_directories(src include)\n",
				PROJ_PROPS("test"),
			},
		},
	};

	const int ret = test_gen(cm_sln_gen, NULL, c_include_in, sizeof(c_include_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(c_depends)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/CMakeLists.txt",
			.data = SLN_LIBTEST_C,
		},
		{
			.path = "tmp/libtest/CMakeLists.txt",
			.data = {
				"file(GLOB_RECURSE libtest_SOURCE src/*.c src/*.h)\n"
				"\n"
				"add_library(libtest STATIC ${libtest_SOURCE})\n"
				"include_directories(src)\n",
				PROJ_PROPS("libtest"),
			},
		},
		{
			.path = "tmp/test/CMakeLists.txt",
			.data = {
				"file(GLOB_RECURSE test_SOURCE src/*.c src/*.h)\n"
				"\n"
				"add_executable(test ${test_SOURCE})\n"
				"target_link_libraries(test libtest)\n"
				"include_directories(src)\n",
				PROJ_PROPS("test"),
			},
		},
	};

	const int ret = test_gen(cm_sln_gen, NULL, c_depends_in, sizeof(c_depends_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

TEST(cpp_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/CMakeLists.txt",
			.data = SLN_TEST_CPP,
		},
		{
			.path = "tmp/test/CMakeLists.txt",
			.data = {
				"file(GLOB_RECURSE test_SOURCE src/*.h src/*.cpp src/*.hpp)\n"
				"\n"
				"add_executable(test ${test_SOURCE})\n"
				"include_directories(src)\n",
				PROJ_PROPS("test"),
			},
		},
	};

	const int ret = test_gen(cm_sln_gen, NULL, cpp_small_in, sizeof(cpp_small_in), out, sizeof(out));
	EXPECT_EQ(ret, 0);

	END;
}

STEST(cm)
{
	SSTART;
	RUN(c_small);
	RUN(c_args);
	RUN(c_include);
	RUN(c_depends);
	RUN(cpp_small);
	SEND;
}
