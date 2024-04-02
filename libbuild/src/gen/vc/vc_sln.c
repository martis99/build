#include "gen/vc/vc_sln.h"

#include "gen/mk/mk_sln.h"
#include "gen/proj.h"
#include "gen/var.h"

#include "vc_proj.h"

#include "common.h"

#include <errno.h>

int vc_sln_gen(sln_t *sln, const path_t *path)
{
	int ret = 0;

	ret |= mk_sln_gen(sln, path);

	MSG("%s", "generating tasks");

	path_t tasks_path = *path;
	if (path_child(&tasks_path, CSTR(".vscode")) == NULL) {
		return 1;
	}

	if (!folder_exists(tasks_path.path)) {
		folder_create(tasks_path.path);
	}

	path_t launch_path = tasks_path;

	if (path_child(&tasks_path, CSTR("tasks.json")) == NULL) {
		return 1;
	}

	const prop_t *configs = &sln->props[SLN_PROP_CONFIGS];
	const prop_t *outdir  = &sln->props[SLN_PROP_OUTDIR];

	{
		json_t json = { 0 };
		json_init(&json, 400);

		const json_val_t root = json_add_val(&json, JSON_END, str_null(), JSON_OBJ());
		json_add_val(&json, root, STR("version"), JSON_STR(STR("2.0.0")));
		const json_val_t tasks = json_add_val(&json, root, STR("tasks"), JSON_ARR());

		const proj_t **pproj;
		arr_foreach(&sln->build_order, pproj)
		{
			vc_proj_gen_build(*pproj, sln->props, &json, tasks);
		}

		FILE *file = file_open(tasks_path.path, "w");
		if (file == NULL) {
			return 1;
		}

		ret |= json_print(&json, root, PRINT_DST_FILE(file), "        ") == 0;

		file_close(file);

		json_free(&json);

		if (ret == 0) {
			SUC("generating tasks: %s success", tasks_path.path);
		} else {
			ERR("generating tasks: %s failed", tasks_path.path);
		}
	}

	if (!(outdir->flags & PROP_SET)) {
		return ret;
	}

	MSG("%s", "generating launch");

	if (path_child(&launch_path, CSTR("launch.json")) == NULL) {
		return 1;
	}

	{
		json_t json = { 0 };
		json_init(&json, 400);

		const json_val_t root = json_add_val(&json, JSON_END, str_null(), JSON_OBJ());
		json_add_val(&json, root, STR("version"), JSON_STR(STR("0.2.0")));
		const json_val_t confs = json_add_val(&json, root, STR("configurations"), JSON_ARR());

		dict_foreach(&sln->projects, pair)
		{
			proj_t *proj = pair->value;

			if (!proj_runnable(proj)) {
				continue;
			}

			vc_proj_gen_launch(proj, &sln->projects, sln->props, &json, confs);
		}

		FILE *file = file_open(launch_path.path, "w");
		if (file == NULL) {
			return 1;
		}

		ret |= json_print(&json, root, PRINT_DST_FILE(file), "        ") == 0;

		file_close(file);

		json_free(&json);

		if (ret == 0) {
			SUC("generating tasks: %s success", launch_path.path);
		} else {
			ERR("generating tasks: %s failed", launch_path.path);
		}
	}

	mk_sln_free(sln);

	return ret;
}
