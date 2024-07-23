#include "vc_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

static size_t resolve(const prop_str_t *prop, char *buf, size_t buf_size, const proj_t *proj, const prop_str_t *config, const prop_str_t *arch, const char *outdir,
		      size_t outdir_len)
{
	size_t buf_len = prop->val.len;
	mem_cpy(buf, buf_size, prop->val.data, prop->val.len);

	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(SLN_DIR)"), CSTR("${workspaceFolder}/"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_DIR)"), proj->rel_dir.path, proj->rel_dir.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_NAME)"), proj->name.data, proj->name.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(CONFIG)"), config ? config->val.data : "", config ? config->val.len : 0, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(ARCH)"), arch ? arch->val.data : "", arch ? arch->val.len : 0, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(BIN)"), CSTR("$(BIN_DIR)$(BIN_FILE)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(BIN_DIR)"), outdir, outdir_len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(BIN_FILE)"), proj->name.data, proj->name.len, NULL);
	convert_slash(buf, buf_len);

	return buf_len;
}

#define NAME_PATTERN			  "%.*s-%s%s%.*s%s%.*s"
#define NAME_VAL(_val)			  _val ? "-" : "", _val ? ((const prop_str_t *)_val)->val.len : 0, _val ? ((const prop_str_t *)_val)->val.data : ""
#define NAME(_action, _config, _arch) name->len, name->data, _action, NAME_VAL(_config), NAME_VAL(_arch)

static json_val_t add_launch_header(const proj_t *proj, const char *action, const prop_str_t *config, const prop_str_t *arch, json_t *json, json_val_t confs,
				    str_t type)
{
	const str_t *name = &proj->name;

	const prop_t *run = &proj->props[PROJ_PROP_RUN];

	const json_val_t conf = json_add_val(json, confs, str_null(), JSON_OBJ());
	json_add_val(json, conf, STR("name"), JSON_STR(strf(NAME_PATTERN, NAME(action, config, arch))));
	json_add_val(json, conf, STR("type"), JSON_STR(type));
	json_add_val(json, conf, STR("request"), JSON_STR(STR("launch")));
	json_add_val(json, conf, STR("preLaunchTask"), JSON_STR(strf(NAME_PATTERN, NAME(action, config, arch))));

	return conf;
}

static int cppdbg(proj_t *proj, const dict_t *projects, const char *action, const prop_str_t *config, const prop_str_t *arch, json_t *json, json_val_t confs)
{
	const str_t *name = &proj->name;

	const proj_type_t type = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *run     = &proj->props[PROJ_PROP_RUN];
	const prop_t *wdir    = &proj->props[PROJ_PROP_WDIR];
	const prop_t *args    = &proj->props[PROJ_PROP_ARGS];
	const prop_t *program = &proj->props[PROJ_PROP_PROGRAM];

	prop_str_t rel_path = { .val = strc(proj->rel_dir.path, proj->rel_dir.len) };

	const prop_str_t *outdir = &proj->props[PROJ_PROP_OUTDIR].value;
	const prop_str_t *cwd	 = (wdir->flags & PROP_SET) ? &wdir->value : &rel_path;

	char out[P_MAX_PATH] = { 0 };
	char buf[P_MAX_PATH] = { 0 };

	size_t out_len;
	size_t buf_len;

	out_len = resolve(outdir, CSTR(out), proj, config, arch, "", 0);

	const json_val_t conf = add_launch_header(proj, action, config, arch, json, confs, STR("cppdbg"));

	proj_t *pproj = proj;

	if (program->flags & PROP_SET) {
		const prop_str_t *pname = &program->value;
		if (dict_get(projects, pname->val.data, pname->val.len, (void **)&pproj)) {
			ERR("project doesn't exists: '%.*s'", (int)pname->val.len, pname->val.data);
			return 1;
		}
	}

	make_ext_set_val(&pproj->gen.make, STR("SLNDIR"), MSTR(STR("${workspaceFolder}/")));
	make_ext_set_val(&pproj->gen.make, STR("CONFIG"), MSTR(strs(config->val)));
	make_ext_set_val(&pproj->gen.make, STR("ARCH"), MSTR(strs(arch->val)));
	make_expand(&pproj->gen.make);
	str_t mtarget = make_var_get_resolved(&pproj->gen.make, STR("TARGET"));

	json_add_val(json, conf, STR("program"), JSON_STR(str_cpy(mtarget)));

	const json_val_t jargs = json_add_val(json, conf, STR("args"), JSON_ARR());

	if (args->flags & PROP_SET) {
		int end = 0;

		str_t arg  = args->value.val;
		str_t next = { 0 };

		while (!end) {
			if (str_chr(arg, &arg, &next, ' ')) {
				str_chr(arg, &arg, &next, '\n');
				end = 1;
			}

			const prop_str_t argp = { .val = strc(arg.data, arg.len) };

			buf_len = resolve(&argp, CSTR(buf), proj, config, arch, out, out_len);

			json_add_val(json, jargs, str_null(), JSON_STR(strn(buf, buf_len, buf_len + 1)));

			arg = next;
		}
	}

	if (run->flags & PROP_SET) {
		json_add_val(json, conf, STR("miDebuggerServerAddress"), JSON_STR(STR("localhost:1234")));
	}

	json_add_val(json, conf, STR("stopAtEntry"), JSON_BOOL(0));
	json_add_val(json, conf, STR("cwd"), JSON_STR(strf("${workspaceFolder}/%.*s", cwd->val.len - 1, cwd->val.data)));
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

static int f5anything(const proj_t *proj, const prop_str_t *config, const prop_str_t *arch, json_t *json, json_val_t confs)
{
	add_launch_header(proj, "compile", config, arch, json, confs, STR("f5anything"));

	return 0;
}

static int add_launch(proj_t *proj, const dict_t *projects, const prop_str_t *config, const prop_str_t *arch, json_t *json, json_val_t confs)
{
	if (config && cstr_eq(config->val.data, config->val.len, CSTR("Release"))) {
		return f5anything(proj, config, arch, json, confs);
	} else {
		return cppdbg(proj, projects, "compile", config, arch, json, confs);
	}
}

int vc_proj_gen_launch(proj_t *proj, const dict_t *projects, const prop_t *sln_props, json_t *json, json_val_t confs)
{
	const str_t *name = &proj->name;

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *archs = &sln_props[SLN_PROP_ARCHS];

	if (!(configs->flags & PROP_SET) && !(archs->flags & PROP_SET)) {
		add_launch(proj, projects, NULL, NULL, json, confs);
	} else if ((configs->flags & PROP_SET) && !(archs->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);
			add_launch(proj, projects, config, NULL, json, confs);
		}
	} else if (!(configs->flags & PROP_SET) && (archs->flags & PROP_SET)) {
		for (uint i = 0; i < archs->arr.cnt; i++) {
			const prop_str_t *arch = arr_get(&archs->arr, i);
			add_launch(proj, projects, NULL, arch, json, confs);
		}
	} else if ((archs->flags & PROP_SET) && (configs->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			for (uint j = 0; j < archs->arr.cnt; j++) {
				const prop_str_t *arch = arr_get(&archs->arr, j);
				add_launch(proj, projects, config, arch, json, confs);
			}
		}
	}

	return 0;
}
