#include "vc_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

static const var_pol_t vars = {
	.names = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "${workspaceFolder}",
		[VAR_PLATFORM] = "x64",
	},
};

static const var_pol_t vars2 = {
	.names = {
		[VAR_SLN_DIR] = "$(SolutionDir)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "${workspaceFolder}/",
		[VAR_PLATFORM] = "x64",
	},
};

static size_t resolve(const prop_str_t *prop, char *dst, size_t dst_max_len, const proj_t *proj, const var_pol_t *vars)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len, dst_len;

	buf_len = convert_slash(CSTR(buf), prop->data, prop->len);
	dst_len = cstr_replaces(buf, buf_len, dst, dst_max_len, vars->names, vars->tos, __VAR_MAX);
	buf_len = cstr_replace(dst, dst_len, CSTR(buf), "$(PROJ_NAME)", 12, proj->name->data, proj->name->len);
	dst_len = cstr_replace(buf, buf_len, dst, dst_max_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len);

	return dst_len;
}

int vc_proj_gen_build(const proj_t *proj, const prop_t *sln_props, FILE *f)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *configs = &sln_props[SLN_PROP_CONFIGS];

	if (!(configs->flags & PROP_SET)) {
		p_fprintf(f,
			  "                {\n"
			  "                        \"label\": \"Build-%.*s\",\n"
			  "                        \"type\": \"shell\",\n"
			  "                        \"command\": \"make\",\n"
			  "                        \"args\": [\n"
			  "                                \"clean\",\n"
			  "                                \"%.*s\"\n"
			  "                        ],\n"
			  "                        \"group\": {\n"
			  "                                \"kind\": \"build\",\n"
			  "                                \"isDefault\": true\n"
			  "                        }\n"
			  "                }",
			  name->len, name->data, name->len, name->data);
	} else {
		for (int i = 0; i < configs->arr.count; i++) {
			const prop_str_t *config = array_get(&configs->arr, i);

			p_fprintf(f,
				  "                {\n"
				  "                        \"label\": \"Build-%.*s-%.*s\",\n"
				  "                        \"type\": \"shell\",\n"
				  "                        \"command\": \"make\",\n"
				  "                        \"args\": [\n"
				  "                                \"clean\",\n"
				  "                                \"%.*s\",\n"
				  "                                \"CONFIG=%.*s\"\n"
				  "                        ],\n"
				  "                        \"group\": {\n"
				  "                                \"kind\": \"build\",\n"
				  "                                \"isDefault\": true\n"
				  "                        }\n"
				  "                }",
				  name->len, name->data, config->len, config->data, name->len, name->data, config->len, config->data);

			if (i < configs->arr.count - 1) {
				p_fprintf(f, ",\n");
			}
		}
	}

	return 0;
}

int vc_proj_gen_launch(const proj_t *proj, const hashmap_t *projects, const prop_t *sln_props, FILE *f)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	if (type != PROJ_TYPE_EXE) {
		return 0;
	}

	const prop_t *configs = &sln_props[SLN_PROP_CONFIGS];
	const prop_t *outdir  = &proj->props[PROJ_PROP_OUTDIR];

	char buf[P_MAX_PATH]	    = { 0 };
	char outdir_buf[P_MAX_PATH] = { 0 };

	size_t buf_len;
	size_t outdir_buf_len;

	outdir_buf_len = resolve(&outdir->value, CSTR(outdir_buf), proj, &vars);

	const prop_str_t *cwd;

	const prop_t *wdir = &proj->props[PROJ_PROP_WDIR];
	const prop_t *args = &proj->props[PROJ_PROP_ARGS];

	prop_str_t rel_path = {
		.cdata = proj->rel_path.path,
		.len   = proj->rel_path.len,
	};

	cwd = (wdir->flags & PROP_SET) ? &wdir->value : &rel_path;

	if (!(configs->flags & PROP_SET)) {
		buf_len = cstr_replace(outdir_buf, outdir_buf_len, buf, sizeof(buf) - 1, "$(CONFIG)", 9, "Debug", 5);

		p_fprintf(f,
			  "                {\n"
			  "                        \"name\": \"Launch\",\n"
			  "                        \"type\": \"cppdbg\",\n"
			  "                        \"request\": \"launch\",\n"
			  "                        \"program\": \"%.*s%.*s\",\n"
			  "                        \"args\": [",
			  buf_len, buf, name->len, name->data);

		if (args->flags & PROP_SET) {
			p_fprintf(f, "\n");
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

				const prop_str_t argp = {
					.cdata = arg.data,
					.len  = arg.len,
				};

				buf_len = resolve(&argp, CSTR(buf), proj, &vars2);

				p_fprintf(f, "                                \"%.*s\"%.*s\n", buf_len, buf, !end, ",");

				arg = next;
			}

			p_fprintf(f, "                        ");
		}

		p_fprintf(f,
			  "],\n"
			  "                        \"preLaunchTask\": \"Build-%.*s\",\n"
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
			  "                }",
			  name->len, name->data, cwd->len, cwd->data);
	} else {
		for (int i = 0; i < configs->arr.count; i++) {
			const prop_str_t *config = array_get(&configs->arr, i);

			buf_len = cstr_replace(outdir_buf, outdir_buf_len, buf, sizeof(buf) - 1, "$(CONFIG)", 9, config->data, config->len);

			p_fprintf(f,
				  "                {\n"
				  "                        \"name\": \"%.*s-%.*s\",\n"
				  "                        \"type\": \"cppdbg\",\n"
				  "                        \"request\": \"launch\",\n"
				  "                        \"program\": \"%.*s%.*s\",\n"
				  "                        \"args\": [",
				  name->len, name->data, config->len, config->data, buf_len, buf, name->len, name->data);

			if (args->flags & PROP_SET) {
				p_fprintf(f, "\n");
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

					const prop_str_t argp = {
						.cdata = arg.data,
						.len  = arg.len,
					};

					buf_len = resolve(&argp, CSTR(buf), proj, &vars2);

					p_fprintf(f, "                                \"%.*s\"%.*s\n", buf_len, buf, !end, ",");

					arg = next;
				}

				p_fprintf(f, "                        ");
			}

			p_fprintf(f,
				  "],\n"
				  "                        \"preLaunchTask\": \"Build-%.*s-%.*s\",\n"
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
				  "                }",
				  name->len, name->data, config->len, config->data, cwd->len, cwd->data);

			if (i < configs->arr.count - 1) {
				p_fprintf(f, ",\n");
			}
		}
	}

	return 0;
}
