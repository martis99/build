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

static int add_task(const proj_t *proj, const prop_t *sln_props, const prop_str_t *config, const prop_str_t *platform, const char *prefix, const char *action,
		    json_t *json, json_val_t tasks)
{
	const prop_str_t *name = proj->name;

	const prop_t *run = &proj->props[PROJ_PROP_RUN];

	const json_val_t task = json_add_val(json, tasks, str_null(), JSON_OBJ());
	json_add_val(json, task, STR("label"), JSON_STR(strf("%s-" NAME_PATTERN, prefix, NAME(config, platform))));
	json_add_val(json, task, STR("type"), JSON_STR(STR("shell")));
	json_add_val(json, task, STR("command"), JSON_STR(STR("make")));
	const json_val_t args = json_add_val(json, task, STR("args"), JSON_ARR());
	json_add_val(json, args, str_null(), JSON_STR(STR("clean")));
	json_add_val(json, args, str_null(), JSON_STR(strf("%.*s/%s", name->len, name->data, action)));

	if (config) {
		json_add_val(json, args, str_null(), JSON_STR(strf("CONFIG=%.*s", config->len, config->data)));
	}

	if (platform) {
		json_add_val(json, args, str_null(), JSON_STR(strf("PLATFORM=%.*s", platform->len, platform->data)));
	}

	if (run->flags & PROP_SET) {
		json_add_val(json, task, STR("isBackground"), JSON_BOOL(1));
		const json_val_t matchers = json_add_val(json, task, STR("problemMatcher"), JSON_ARR());
		const json_val_t matcher  = json_add_val(json, matchers, str_null(), JSON_OBJ());

		const json_val_t patterns = json_add_val(json, matcher, STR("pattern"), JSON_ARR());
		const json_val_t pattern  = json_add_val(json, patterns, str_null(), JSON_OBJ());
		json_add_val(json, pattern, STR("regexp"), JSON_STR(STR(".")));
		json_add_val(json, pattern, STR("file"), JSON_INT(1));
		json_add_val(json, pattern, STR("location"), JSON_INT(2));
		json_add_val(json, pattern, STR("message"), JSON_INT(3));

		const json_val_t background = json_add_val(json, matcher, STR("background"), JSON_OBJ());
		json_add_val(json, background, STR("activeOnStart"), JSON_BOOL(1));
		json_add_val(json, background, STR("beginsPattern"), JSON_STR(STR(".")));
		json_add_val(json, background, STR("endsPattern"), JSON_STR(STR(".")));
	}

	const json_val_t group = json_add_val(json, task, STR("group"), JSON_OBJ());
	json_add_val(json, group, STR("kind"), JSON_STR(STR("build")));
	json_add_val(json, group, STR("isDefault"), JSON_BOOL(1));

	return 0;
}

static int add_tasks(const proj_t *proj, const prop_t *sln_props, const char *prefix, const char *action, json_t *json, json_val_t tasks)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *platforms = &sln_props[SLN_PROP_PLATFORMS];

	if (!(configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		add_task(proj, sln_props, NULL, NULL, prefix, action, json, tasks);
	} else if ((configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);
			add_task(proj, sln_props, config, NULL, prefix, action, json, tasks);
		}
	} else if (!(configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < platforms->arr.cnt; i++) {
			const prop_str_t *platform = arr_get(&platforms->arr, i);
			add_task(proj, sln_props, NULL, platform, prefix, action, json, tasks);
		}
	} else if ((configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			for (uint j = 0; j < platforms->arr.cnt; j++) {
				const prop_str_t *platform = arr_get(&platforms->arr, j);
				add_task(proj, sln_props, config, platform, prefix, action, json, tasks);
			}
		}
	}

	return 0;
}

int vc_proj_gen_build(const proj_t *proj, const prop_t *sln_props, json_t *json, json_val_t tasks)
{
	add_tasks(proj, sln_props, "Build", "compile", json, tasks);

	if (proj_runnable(proj)) {
		add_tasks(proj, sln_props, "Run", "run", json, tasks);
	}

	return 0;
}

static json_val_t add_launch_header(const proj_t *proj, const prop_str_t *config, const prop_str_t *platform, json_t *json, json_val_t confs, str_t type)
{
	const prop_str_t *name = proj->name;

	const prop_t *run = &proj->props[PROJ_PROP_RUN];

	const json_val_t conf = json_add_val(json, confs, str_null(), JSON_OBJ());
	json_add_val(json, conf, STR("name"), JSON_STR(strf(NAME_PATTERN, NAME(config, platform))));
	json_add_val(json, conf, STR("type"), JSON_STR(type));
	json_add_val(json, conf, STR("request"), JSON_STR(STR("launch")));
	json_add_val(json, conf, STR("preLaunchTask"), JSON_STR(strf("%s-" NAME_PATTERN, run->flags & PROP_SET ? "Run" : "Build", NAME(config, platform))));

	return conf;
}

static int cppdbg(proj_t *proj, const dict_t *projects, const prop_str_t *config, const prop_str_t *platform, json_t *json, json_val_t confs)
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

	const json_val_t conf = add_launch_header(proj, config, platform, json, confs, STR("cppdbg"));

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

	json_add_val(json, conf, STR("program"), JSON_STR(str_cpy(mtarget)));

	const json_val_t jargs = json_add_val(json, conf, STR("args"), JSON_ARR());

	if (args->flags & PROP_SET) {
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

			json_add_val(json, jargs, str_null(), JSON_STR(strn(buf, buf_len, buf_len + 1)));

			arg = next;
		}
	}

	if (run->flags & PROP_SET) {
		json_add_val(json, conf, STR("miDebuggerServerAddress"), JSON_STR(STR("localhost:1234")));
	}

	json_add_val(json, conf, STR("stopAtEntry"), JSON_BOOL(0));
	json_add_val(json, conf, STR("cwd"), JSON_STR(strf("${workspaceFolder}/%.*s", cwd->len, cwd->data)));
	json_add_val(json, conf, STR("environment"), JSON_ARR());
	json_add_val(json, conf, STR("externalConsole"), JSON_BOOL(0));
	json_add_val(json, conf, STR("MIMode"), JSON_STR(STR("gdb")));

	json_val_t cmds = json_add_val(json, conf, STR("setupCommands"), JSON_ARR());

	json_val_t cmd1 = json_add_val(json, cmds, str_null(), JSON_OBJ());
	json_add_val(json, cmd1, STR("description"), JSON_STR(STR("Enable pretty-printing for gdb")));
	json_add_val(json, cmd1, STR("text"), JSON_STR(STR("-enable-pretty-printing")));
	json_add_val(json, cmd1, STR("ignoreFailures"), JSON_BOOL(1));

	json_val_t cmd2 = json_add_val(json, cmds, str_null(), JSON_OBJ());
	json_add_val(json, cmd2, STR("description"), JSON_STR(STR("Set Disassembly Flavor to Intel")));
	json_add_val(json, cmd2, STR("text"), JSON_STR(STR("-gdb-set disassembly-flavor intel")));
	json_add_val(json, cmd2, STR("ignoreFailures"), JSON_BOOL(1));

	return 0;
}

static int f5anything(const proj_t *proj, const prop_str_t *config, const prop_str_t *platform, json_t *json, json_val_t confs)
{
	add_launch_header(proj, config, platform, json, confs, STR("f5anything"));

	return 0;
}

static int add_launch(proj_t *proj, const dict_t *projects, const prop_str_t *config, const prop_str_t *platform, json_t *json, json_val_t confs)
{
	if (config && cstr_eq(config->data, config->len, CSTR("Release"))) {
		return f5anything(proj, config, platform, json, confs);
	} else {
		return cppdbg(proj, projects, config, platform, json, confs);
	}
}

int vc_proj_gen_launch(proj_t *proj, const dict_t *projects, const prop_t *sln_props, json_t *json, json_val_t confs)
{
	const prop_str_t *name = proj->name;

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *platforms = &sln_props[SLN_PROP_PLATFORMS];

	if (!(configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		add_launch(proj, projects, NULL, NULL, json, confs);
	} else if ((configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);
			add_launch(proj, projects, config, NULL, json, confs);
		}
	} else if (!(configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < platforms->arr.cnt; i++) {
			const prop_str_t *platform = arr_get(&platforms->arr, i);
			add_launch(proj, projects, NULL, platform, json, confs);
		}
	} else if ((platforms->flags & PROP_SET) && (configs->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			for (uint j = 0; j < platforms->arr.cnt; j++) {
				const prop_str_t *platform = arr_get(&platforms->arr, j);
				add_launch(proj, projects, config, platform, json, confs);
			}
		}
	}

	return 0;
}
