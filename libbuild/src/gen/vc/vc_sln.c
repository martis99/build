#include "gen/vc/vc_sln.h"

#include "gen/mk/mk_sln.h"
#include "gen/proj.h"
#include "gen/var.h"

#include "vc_proj.h"

#include "common.h"

#include <errno.h>

int vc_sln_gen(sln_t *sln, const path_t *path)
{
	mk_sln_gen(sln, path);

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

	int ret = 0;

	FILE *file = file_open(tasks_path.path, "w");
	if (file == NULL) {
		ERR("failed to create file: %s, errno: %d\n", tasks_path.path, errno);
		return 1;
	}

	const prop_t *configs = &sln->props[SLN_PROP_CONFIGS];
	const prop_t *outdir  = &sln->props[SLN_PROP_OUTDIR];

	c_fprintf(file, "{\n"
			"        \"version\": \"2.0.0\",\n"
			"        \"tasks\": [");

	int first = 1;

	dict_foreach(&sln->projects, pair)
	{
		const proj_t *proj = pair->value;

		c_fprintf(file, "%.*s\n", !first, ",");
		first = 0;

		vc_proj_gen_build(proj, sln->props, file);
	}

	c_fprintf(file, "\n        ]\n"
			"}");

	file_close(file);
	if (ret == 0) {
		SUC("generating tasks: %s success", tasks_path.path);
	} else {
		ERR("generating tasks: %s failed", tasks_path.path);
	}

	if (!(outdir->flags & PROP_SET)) {
		return 0;
	}

	MSG("%s", "generating launch");

	ret = 0;

	if (path_child(&launch_path, CSTR("launch.json")) == NULL) {
		return 1;
	}

	file = file_open(launch_path.path, "w");
	if (file == NULL) {
		printf("Failed to create file: %s, errno: %d\n", launch_path.path, errno);
		return 1;
	}

	c_fprintf(file, "{\n"
			"        \"version\": \"0.2.0\",\n"
			"        \"configurations\": [");

	first = 1;

	dict_foreach(&sln->projects, pair)
	{
		proj_t *proj = pair->value;

		const proj_type_t type = proj->props[PROJ_PROP_TYPE].mask;

		if (type != PROJ_TYPE_EXE && type != PROJ_TYPE_FAT12) {
			continue;
		}

		c_fprintf(file, "%.*s\n", !first, ",");
		first = 0;

		vc_proj_gen_launch(proj, &sln->projects, sln->props, file);
	}

	c_fprintf(file, "\n        ]\n"
			"}");

	file_close(file);

	mk_sln_free(sln);

	if (ret == 0) {
		SUC("generating tasks: %s success", launch_path.path);
	} else {
		ERR("generating tasks: %s failed", launch_path.path);
	}

	return 0;
}
