#include "gen/vc/vc_sln.h"

#include "gen/mk/mk_sln.h"
#include "gen/proj.h"
#include "gen/var.h"

#include "vc_proj.h"

#include "common.h"

#include <errno.h>

typedef struct generate_build_priv_s {
	const sln_t *sln;
	FILE *file;
	int first;
} generate_build_priv_t;

static void generate_build(void *key, size_t ksize, void *value, const void *priv)
{
	const generate_build_priv_t *p = priv;
	const proj_t *proj	       = value;

	p_fprintf(p->file, "%.*s\n", !p->first, ",");
	((generate_build_priv_t *)p)->first = 0;

	vc_proj_gen_build(proj, p->sln->props, p->file);
}

static void generate_launch(void *key, size_t ksize, void *value, const void *priv)
{
	const generate_build_priv_t *p = priv;
	const proj_t *proj	       = value;

	const proj_type_t type = proj->props[PROJ_PROP_TYPE].mask;

	if (type != PROJ_TYPE_EXE && type != PROJ_TYPE_BIN && type != PROJ_TYPE_FAT12) {
		return;
	}

	p_fprintf(p->file, "%.*s\n", !p->first, ",");
	((generate_build_priv_t *)p)->first = 0;

	vc_proj_gen_launch(proj, &p->sln->projects, p->sln->props, p->file);
}

int vc_sln_gen(const sln_t *sln, const path_t *path)
{
	mk_sln_gen(sln, path);

	MSG("%s", "generating tasks");

	path_t tasks_path = *path;
	if (path_child(&tasks_path, CSTR(".vscode"))) {
		return 1;
	}

	if (!folder_exists(tasks_path.path)) {
		folder_create(tasks_path.path);
	}

	path_t launch_path = tasks_path;

	if (path_child(&tasks_path, CSTR("tasks.json"))) {
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

	p_fprintf(file, "{\n"
			"        \"version\": \"2.0.0\",\n"
			"        \"tasks\": [");

	generate_build_priv_t generate_build_priv = {
		.sln   = sln,
		.file  = file,
		.first = 1,
	};

	hashmap_iterate_c(&sln->projects, generate_build, &generate_build_priv);

	p_fprintf(file, "\n        ]\n"
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

	if (path_child(&launch_path, CSTR("launch.json"))) {
		return 1;
	}

	file = file_open(launch_path.path, "w");
	if (file == NULL) {
		printf("Failed to create file: %s, errno: %d\n", launch_path.path, errno);
		return 1;
	}

	p_fprintf(file, "{\n"
			"        \"version\": \"0.2.0\",\n"
			"        \"configurations\": [");

	generate_build_priv.file  = file;
	generate_build_priv.first = 1;

	hashmap_iterate_c(&sln->projects, generate_launch, &generate_build_priv);

	p_fprintf(file, "\n        ]\n"
			"}");

	file_close(file);
	if (ret == 0) {
		SUC("generating tasks: %s success", launch_path.path);
	} else {
		ERR("generating tasks: %s failed", launch_path.path);
	}

	return 0;
}
