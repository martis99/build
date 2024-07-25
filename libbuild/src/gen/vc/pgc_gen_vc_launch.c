#include "gen/vc/pgc_gen_vc_tasks.h"

#define NAME_FMT				  "%.*s-%.*s-%.*s-%.*s"
#define NAME_VAL(_val)				  _val.len, _val.data
#define NAME_ARGS(_action, _name, _arch, _config) NAME_VAL(_action), NAME_VAL(_name), NAME_VAL(_arch), NAME_VAL(_config)

static json_val_t add_launch_header(const pgc_t *pgc, json_t *json, json_val_t confs, str_t type, pgc_build_type_t b, str_t arch, str_t config)
{
	// clang-format off
	static struct {
		str_t act;
		str_t run;
	} target_c[] = {
		[PGC_BUILD_EXE]	   = { STRS("compile"), STRS("run") },
		[PGC_BUILD_STATIC] = { STRS("static"),  STRS("run_s") },
		[PGC_BUILD_SHARED] = { STRS("shared"),  STRS("run_d") },
		[PGC_BUILD_ELF]	   = { STRS("elf"),     STRS("run_elf") },
		[PGC_BUILD_BIN]	   = { STRS("bin"),     STRS("run_bin" ) },
		[PGC_BUILD_FAT12]  = { STRS("fat12"),   STRS("run_fat12") },
	};
	// clang-format on

	str_t name = pgc->str[PGC_STR_NAME];

	const json_val_t conf = json_add_val(json, confs, str_null(), JSON_OBJ());
	json_add_val(json, conf, STR("name"), JSON_STR(strf(NAME_FMT, NAME_ARGS(target_c[b].run, name, arch, config))));
	json_add_val(json, conf, STR("type"), JSON_STR(type));
	json_add_val(json, conf, STR("request"), JSON_STR(STR("launch")));
	json_add_val(json, conf, STR("preLaunchTask"), JSON_STR(strf(NAME_FMT, NAME_ARGS(target_c[b].act, name, arch, config))));

	return conf;
}

static json_t *pgc_gen_vc_launch_cppdbg(const pgc_t *pgc, json_t *json, json_val_t confs, pgc_build_type_t b, str_t arch, str_t config)
{
	char buf[P_MAX_PATH] = { 0 };

	str_t tmp = strb(buf, sizeof(buf), 0);

	const json_val_t conf = add_launch_header(pgc, json, confs, STR("cppdbg"), b, arch, config);

	str_cat(&tmp, pgc->str[PGC_STR_OUTDIR]);
	str_cat(&tmp, pgc->str[PGC_STR_NAME]);

	json_add_val(json, conf, STR("program"), JSON_STR(str_cpy(tmp)));

	const json_val_t jargs = json_add_val(json, conf, STR("args"), JSON_ARR());

	if (pgc->str[PGC_STR_ARGS].len > 0) {
		int end = 0;

		str_t arg  = pgc->str[PGC_STR_ARGS];
		str_t next = { 0 };

		while (!end) {
			if (str_chr(arg, &arg, &next, ' ')) {
				str_chr(arg, &arg, &next, '\n');
				end = 1;
			}

			json_add_val(json, jargs, str_null(), JSON_STR(strn(arg.data, arg.len, arg.len + 1)));

			arg = next;
		}
	}

	//TODO: Hardcoded
	if ((str_eq(config, STR("Debug")) && pgc->target[PGC_TARGET_STR_RUN_DBG][PGC_BUILD_BIN].data) || pgc->target[PGC_TARGET_STR_RUN][PGC_BUILD_BIN].data) {
		json_add_val(json, conf, STR("miDebuggerServerAddress"), JSON_STR(STR("localhost:1234")));
	}

	json_add_val(json, conf, STR("stopAtEntry"), JSON_BOOL(0));
	if (pgc->str[PGC_STR_CWD].data) {
		str_t cwd = strn(pgc->str[PGC_STR_CWD].data, pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].len + 10);
		//TODO: Hardcoded
		str_replace(&cwd, STR("$(SLNDIR)"), STR("${workspaceFolder}/"));
		json_add_val(json, conf, STR("cwd"), JSON_STR(cwd));
	}
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

	str_free(&tmp);

	return json;
}

static json_t *pgc_gen_vc_launch_f5anything(const pgc_t *pgc, json_t *json, json_val_t confs, pgc_build_type_t b, str_t arch, str_t config)
{
	// clang-format off
	static struct {
		str_t act;
	} target_c[] = {
		[PGC_BUILD_EXE]	   = { STRS("compile") },
		[PGC_BUILD_STATIC] = { STRS("static") },
		[PGC_BUILD_SHARED] = { STRS("shared") },
		[PGC_BUILD_ELF]	   = { STRS("elf") },
		[PGC_BUILD_BIN]	   = { STRS("bin") },
		[PGC_BUILD_FAT12]  = { STRS("fat12") },
	};
	// clang-format on

	str_t name = pgc->str[PGC_STR_NAME];

	const json_val_t conf = json_add_val(json, confs, str_null(), JSON_OBJ());
	json_add_val(json, conf, STR("name"), JSON_STR(strf(NAME_FMT, NAME_ARGS(target_c[b].act, name, arch, config))));
	json_add_val(json, conf, STR("type"), JSON_STR(STR("f5anything")));
	json_add_val(json, conf, STR("request"), JSON_STR(STR("launch")));
	json_add_val(json, conf, STR("preLaunchTask"), JSON_STR(strf(NAME_FMT, NAME_ARGS(target_c[b].act, name, arch, config))));

	return json;
}

json_t *pgc_gen_vc_launch(const pgc_t *pgc, json_t *json, json_val_t configs)
{
	if (pgc == NULL || json == NULL) {
		return NULL;
	}

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (pgc->str[PGC_STR_OUTDIR].data == NULL || pgc->str[PGC_STR_NAME].data == NULL || (pgc->builds & (1 << b)) == 0) {
			continue;
		}

		const str_t *arch;
		arr_foreach(&pgc->arr[PGC_ARR_ARCHS], arch)
		{
			const str_t *conf;
			arr_foreach(&pgc->arr[PGC_ARR_CONFIGS], conf)
			{
				if ((str_eq(*conf, STR("Debug")) && pgc->target[PGC_TARGET_STR_RUN_DBG][b].data) || pgc->target[PGC_TARGET_STR_RUN][b].data ||
				    b == PGC_BUILD_EXE) {
					pgc_gen_vc_launch_cppdbg(pgc, json, configs, b, *arch, *conf);
				} else {
					pgc_gen_vc_launch_f5anything(pgc, json, configs, b, *arch, *conf);
				}
			}
		}
	}

	return json;
}
