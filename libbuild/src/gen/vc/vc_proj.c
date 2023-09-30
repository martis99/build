#include "vc_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

static const var_pol_t vars = {
	.old = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
	},
	.new = {
		[VAR_SLN_DIR] = "${workspaceFolder}",
	},
};

static size_t resolve(const prop_str_t *prop, char *buf, size_t buf_size, const proj_t *proj, const prop_str_t *config, const prop_str_t *platform, const char *outdir,
		      size_t outdir_len)
{
	size_t buf_len = prop->len;
	mem_cpy(buf, buf_size, prop->data, prop->len);

	buf_len = invert_slash(buf, buf_len);
	buf_len = cstr_replaces(buf, buf_size, buf_len, vars.old, vars.new, __VAR_MAX, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_NAME)"), proj->name->data, proj->name->len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(CONFIG)"), config ? config->data : "", config ? config->len : 0, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PLATFORM)"), platform ? platform->data : "", platform ? platform->len : 0, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(BIN)"), CSTR("$(BIN_PATH)$(BIN_FILE)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(BIN_PATH)"), outdir, outdir_len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(BIN_FILE)"), proj->name->data, proj->name->len, NULL);

	return buf_len;
}

#define NAME_PATTERN		 "%.*s%s%.*s%s%.*s"
#define NAME_VAL(_val)		 _val ? "-" : "", _val ? ((const prop_str_t *)_val)->len : 0, _val ? ((const prop_str_t *)_val)->data : ""
#define NAME(_config, _platform) name->len, name->data, NAME_VAL(_config), NAME_VAL(_platform)

static int add_task(const proj_t *proj, const prop_t *sln_props, const prop_str_t *config, const prop_str_t *platform, const char *prefix, const char *action, FILE *f)
{
	const prop_str_t *name = proj->name;

	const prop_t *run = &proj->props[PROJ_PROP_RUN];

	c_fprintf(f,
		  "                {\n"
		  "                        \"label\": \"%s-" NAME_PATTERN "\",\n"
		  "                        \"type\": \"shell\",\n"
		  "                        \"command\": \"make\",\n"
		  "                        \"args\": [\n"
		  "                                \"clean\",\n"
		  "                                \"%.*s/%s\"",
		  prefix, NAME(config, platform), name->len, name->data, action);

	if (config) {
		c_fprintf(f,
			  ",\n"
			  "                                \"CONFIG=%.*s\"",
			  config->len, config->data);
	}

	if (platform) {
		c_fprintf(f,
			  ",\n"
			  "                                \"PLATFORM=%.*s\"",
			  platform->len, platform->data);
	}

	c_fprintf(f, "\n"
		     "                        ]");

	if (run->flags & PROP_SET) {
		c_fprintf(f, ",\n"
			     "                        \"isBackground\": true,\n"
			     "                        \"problemMatcher\": [\n"
			     "                                {\n"
			     "                                        \"pattern\": [\n"
			     "                                        {\n"
			     "                                                \"regexp\": \".\",\n"
			     "                                                \"file\": 1,\n"
			     "                                                \"location\": 2,\n"
			     "                                                \"message\": 3\n"
			     "                                        }\n"
			     "                                        ],\n"
			     "                                        \"background\": {\n"
			     "                                                \"activeOnStart\": true,\n"
			     "                                                \"beginsPattern\": \".\",\n"
			     "                                                \"endsPattern\": \".\",\n"
			     "                                        }\n"
			     "                                }\n"
			     "                        ]");
	}

	c_fprintf(f, ",\n"
		     "                        \"group\": {\n"
		     "                                \"kind\": \"build\",\n"
		     "                                \"isDefault\": true\n"
		     "                        }\n"
		     "                }");

	return 0;
}

static int add_tasks(const proj_t *proj, const prop_t *sln_props, const char *prefix, const char *action, FILE *f)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *platforms = &sln_props[SLN_PROP_PLATFORMS];

	if (!(configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		add_task(proj, sln_props, NULL, NULL, prefix, action, f);
	} else if ((configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			add_task(proj, sln_props, config, NULL, prefix, action, f);

			if (i < configs->arr.cnt - 1) {
				c_fprintf(f, ",\n");
			}
		}
	} else if (!(configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < platforms->arr.cnt; i++) {
			const prop_str_t *platform = arr_get(&platforms->arr, i);

			add_task(proj, sln_props, NULL, platform, prefix, action, f);

			if (i < platforms->arr.cnt - 1) {
				c_fprintf(f, ",\n");
			}
		}
	} else if ((configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			for (uint j = 0; j < platforms->arr.cnt; j++) {
				const prop_str_t *platform = arr_get(&platforms->arr, j);

				add_task(proj, sln_props, config, platform, prefix, action, f);

				if (i < configs->arr.cnt - 1 || j < platforms->arr.cnt - 1) {
					c_fprintf(f, ",\n");
				}
			}
		}
	}

	return 0;
}

int vc_proj_gen_build(const proj_t *proj, const prop_t *sln_props, FILE *f)
{
	add_tasks(proj, sln_props, "Build", "compile", f);

	if (proj_runnable(proj)) {
		c_fprintf(f, ",\n");
		add_tasks(proj, sln_props, "Run", "run", f);
	}

	return 0;
}

static int cppdbg(proj_t *proj, const dict_t *projects, const prop_str_t *config, const prop_str_t *platform, FILE *f)
{
	const prop_str_t *name = proj->name;

	const proj_type_t type = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *run     = &proj->props[PROJ_PROP_RUN];
	const prop_t *wdir    = &proj->props[PROJ_PROP_WDIR];
	const prop_t *args    = &proj->props[PROJ_PROP_ARGS];
	const prop_t *program = &proj->props[PROJ_PROP_PROGRAM];

	prop_str_t rel_path = {
		.cdata = proj->rel_path.path,
		.len   = proj->rel_path.len,
	};

	const prop_str_t *outdir = &proj->props[PROJ_PROP_OUTDIR].value;
	const prop_str_t *cwd	 = (wdir->flags & PROP_SET) ? &wdir->value : &rel_path;

	char out[P_MAX_PATH] = { 0 };
	char buf[P_MAX_PATH] = { 0 };

	size_t out_len;
	size_t buf_len;

	out_len = resolve(outdir, CSTR(out), proj, config, platform, "", 0);

	c_fprintf(f,
		  "                {\n"
		  "                        \"name\": \"" NAME_PATTERN "\",\n"
		  "                        \"type\": \"cppdbg\",\n"
		  "                        \"request\": \"launch\",\n",
		  NAME(config, platform));

	proj_t *pproj = proj;

	if (program->flags & PROP_SET) {
		const prop_str_t *pname = &program->value;
		if (dict_get(projects, pname->data, pname->len, (void **)&pproj)) {
			ERR("project doesn't exists: '%.*s'", (int)pname->len, pname->data);
			return 1;
		}
	}

	make_ext_set_val(&pproj->make, STR("SLNDIR"), MSTR(STR("${workspaceFolder}")));
	make_ext_set_val(&pproj->make, STR("CONFIG"), MSTR(strc(config->data, config->len)));
	make_ext_set_val(&pproj->make, STR("PLATFORM"), MSTR(strc(platform->data, platform->len)));
	make_expand(&pproj->make);
	str_t mtarget = make_var_get_resolved(&pproj->make, STR("TARGET"));
	c_fprintf(f, "                        \"program\": \"%.*s\",\n", mtarget.len, mtarget.data);

	c_fprintf(f, "                        \"args\": [");

	if (args->flags & PROP_SET) {
		c_fprintf(f, "\n");
		int end = 0;

		str_t arg = {
			.data = args->value.data,
			.len  = args->value.len,
		};
		str_t next = { 0 };

		while (!end) {
			if (str_chr(arg, &arg, &next, ' ')) {
				str_chr(arg, &arg, &next, '\n');
				end = 1;
			}

			const prop_str_t argp = {
				.cdata = arg.data,
				.len   = arg.len,
			};

			buf_len = resolve(&argp, CSTR(buf), proj, config, platform, out, out_len);

			c_fprintf(f, "                                \"%.*s\"%.*s\n", buf_len, buf, !end, ",");

			arg = next;
		}

		c_fprintf(f, "                        ");
	}

	c_fprintf(f, "],\n");

	if (run->flags & PROP_SET) {
		c_fprintf(f, "                        \"miDebuggerServerAddress\": \"localhost:1234\",\n");
	}

	c_fprintf(f,
		  "                        \"preLaunchTask\": \"%s-" NAME_PATTERN "\",\n"
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
		  run->flags & PROP_SET ? "Run" : "Build", NAME(config, platform), cwd->len, cwd->data);

	return 0;
}

static int f5anything(const proj_t *proj, const prop_str_t *config, const prop_str_t *platform, FILE *f)
{
	const prop_str_t *name = proj->name;

	const prop_t *run = &proj->props[PROJ_PROP_RUN];

	c_fprintf(f,
		  "                {\n"
		  "                        \"name\": \"" NAME_PATTERN "\",\n"
		  "                        \"type\": \"f5anything\",\n"
		  "                        \"request\": \"launch\",\n"
		  "                        \"preLaunchTask\": \"%s-" NAME_PATTERN "\",\n"
		  "                }",
		  NAME(config, platform), run->flags & PROP_SET ? "Run" : "Build", NAME(config, platform));

	return 0;
}

static int add_launch(proj_t *proj, const dict_t *projects, const prop_str_t *config, const prop_str_t *platform, FILE *f)
{
	if (config && cstr_eq(config->data, config->len, CSTR("Release"))) {
		return f5anything(proj, config, platform, f);
	} else {
		return cppdbg(proj, projects, config, platform, f);
	}
}

int vc_proj_gen_launch(proj_t *proj, const dict_t *projects, const prop_t *sln_props, FILE *f)
{
	const prop_str_t *name = proj->name;

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *platforms = &sln_props[SLN_PROP_PLATFORMS];

	if (!(configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		add_launch(proj, projects, NULL, NULL, f);
	} else if ((configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			add_launch(proj, projects, config, NULL, f);

			if (i < configs->arr.cnt - 1) {
				c_fprintf(f, ",\n");
			}
		}
	} else if (!(configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < platforms->arr.cnt; i++) {
			const prop_str_t *platform = arr_get(&platforms->arr, i);

			add_launch(proj, projects, NULL, platform, f);

			if (i < platforms->arr.cnt - 1) {
				c_fprintf(f, ",\n");
			}
		}
	} else if ((platforms->flags & PROP_SET) && (configs->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			for (uint j = 0; j < platforms->arr.cnt; j++) {
				const prop_str_t *platform = arr_get(&platforms->arr, j);

				add_launch(proj, projects, config, platform, f);

				if (i < configs->arr.cnt - 1 || j < platforms->arr.cnt - 1) {
					c_fprintf(f, ",\n");
				}
			}
		}
	}

	return 0;
}
