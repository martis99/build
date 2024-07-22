#include "mk_proj.h"

#include "gen/mk/pgc_gen_mk.h"
#include "gen/proj_gen.h"
#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

//TODO: Resolve when reading config file
static str_t resolve_path(str_t rel, str_t path, str_t *buf)
{
	if (str_eqn(path, STR("$(SLN_DIR)"), 9)) {
		str_cpyd(path, buf);
		return *buf;
	}

	buf->len = 0;
	str_cat(buf, STR("$(SLN_DIR)"));
	str_cat(buf, rel);
	str_cat(buf, path);
	return *buf;
}

static str_t resolve(str_t str, str_t *buf, const proj_t *proj)
{
	str_cpyd(str, buf);
	str_replace(buf, STR("$(SLN_DIR)"), STR("$(SLNDIR)"));
	str_replace(buf, STR("$(PROJ_DIR)"), strc(proj->rel_dir.path, proj->rel_dir.len));
	str_replace(buf, STR("$(PROJ_NAME)"), strc(proj->name.data, proj->name.len));
	str_replace(buf, STR("$(CONFIG)"), STR("$(CONFIG)"));
	str_replace(buf, STR("$(ARCH)"), STR("$(ARCH)"));
	convert_slash((char *)buf->data, buf->len);
	return *buf;
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

	pgc_t gen = { 0 };
	pgc_init(&gen);

	proj_gen(proj, projects, sln_props, resolve, resolve_path, &gen);

	make_init(&proj->gen.make, 8, 8, 8);
	pgc_gen_mk(&gen, &proj->gen.make);
	pgc_free(&gen);

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
