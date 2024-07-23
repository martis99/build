#include "gen/vc/pgc_gen_vc_tasks.h"

#define NAME_FMT				  "%.*s-%.*s-%.*s-%.*s"
#define NAME_VAL(_val)				  _val.len, _val.data
#define NAME_ARGS(_action, _name, _arch, _config) NAME_VAL(_action), NAME_VAL(_name), NAME_VAL(_arch), NAME_VAL(_config)

static json_t *pgc_gen_vc_task(const pgc_t *gen, json_t *json, json_val_t tasks, pgc_build_type_t b, str_t arch, str_t config)
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

	str_t name = gen->str[PGC_STR_NAME];

	const json_val_t task = json_add_val(json, tasks, str_null(), JSON_OBJ());
	json_add_val(json, task, STR("label"), JSON_STR(strf(NAME_FMT, NAME_ARGS(target_c[b].act, name, arch, config))));
	json_add_val(json, task, STR("type"), JSON_STR(STR("shell")));
	json_add_val(json, task, STR("command"), JSON_STR(STR("make")));
	const json_val_t args = json_add_val(json, task, STR("args"), JSON_ARR());
	json_add_val(json, args, str_null(), JSON_STR(strf("%.*s/%.*s", name.len, name.data, target_c[b].act.len, target_c[b].act.data)));
	json_add_val(json, args, str_null(), JSON_STR(strf("ARCH=%.*s", arch.len, arch.data)));
	json_add_val(json, args, str_null(), JSON_STR(strf("CONFIG=%.*s", config.len, config.data)));

	if ((str_eq(config, STR("Debug")) && gen->target[PGC_TARGET_STR_RUN_DBG][b].data) || gen->target[PGC_TARGET_STR_RUN][b].data || b == PGC_BUILD_EXE) {
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

	return json;
}

json_t *pgc_gen_vc_tasks(const pgc_t *gen, json_t *json, json_val_t tasks)
{
	if (gen == NULL || json == NULL) {
		return NULL;
	}

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (gen->str[PGC_STR_OUTDIR].data == NULL || gen->str[PGC_STR_NAME].data == NULL || (gen->builds & (1 << b)) == 0) {
			continue;
		}

		const str_t *arch;
		arr_foreach(&gen->arr[PGC_ARR_ARCHS], arch)
		{
			const str_t *conf;
			arr_foreach(&gen->arr[PGC_ARR_CONFIGS], conf)
			{
				pgc_gen_vc_task(gen, json, tasks, b, *arch, *conf);
			}
		}
	}

	return json;
}
