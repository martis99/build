#include "gen/vc/pgc_gen_vc_tasks.h"

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

TEST(t_pgc_gen_vc_tasks_empty)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_empty(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t tasks = json_add_val(&json, JSON_END, STR("tasks"), JSON_ARR());

	char buf[1024] = { 0 };
	EXPECT_EQ(pgc_gen_vc_tasks(NULL, NULL, JSON_END), NULL);
	EXPECT_EQ(pgc_gen_vc_tasks(&pgc, &json, tasks), &json);
	json_print(&json, tasks, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_tasks_archs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_archs(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t tasks = json_add_val(&json, JSON_END, STR("tasks"), JSON_ARR());

	pgc_gen_vc_tasks(&pgc, &json, tasks);

	char buf[2048] = { 0 };
	json_print(&json, tasks, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"label\": \"compile-test-x86_64-Debug\",\n"
			"        \"type\": \"shell\",\n"
			"        \"command\": \"make\",\n"
			"        \"args\": [\n"
			"            \"test/compile\",\n"
			"            \"ARCH=x86_64\",\n"
			"            \"CONFIG=Debug\"\n"
			"        ],\n"
			"" TASK_RUN ""
			"        \"group\": {\n"
			"            \"kind\": \"build\",\n"
			"            \"isDefault\": true\n"
			"        }\n"
			"    },\n"
			"    {\n"
			"        \"label\": \"compile-test-i386-Debug\",\n"
			"        \"type\": \"shell\",\n"
			"        \"command\": \"make\",\n"
			"        \"args\": [\n"
			"            \"test/compile\",\n"
			"            \"ARCH=i386\",\n"
			"            \"CONFIG=Debug\"\n"
			"        ],\n"
			"" TASK_RUN ""
			"        \"group\": {\n"
			"            \"kind\": \"build\",\n"
			"            \"isDefault\": true\n"
			"        }\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_tasks_configs)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_configs(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t tasks = json_add_val(&json, JSON_END, STR("tasks"), JSON_ARR());

	pgc_gen_vc_tasks(&pgc, &json, tasks);

	char buf[2048] = { 0 };
	json_print(&json, tasks, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"label\": \"compile-test-x86_64-Debug\",\n"
			"        \"type\": \"shell\",\n"
			"        \"command\": \"make\",\n"
			"        \"args\": [\n"
			"            \"test/compile\",\n"
			"            \"ARCH=x86_64\",\n"
			"            \"CONFIG=Debug\"\n"
			"        ],\n"
			"" TASK_RUN ""
			"        \"group\": {\n"
			"            \"kind\": \"build\",\n"
			"            \"isDefault\": true\n"
			"        }\n"
			"    },\n"
			"    {\n"
			"        \"label\": \"compile-test-x86_64-Release\",\n"
			"        \"type\": \"shell\",\n"
			"        \"command\": \"make\",\n"
			"        \"args\": [\n"
			"            \"test/compile\",\n"
			"            \"ARCH=x86_64\",\n"
			"            \"CONFIG=Release\"\n"
			"        ],\n"
			"" TASK_RUN ""
			"        \"group\": {\n"
			"            \"kind\": \"build\",\n"
			"            \"isDefault\": true\n"
			"        }\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_tasks_c_static)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_static(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t tasks = json_add_val(&json, JSON_END, STR("tasks"), JSON_ARR());

	pgc_gen_vc_tasks(&pgc, &json, tasks);

	char buf[1024] = { 0 };
	json_print(&json, tasks, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"label\": \"static-test-x86_64-Debug\",\n"
			"        \"type\": \"shell\",\n"
			"        \"command\": \"make\",\n"
			"        \"args\": [\n"
			"            \"test/static\",\n"
			"            \"ARCH=x86_64\",\n"
			"            \"CONFIG=Debug\"\n"
			"        ],\n"
			"        \"group\": {\n"
			"            \"kind\": \"build\",\n"
			"            \"isDefault\": true\n"
			"        }\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

TEST(t_pgc_gen_vc_tasks_c_shared)
{
	START;

	pgc_t pgc = { 0 };
	pgc_gen_c_shared(&pgc);

	json_t json = { 0 };
	json_init(&json, 1);
	const json_val_t tasks = json_add_val(&json, JSON_END, STR("tasks"), JSON_ARR());

	pgc_gen_vc_tasks(&pgc, &json, tasks);

	char buf[1024] = { 0 };
	json_print(&json, tasks, PRINT_DST_BUF(buf, sizeof(buf), 0), "    ");
	EXPECT_STR(buf, "[\n"
			"    {\n"
			"        \"label\": \"shared-test-x86_64-Debug\",\n"
			"        \"type\": \"shell\",\n"
			"        \"command\": \"make\",\n"
			"        \"args\": [\n"
			"            \"test/shared\",\n"
			"            \"ARCH=x86_64\",\n"
			"            \"CONFIG=Debug\"\n"
			"        ],\n"
			"        \"group\": {\n"
			"            \"kind\": \"build\",\n"
			"            \"isDefault\": true\n"
			"        }\n"
			"    }\n"
			"]");

	json_free(&json);
	pgc_free(&pgc);

	END;
}

STEST(t_pgc_gen_vc_tasks)
{
	SSTART;
	RUN(t_pgc_gen_vc_tasks_empty);
	RUN(t_pgc_gen_vc_tasks_archs);
	RUN(t_pgc_gen_vc_tasks_configs);
	RUN(t_pgc_gen_vc_tasks_c_static);
	RUN(t_pgc_gen_vc_tasks_c_shared);
	SEND;
}
