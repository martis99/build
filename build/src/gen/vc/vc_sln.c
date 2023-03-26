#include "vc_sln.h"

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

	const prop_t *configs = &p->sln->props[SLN_PROP_CONFIGS];

	p_fprintf(p->file, "%.*s\n", !p->first, ",");
	((generate_build_priv_t *)p)->first = 0;

	vc_proj_gen_build(proj, configs, p->file);
}

static void generate_launch(void *key, size_t ksize, void *value, const void *priv)
{
	const generate_build_priv_t *p = priv;
	const proj_t *proj	       = value;

	if (proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXE) {
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
	if (path_child(&tasks_path, ".vscode", 7)) {
		return 1;
	}

	folder_create(tasks_path.path);

	path_t launch_path = tasks_path;

	if (path_child(&tasks_path, "tasks.json", 11)) {
		return 1;
	}

	int ret = 0;

	FILE *file = file_open(tasks_path.path, "w", 1);
	if (file == NULL) {
		printf("Failed to create file: %s, errno: %d\n", tasks_path.path, errno);
		return 1;
	}

	const prop_t *configs = &sln->props[SLN_PROP_CONFIGS];
	const prop_t *outdir  = &sln->props[SLN_PROP_OUTDIR];
	const prop_t *startup = &sln->props[SLN_PROP_STARTUP];

	const proj_t *startup_proj = NULL;
	if (hashmap_get(&sln->projects, startup->value.data, startup->value.len, (void **)&startup_proj)) {
		ERR("project doesn't exists: '%.*s'", startup->value.len, startup->value.data);
		return 1;
	}

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

	fclose(file);
	if (ret == 0) {
		SUC("generating tasks: %s success", tasks_path.path);
	} else {
		ERR("generating tasks: %s failed", tasks_path.path);
	}

	if (!(startup->flags & PROP_SET) || !(outdir->flags & PROP_SET)) {
		return 0;
	}

	MSG("%s", "generating launch");

	ret = 0;

	if (path_child(&launch_path, "launch.json", 11)) {
		return 1;
	}

	file = file_open(launch_path.path, "w", 1);
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

	fclose(file);
	if (ret == 0) {
		SUC("generating tasks: %s success", launch_path.path);
	} else {
		ERR("generating tasks: %s failed", launch_path.path);
	}

	return 0;
}
