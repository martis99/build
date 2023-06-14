#include "t_cm.h"

#include "gen/cm/cm_sln.h"

#include "common.h"
#include "test.h"

TEST(c_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/CMakeLists.txt",
			.data = "cmake_minimum_required(VERSION 3.16)\n"
				"\n"
				"project(\"test\" LANGUAGES C)\n"
				"\n"
				"set(CMAKE_CONFIGURATION_TYPES \"Debug\" CACHE STRING \"\" FORCE)\n"
				"\n"
				"set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n"
				"set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)\n"
				"\n"
				"add_subdirectory(test)\n",
		},
		{
			.path = "tmp/test/CMakeLists.txt",
			.data = "file(GLOB_RECURSE test_SOURCE src/*.c src/*.h)\n"
				"\n"
				"add_executable(test ${test_SOURCE})\n"
				"target_link_libraries(test)\n"
				"include_directories(src)\n"
				"set_target_properties(test\n"
				"    PROPERTIES\n"
				"    ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    LIBRARY_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    LIBRARY_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    RUNTIME_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    RUNTIME_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				")\n"
				"set_property(TARGET test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/test\")\n",

		},
	};

	EXPECT_EQ(test_gen(cm_sln_gen, c_small_in, sizeof(c_small_in), out, sizeof(out)), 0);

	END;
}

TEST(cpp_small)
{
	START;

	test_gen_file_t out[] = {
		{
			.path = "tmp/CMakeLists.txt",
			.data = "cmake_minimum_required(VERSION 3.16)\n"
				"\n"
				"project(\"test\" LANGUAGES CXX)\n"
				"\n"
				"set(CMAKE_CONFIGURATION_TYPES \"Debug\" CACHE STRING \"\" FORCE)\n"
				"\n"
				"set_property(GLOBAL PROPERTY USE_FOLDERS ON)\n"
				"set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)\n"
				"\n"
				"add_subdirectory(test)\n",
		},
		{
			.path = "tmp/test/CMakeLists.txt",
			.data = "file(GLOB_RECURSE test_SOURCE src/*.h src/*.cpp src/*.hpp)\n"
				"\n"
				"add_executable(test ${test_SOURCE})\n"
				"target_link_libraries(test)\n"
				"include_directories(src)\n"
				"set_target_properties(test\n"
				"    PROPERTIES\n"
				"    ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    LIBRARY_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    LIBRARY_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    RUNTIME_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				"    RUNTIME_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_SOURCE_DIR}/bin/$(Configuration)-${CMAKE_VS_PLATFORM_NAME}/test/\"\n"
				")\n"
				"set_property(TARGET test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/test\")\n",

		},
	};

	EXPECT_EQ(test_gen(cm_sln_gen, cpp_small_in, sizeof(cpp_small_in), out, sizeof(out)), 0);

	END;
}

STEST(cm)
{
	SSTART;
	RUN(c_small);
	RUN(cpp_small);
	SEND;
}
