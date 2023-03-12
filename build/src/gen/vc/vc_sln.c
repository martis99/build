#include "vc_sln.h"

#include "gen/mk/mk_sln.h"
#include "gen/proj.h"
#include "gen/var.h"

#include "common.h"

#include <errno.h>

static const var_pol_t vars = {
	.names = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "${workspaceFolder}",
		[VAR_CONFIG] = "Debug",
		[VAR_PLATFORM] = "x64",
	},
};

static const var_pol_t vars2 = {
	.names = {
		[VAR_SLN_DIR] = "$(SolutionDir)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "${workspaceFolder}/",
		[VAR_CONFIG] = "Debug",
		[VAR_PLATFORM] = "x64",
	},
};

int vc_sln_gen(const sln_t *sln, const path_t *path)
{
	mk_sln_gen(sln, path);

	char buf[P_MAX_PATH]  = { 0 };
	char buf2[P_MAX_PATH] = { 0 };

	unsigned int buf_len;
	unsigned int buf2_len;

	MSG("%s", "generating tasks");

	path_t tasks_path = *path;
	if (path_child(&tasks_path, ".vscode", 7)) {
		return 1;
	}

	folder_create(tasks_path.path);

	path_t launch_path = tasks_path;

	if (path_child(&tasks_path, "tasks.json", 11)) {
		return 1;
	}

	int ret = 0;

	FILE *fp = file_open(tasks_path.path, "w", 1);
	if (fp == NULL) {
		printf("Failed to create file: %s, errno: %d\n", tasks_path.path, errno);
		return 1;
	}

	p_fprintf(fp, "{\n"
		      "        \"version\": \"2.0.0\",\n"
		      "        \"tasks\": [\n"
		      "                {\n"
		      "                        \"label\": \"Build\",\n"
		      "                        \"type\": \"shell\",\n"
		      "                        \"command\": \"make\",\n"
		      "                        \"args\": [],\n"
		      "                        \"group\": {\n"
		      "                                \"kind\": \"build\",\n"
		      "                                \"isDefault\": true\n"
		      "                        }\n"
		      "                }\n"
		      "        ]\n"
		      "}");

	fclose(fp);
	if (ret == 0) {
		SUC("generating tasks: %s success", tasks_path.path);
	} else {
		ERR("generating tasks: %s failed", tasks_path.path);
	}

	const prop_t *outdir  = &sln->props[SLN_PROP_OUTDIR];
	const prop_t *startup = &sln->props[SLN_PROP_STARTUP];

	if (!startup->set || !outdir->set) {
		return 0;
	}

	MSG("%s", "generating launch");

	ret = 0;

	if (path_child(&launch_path, "launch.json", 11)) {
		return 1;
	}

	fp = file_open(launch_path.path, "w", 1);
	if (fp == NULL) {
		printf("Failed to create file: %s, errno: %d\n", launch_path.path, errno);
		return 1;
	}

	buf_len	 = convert_slash(buf, sizeof(buf) - 1, outdir->value.data, outdir->value.len);
	buf2_len = cstr_replaces(buf, buf_len, buf2, sizeof(buf2) - 1, vars.names, vars.tos, __VAR_MAX);
	buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, startup->value.data, startup->value.len);

	const prop_str_t *cwd;

	const proj_t *startup_proj = NULL;
	if (hashmap_get(&sln->projects, startup->value.data, startup->value.len, (void **)&startup_proj)) {
		ERR("project doesn't exists: '%.*s'", startup->value.len, startup->value.data);
		return 1;
	}

	const prop_t *wdir = &startup_proj->props[PROJ_PROP_WDIR];
	const prop_t *args = &startup_proj->props[PROJ_PROP_ARGS];

	cwd = wdir->set ? &wdir->value : &startup->value;

	p_fprintf(fp,
		  "{\n"
		  "        \"version\": \"0.2.0\",\n"
		  "        \"configurations\": [\n"
		  "                {\n"
		  "                        \"name\": \"Launch\",\n"
		  "                        \"type\": \"cppdbg\",\n"
		  "                        \"request\": \"launch\",\n"
		  "                        \"program\": \"%.*s%.*s\",\n"
		  "                        \"args\": [",
		  buf_len, buf, startup->value.len, startup->value.data);

	if (args->set) {
		p_fprintf(fp, "\n");
		int end = 0;

		str_t arg = {
			.data = args->value.data,
			.len  = args->value.len,
		};
		str_t next = { 0 };

		while (!end) {
			if (str_chr(&arg, &arg, &next, ' ')) {
				str_chr(&arg, &arg, &next, '\n');
				end = 1;
			}

			buf_len	 = convert_slash(buf, sizeof(buf) - 1, arg.data, arg.len);
			buf2_len = cstr_replaces(buf, buf_len, buf2, sizeof(buf2) - 1, vars2.names, vars2.tos, __VAR_MAX);
			buf_len	 = cstr_replace(buf2, buf2_len, buf, sizeof(buf) - 1, "$(PROJ_NAME)", 12, startup->value.data, startup->value.len);

			p_fprintf(fp, "                                \"%.*s\"%.*s\n", buf_len, buf, !end, ",");

			arg = next;
		}

		p_fprintf(fp, "                        ");
	}

	p_fprintf(fp,
		  "],\n"
		  "                        \"preLaunchTask\": \"Build\",\n"
		  "                        \"stopAtEntry\": false,\n"
		  "                        \"cwd\": \"${workspaceFolder}/%.*s\",\n"
		  "                        \"environment\": [],\n"
		  "                        \"externalConsole\": false,\n"
		  "                        \"MIMode\": \"gdb\",\n"
		  "                        \"setupCommands\": [\n"
		  "                                {\n"
		  "                                        \"description\": \"Enable pretty-printing for gdb\",\n"
		  "                                        \"text\": \"-enable-pretty-printing\",\n"
		  "                                        \"ignoreFailures\": true\n"
		  "                                },\n"
		  "                                {\n"
		  "                                        \"description\": \"Set Disassembly Flavor to Intel\",\n"
		  "                                        \"text\": \"-gdb-set disassembly-flavor intel\",\n"
		  "                                        \"ignoreFailures\": true\n"
		  "                                }\n"
		  "                        ]\n"
		  "                }\n"
		  "        ]\n"
		  "}",
		  cwd->len, cwd->data);

	fclose(fp);
	if (ret == 0) {
		SUC("generating tasks: %s success", launch_path.path);
	} else {
		ERR("generating tasks: %s failed", launch_path.path);
	}

	return 0;
}
