#include "mk_proj.h"

#include "gen/mk/pgc_gen_mk.h"
#include "gen/proj_gen.h"
#include "gen/sln.h"

#include "common.h"

str_t *mk_proj_get_vars(const proj_t *proj, str_t *vars)
{
	vars[PROJ_VAR_SLNDIR]	= STR("$(SLNDIR)");
	vars[PROJ_VAR_PROJDIR]	= strc(proj->rel_dir.path, proj->rel_dir.len);
	vars[PROJ_VAR_PROJNAME] = strc(proj->name.data, proj->name.len);
	vars[PROJ_VAR_CONFIG]	= STR("$(CONFIG)");
	vars[PROJ_VAR_ARCH]	= STR("$(ARCH)");
	return vars;
}

int mk_proj_gen(proj_t *proj, const dict_t *projects, const prop_t *sln_props)
{
	path_t gen_path = { 0 };
	path_init(&gen_path, proj->dir.path, proj->dir.len);

	if (!folder_exists(gen_path.path)) {
		folder_create(gen_path.path);
	}

	if (path_child(&gen_path, CSTR("Makefile")) == NULL) {
		return 1;
	}

	MSG("generating project: %s", gen_path.path);

	FILE *file = file_open(gen_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	int len = make_print(&proj->gen.make, PRINT_DST_FILE(file));

	file_close(file);

	if (len == 0) {
		ERR("generating project: %s failed", gen_path.path);
		return 1;
	}

	SUC("generating project: %s success", gen_path.path);
	return 0;
}

void mk_proj_free(proj_t *proj)
{
	make_free(&proj->gen.make);
}
