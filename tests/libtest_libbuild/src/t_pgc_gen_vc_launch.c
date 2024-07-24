#include "gen/vc/pgc_gen_vc_launch.h"

#include "mem.h"
#include "pgc_gen.h"
#include "test.h"

#define TASK_RUN                                          \
	"        \"isBackground\": true,\n"               \
	"        \"problemMatcher\": [\n"                 \
	"            {\n"                                 \
	"                \"pattern\": [\n"                \
	"                    {\n"                         \
	"                        \"regexp\": \".\",\n"    \
	"                        \"file\": 1,\n"          \
	"                        \"location\": 2,\n"      \
	"                        \"message\": 3\n"        \
	"                    }\n"                         \
	"                ],\n"                            \
	"                \"background\": {\n"             \
	"                    \"activeOnStart\": true,\n"  \
	"                    \"beginsPattern\": \".\",\n" \
	"                    \"endsPattern\": \".\"\n"    \
	"                }\n"                             \
	"            }\n"                                 \
	"        ],\n"

TEST(t_pgc_gen_vc_launch_empty)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_empty(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t confs = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	char buf[1024] = { 0 };
	EXPECT_EQ(pgc_gen_vc_launch(NULL, NULL, JSON_END), NULL);
	EXPECT_EQ(pgc_gen_vc_launch(&pgc, &json, confs), &json);
	json_print(&json, confs, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_launch_args)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_args(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t confs = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	pgc_gen_vc_launch(&pgc, &json, confs);

	char buf[2048] = { 0 };
	json_print(&json, confs, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"name\": \"run-test-x86_64-Debug\",\n"
			"        \"type\": \"cppdbg\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"compile-test-x86_64-Debug\",\n"
			"        \"program\": \"$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/test\",\n"
			"        \"args\": [\n"
			"            \"-D\"\n"
			"        ],\n"
			"        \"stopAtEntry\": false,\n"
			"        \"environment\": [],\n"
			"        \"externalConsole\": false,\n"
			"        \"MIMode\": \"gdb\",\n"
			"        \"setupCommands\": [\n"
			"            {\n"
			"                \"description\": \"Enable pretty-printing for gdb\",\n"
			"                \"text\": \"-enable-pretty-printing\",\n"
			"                \"ignoreFailures\": true\n"
			"            },\n"
			"            {\n"
			"                \"description\": \"Set Disassembly Flavor to Intel\",\n"
			"                \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			"                \"ignoreFailures\": true\n"
			"            }\n"
			"        ]\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_launch_cwd)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_cwd(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t confs = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	pgc_gen_vc_launch(&pgc, &json, confs);

	char buf[2048] = { 0 };
	json_print(&json, confs, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"name\": \"run-test-x86_64-Debug\",\n"
			"        \"type\": \"cppdbg\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"compile-test-x86_64-Debug\",\n"
			"        \"program\": \"$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/test\",\n"
			"        \"args\": [],\n"
			"        \"stopAtEntry\": false,\n"
			"        \"cwd\": \"projects/test\",\n"
			"        \"environment\": [],\n"
			"        \"externalConsole\": false,\n"
			"        \"MIMode\": \"gdb\",\n"
			"        \"setupCommands\": [\n"
			"            {\n"
			"                \"description\": \"Enable pretty-printing for gdb\",\n"
			"                \"text\": \"-enable-pretty-printing\",\n"
			"                \"ignoreFailures\": true\n"
			"            },\n"
			"            {\n"
			"                \"description\": \"Set Disassembly Flavor to Intel\",\n"
			"                \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			"                \"ignoreFailures\": true\n"
			"            }\n"
			"        ]\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_launch_bin_run)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_bin_run(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t confs = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	pgc_gen_vc_launch(&pgc, &json, confs);

	char buf[2048] = { 0 };
	json_print(&json, confs, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"name\": \"run_bin-test-x86_64-Debug\",\n"
			"        \"type\": \"cppdbg\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"bin-test-x86_64-Debug\",\n"
			"        \"program\": \"$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/test\",\n"
			"        \"args\": [],\n"
			"        \"miDebuggerServerAddress\": \"localhost:1234\",\n"
			"        \"stopAtEntry\": false,\n"
			"        \"environment\": [],\n"
			"        \"externalConsole\": false,\n"
			"        \"MIMode\": \"gdb\",\n"
			"        \"setupCommands\": [\n"
			"            {\n"
			"                \"description\": \"Enable pretty-printing for gdb\",\n"
			"                \"text\": \"-enable-pretty-printing\",\n"
			"                \"ignoreFailures\": true\n"
			"            },\n"
			"            {\n"
			"                \"description\": \"Set Disassembly Flavor to Intel\",\n"
			"                \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			"                \"ignoreFailures\": true\n"
			"            }\n"
			"        ]\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_launch_archs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_archs(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t confs = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	pgc_gen_vc_launch(&pgc, &json, confs);

	char buf[2048] = { 0 };
	json_print(&json, confs, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"name\": \"run-test-x86_64-Debug\",\n"
			"        \"type\": \"cppdbg\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"compile-test-x86_64-Debug\",\n"
			"        \"program\": \"$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/test\",\n"
			"        \"args\": [],\n"
			"        \"stopAtEntry\": false,\n"
			"        \"environment\": [],\n"
			"        \"externalConsole\": false,\n"
			"        \"MIMode\": \"gdb\",\n"
			"        \"setupCommands\": [\n"
			"            {\n"
			"                \"description\": \"Enable pretty-printing for gdb\",\n"
			"                \"text\": \"-enable-pretty-printing\",\n"
			"                \"ignoreFailures\": true\n"
			"            },\n"
			"            {\n"
			"                \"description\": \"Set Disassembly Flavor to Intel\",\n"
			"                \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			"                \"ignoreFailures\": true\n"
			"            }\n"
			"        ]\n"
			"    },\n"
			"    {\n"
			"        \"name\": \"run-test-i386-Debug\",\n"
			"        \"type\": \"cppdbg\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"compile-test-i386-Debug\",\n"
			"        \"program\": \"$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/test\",\n"
			"        \"args\": [],\n"
			"        \"stopAtEntry\": false,\n"
			"        \"environment\": [],\n"
			"        \"externalConsole\": false,\n"
			"        \"MIMode\": \"gdb\",\n"
			"        \"setupCommands\": [\n"
			"            {\n"
			"                \"description\": \"Enable pretty-printing for gdb\",\n"
			"                \"text\": \"-enable-pretty-printing\",\n"
			"                \"ignoreFailures\": true\n"
			"            },\n"
			"            {\n"
			"                \"description\": \"Set Disassembly Flavor to Intel\",\n"
			"                \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			"                \"ignoreFailures\": true\n"
			"            }\n"
			"        ]\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_launch_configs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_configs(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t confs = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	pgc_gen_vc_launch(&pgc, &json, confs);

	char buf[2048] = { 0 };
	json_print(&json, confs, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"name\": \"run-test-x86_64-Debug\",\n"
			"        \"type\": \"cppdbg\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"compile-test-x86_64-Debug\",\n"
			"        \"program\": \"$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/test\",\n"
			"        \"args\": [],\n"
			"        \"stopAtEntry\": false,\n"
			"        \"environment\": [],\n"
			"        \"externalConsole\": false,\n"
			"        \"MIMode\": \"gdb\",\n"
			"        \"setupCommands\": [\n"
			"            {\n"
			"                \"description\": \"Enable pretty-printing for gdb\",\n"
			"                \"text\": \"-enable-pretty-printing\",\n"
			"                \"ignoreFailures\": true\n"
			"            },\n"
			"            {\n"
			"                \"description\": \"Set Disassembly Flavor to Intel\",\n"
			"                \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			"                \"ignoreFailures\": true\n"
			"            }\n"
			"        ]\n"
			"    },\n"
			"    {\n"
			"        \"name\": \"run-test-x86_64-Release\",\n"
			"        \"type\": \"cppdbg\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"compile-test-x86_64-Release\",\n"
			"        \"program\": \"$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/test\",\n"
			"        \"args\": [],\n"
			"        \"stopAtEntry\": false,\n"
			"        \"environment\": [],\n"
			"        \"externalConsole\": false,\n"
			"        \"MIMode\": \"gdb\",\n"
			"        \"setupCommands\": [\n"
			"            {\n"
			"                \"description\": \"Enable pretty-printing for gdb\",\n"
			"                \"text\": \"-enable-pretty-printing\",\n"
			"                \"ignoreFailures\": true\n"
			"            },\n"
			"            {\n"
			"                \"description\": \"Set Disassembly Flavor to Intel\",\n"
			"                \"text\": \"-gdb-set disassembly-flavor intel\",\n"
			"                \"ignoreFailures\": true\n"
			"            }\n"
			"        ]\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_launch_c_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_static(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t tasks = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	pgc_gen_vc_launch(&pgc, &json, tasks);

	char buf[1024] = { 0 };
	json_print(&json, tasks, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"name\": \"run_s-test-x86_64-Debug\",\n"
			"        \"type\": \"f5anything\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"static-test-x86_64-Debug\"\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_launch_c_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_shared(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t tasks = json_add_val(&json, JSON_END, STR("configurations"), JSON_ARR());

	pgc_gen_vc_launch(&pgc, &json, tasks);

	char buf[1024] = { 0 };
	json_print(&json, tasks, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"name\": \"run_d-test-x86_64-Debug\",\n"
			"        \"type\": \"f5anything\",\n"
			"        \"request\": \"launch\",\n"
			"        \"preLaunchTask\": \"shared-test-x86_64-Debug\"\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

STEST(t_pgc_gen_vc_launch)
{
	SSTART;
	RUN(t_pgc_gen_vc_launch_empty);
	RUN(t_pgc_gen_vc_launch_args);
	RUN(t_pgc_gen_vc_launch_cwd);
	RUN(t_pgc_gen_vc_launch_bin_run);
	RUN(t_pgc_gen_vc_launch_archs);
	RUN(t_pgc_gen_vc_launch_configs);
	RUN(t_pgc_gen_vc_launch_c_static);
	RUN(t_pgc_gen_vc_launch_c_shared);
	SEND;
}
