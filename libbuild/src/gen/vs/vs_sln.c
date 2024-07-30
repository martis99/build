#include "gen/vs/vs_sln.h"

#include "gen/dir.h"
#include "gen/proj.h"
#include "gen/proj_gen_pgc.h"
#include "gen/vs/pgc_gen_vs_proj.h"
#include "gen/vs/pgc_gen_vs_user.h"

#include "common.h"

str_t *vs_proj_get_vars(const proj_t *proj, str_t *vars)
{
	vars[PROJ_VAR_SLNDIR]	= STR("$(SolutionDir)");
	vars[PROJ_VAR_PROJDIR]	= strf("$(SolutionDir)%.*s", proj->rel_dir.len, proj->rel_dir.path);
	vars[PROJ_VAR_PROJNAME] = strc(proj->name.data, proj->name.len);
	vars[PROJ_VAR_CONFIG]	= STR("$(Configuration)");
	vars[PROJ_VAR_ARCH]	= STR("$(PlatformTarget)");
	return vars;
}

int vs_sln_gen(sln_t *sln, const path_t *path)
{
	if (!folder_exists(path->path)) {
		ERR("folder does not exists: %.*s", (int)path->len, path->path);
		return 1;
	}

	str_t vars[__PROJ_VAR_MAX] = { 0 };

	int ret = 0;
	const proj_t **pproj;
	arr_foreach(&sln->build_order, pproj)
	{
		proj_t *proj = *(proj_t **)pproj;

		proj_gen_pgc(proj, sln->props, &proj->pgc);

		vs_proj_get_vars(proj, vars);
		pgc_replace_vars(&proj->pgc, &proj->pgcr, s_proj_vars, vars, __PROJ_VAR_MAX, '/');

		for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
			if (!(proj->pgcr.builds & (1 << b))) {
				continue;
			}

			str_t guid = proj->pgcr.str[PGC_STR_GUID];
			if (b == PGC_BUILD_SHARED) {
				proj->pgcr.str[PGC_STR_GUID] = strc(proj->guid2, sizeof(proj->guid2) - 1);
			}

			xml_init(&proj->gen.xml, 256, 256);
			xml_tag_t xproj = pgc_gen_vs_proj(&proj->pgcr, &proj->gen.xml, b);

			path_t gen_path = { 0 };
			path_init(&gen_path, proj->dir.path, proj->dir.len);

			if (!folder_exists(gen_path.path)) {
				folder_create(gen_path.path);
			}

			if (path_child(&gen_path, proj->name.data, proj->name.len) == NULL) {
				return 1;
			}

			if (path_child_s(&gen_path, CSTR("vcxproj"), '.') == NULL) {
				return 1;
			}

			FILE *file = file_open(gen_path.path, "w");
			if (file == NULL) {
				return 1;
			}

			xml_print(&proj->gen.xml, xproj, PRINT_DST_FILE(file));
			file_close(file);
			xml_free(&proj->gen.xml);

			if (b == PGC_BUILD_SHARED) {
				proj->pgcr.str[PGC_STR_GUID] = guid;
			}

			xml_init(&proj->gen.xml, 256, 256);
			xml_tag_t xuser = pgc_gen_vs_user(&proj->pgcr, &proj->gen.xml);

			path_t gen_path_user = gen_path;
			path_child_s(&gen_path_user, CSTR("user"), '.');

			file = file_open(gen_path_user.path, "w");
			if (file == NULL) {
				return 1;
			}

			ret |= xml_print(&proj->gen.xml, xuser, PRINT_DST_FILE(file)) == 0;

			file_close(file);
			xml_free(&proj->gen.xml);
		}
	}

	path_t cmake_path = *path;
	if (path_child(&cmake_path, sln->props[SLN_PROP_NAME].value.val.data, sln->props[SLN_PROP_NAME].value.val.len) == NULL) {
		return 1;
	}

	if (path_child_s(&cmake_path, "sln", 3, '.') == NULL) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating solution: %s", cmake_path.path);

	c_fprintf(file, "Microsoft Visual Studio Solution File, Format Version 12.00\n"
			"# Visual Studio Version 17\n"
			"VisualStudioVersion = 17.4.33205.214\n"
			"MinimumVisualStudioVersion = 10.0.40219.1\n");

	dict_foreach(&sln->dirs, pair)
	{
		dir_t *dir = pair->value;

		c_fprintf(file, "Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"%.*s\", \"%.*s\", \"{%s}\"\nEndProject\n", (int)dir->name.len, dir->name.data,
			  (int)dir->name.len, dir->name.data, dir->guid);
	}

	dict_foreach(&sln->projects, pair)
	{
		proj_t *proj  = pair->value;
		byte buf[256] = { 0 };
		mem_cpy(buf, sizeof(buf), proj->gen_path.path, proj->gen_path.len);
		convert_backslash(buf, proj->gen_path.len);
		c_fprintf(file, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%.*s\", \"%.*s\", \"{%s}\"\nEndProject\n", (int)proj->name.len, proj->name.data,
			  (int)proj->gen_path.len, buf, proj->guid);
		if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
			mem_cpy(buf, sizeof(buf), proj->gen_path_d.path, proj->gen_path_d.len);
			convert_backslash(buf, proj->gen_path_d.len);
			c_fprintf(file, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%.*s.d\", \"%.*s\", \"{%s}\"\nEndProject\n", (int)proj->name.len,
				  proj->name.data, (int)proj->gen_path_d.len, buf, proj->guid2);
		}
	}

	c_fprintf(file, "Global\n"
			"\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n");

	if (!(sln->props[SLN_PROP_CONFIGS].flags & PROP_SET) || !(sln->props[SLN_PROP_ARCHS].flags & PROP_SET) || sln->props[SLN_PROP_CONFIGS].arr.cnt < 1 ||
	    sln->props[SLN_PROP_ARCHS].arr.cnt < 1) {
		ERR("%s", "at least one config and arch must be set");
		return ret + 1;
	}

	const prop_t *configs = &sln->props[SLN_PROP_CONFIGS];
	const arr_t *archs    = &sln->props[SLN_PROP_ARCHS].arr;

	for (uint i = 0; i < configs->arr.cnt; i++) {
		prop_str_t *config = arr_get(&configs->arr, i);
		for (uint j = 0; j < archs->cnt; j++) {
			prop_str_t *arch = arr_get(archs, j);
			c_fprintf(file, "\t\t%.*s|%.*s = %.*s|%.*s\n", config->val.len, config->val.data, arch->val.len, arch->val.data, config->val.len,
				  config->val.data, arch->val.len, arch->val.data);
		}
	}

	c_fprintf(file, "\tEndGlobalSection\n"
			"\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n");

	dict_foreach(&sln->projects, pair)
	{
		const proj_t *proj = pair->value;

		for (uint i = 0; i < configs->arr.cnt; i++) {
			prop_str_t *config = arr_get(&configs->arr, i);
			for (uint j = 0; j < archs->cnt; j++) {
				prop_str_t *arch  = arr_get(archs, j);
				const char *platf = arch->val.data;
				size_t platf_len  = arch->val.len;
				if (cstr_eq(arch->val.data, arch->val.len, CSTR("i386"))) {
					platf	  = "Win32";
					platf_len = 5;
				}

				c_fprintf(file,
					  "\t\t{%s}.%.*s|%.*s.ActiveCfg = %.*s|%.*s\n"
					  "\t\t{%s}.%.*s|%.*s.Build.0 = %.*s|%.*s\n",
					  proj->guid, config->val.len, config->val.data, arch->val.len, arch->val.data, config->val.len, config->val.data, platf_len,
					  platf, proj->guid, config->val.len, config->val.data, arch->val.len, arch->val.data, config->val.len, config->val.data,
					  platf_len, platf);
			}
		}

		if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
			for (uint i = 0; i < configs->arr.cnt; i++) {
				prop_str_t *config = arr_get(&configs->arr, i);
				for (uint j = 0; j < archs->cnt; j++) {
					prop_str_t *arch  = arr_get(archs, j);
					const char *platf = arch->val.data;
					size_t platf_len  = arch->val.len;
					if (cstr_eq(arch->val.data, arch->val.len, CSTR("i386"))) {
						platf	  = "Win32";
						platf_len = 5;
					}

					c_fprintf(file,
						  "\t\t{%s}.%.*s|%.*s.ActiveCfg = %.*s|%.*s\n"
						  "\t\t{%s}.%.*s|%.*s.Build.0 = %.*s|%.*s\n",
						  proj->guid2, config->val.len, config->val.data, arch->val.len, arch->val.data, config->val.len, config->val.data,
						  platf_len, platf, proj->guid2, config->val.len, config->val.data, arch->val.len, arch->val.data, config->val.len,
						  config->val.data, platf_len, platf);
				}
			}
		}
	}

	c_fprintf(file, "\tEndGlobalSection\n"
			"\tGlobalSection(SolutionProperties) = preSolution\n"
			"\t\tHideSolutionNode = FALSE\n"
			"\tEndGlobalSection\n"
			"\tGlobalSection(NestedProjects) = preSolution\n");

	dict_foreach(&sln->dirs, pair)
	{
		const dir_t *dir = pair->value;

		if (dir->parent) {
			c_fprintf(file, "\t\t{%s} = {%s}\n", dir->guid, dir->parent->guid);
		}
	}

	dict_foreach(&sln->projects, pair)
	{
		const proj_t *proj = pair->value;

		if (proj->parent) {
			c_fprintf(file, "\t\t{%s} = {%s}\n", proj->guid, proj->parent->guid);
			if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
				c_fprintf(file, "\t\t{%s} = {%s}\n", proj->guid2, proj->parent->guid);
			}
		}
	}

	c_fprintf(file,
		  "\tEndGlobalSection\n"
		  "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
		  "\t\tSolutionGuid = {%s}\n"
		  "\tEndGlobalSection\n"
		  "EndGlobal\n",
		  sln->guid);

	file_close(file);
	if (ret == 0) {
		SUC("generating solution: %s success", cmake_path.path);
	} else {
		ERR("generating solution: %s failed", cmake_path.path);
	}

	return ret;
}
