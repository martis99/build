#include "cm_proj.h"

#include "gen/var.h"

#include "common.h"

static const var_pol_t vars = {
	.old = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
		[VAR_SLN_NAME] = "$(SLN_NAME)",
		[VAR_PROJ_DIR] = "$(PROJ_DIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.new = {
		[VAR_SLN_DIR] = "${CMAKE_SOURCE_DIR}",
		[VAR_SLN_NAME] = "${CMAKE_PROJECT_NAME}",
		[VAR_PROJ_DIR] = "$(ProjectDir)",
		[VAR_CONFIG] = "$(Configuration)",
		[VAR_PLATFORM] = "${CMAKE_VS_PLATFORM_NAME}",
	},
};

static size_t resolve(const prop_str_t *prop, char *dst, size_t dst_max_len, const proj_t *proj)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len, dst_len;

	buf_len = convert_slash(CSTR(buf), prop->data, prop->len);
	dst_len = cstr_replaces(buf, buf_len, dst, dst_max_len, vars.old, vars.new, __VAR_MAX);
	buf_len = cstr_replace(dst, dst_len, CSTR(buf), CSTR("$(PROJ_NAME)"), proj->name->data, proj->name->len);
	dst_len = cstr_replace(buf, buf_len, dst, dst_max_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len);

	return dst_len;
}

static inline void print_rel_path(FILE *fp, const proj_t *proj, const char *path, size_t path_len)
{
	char path_b[P_MAX_PATH]	  = { 0 };
	size_t path_b_len	  = convert_slash(CSTR(path_b), path, path_len);
	char rel_path[P_MAX_PATH] = { 0 };
	size_t rel_path_len	  = convert_slash(CSTR(rel_path), proj->rel_path.path, proj->rel_path.len);

	if (cstrn_cmp(path_b, path_b_len, CSTR("${CMAKE_SOURCE_DIR}"), 19)) {
		p_fprintf(fp, "%.*s", path_b_len, path_b);
	} else {
		p_fprintf(fp, "${CMAKE_SOURCE_DIR}/%.*s/%.*s", rel_path_len, rel_path, path_b_len, path_b);
	}
}

int cm_proj_gen(const proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *sln_props)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *langs   = &proj->props[PROJ_PROP_LANGS];
	const prop_t *charset = &proj->props[PROJ_PROP_CHARSET];
	const prop_t *cflags  = &proj->props[PROJ_PROP_CFLAGS];
	const prop_t *outdir  = &proj->props[PROJ_PROP_OUTDIR];
	const prop_t *intdir  = &proj->props[PROJ_PROP_INTDIR];

	char enclude[P_MAX_PATH] = { 0 };
	char buf[P_MAX_PATH]	 = { 0 };
	char buf2[P_MAX_PATH]	 = { 0 };

	size_t enclude_len;
	size_t buf_len;
	size_t buf2_len;

	if (proj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
		enclude_len = convert_slash(CSTR(enclude), proj->props[PROJ_PROP_ENCLUDE].value.data, proj->props[PROJ_PROP_ENCLUDE].value.len);
	}

	int ret = 0;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, CSTR("CMakeLists.txt"))) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	uint lang = langs->mask;

	if ((proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) || (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) || (proj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET)) {
		p_fprintf(file, "file(GLOB_RECURSE %.*s_SOURCE", name->len, name->data);
		if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
			const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

			for (uint i = 0; i < sources->cnt; i++) {
				prop_str_t *source = arr_get(sources, i);
				buf_len		   = convert_slash(CSTR(buf), source->data, source->len);

				if (lang & (1 << LANG_ASM)) {
					p_fprintf(file, " %.*s/*.asm %.*s/*.inc", buf_len, buf, buf_len, buf);
				}
				if (lang & (1 << LANG_C)) {
					p_fprintf(file, " %.*s/*.c", buf_len, buf);
				}
				if ((lang & (1 << LANG_C)) || (lang & (1 << LANG_CPP))) {
					p_fprintf(file, " %.*s/*.h", buf_len, buf);
				}
				if (lang & (1 << LANG_CPP)) {
					p_fprintf(file, " %.*s/*.cpp %.*s/*.hpp", buf_len, buf, buf_len, buf);
				}
			}
		}

		if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				prop_str_t *include = arr_get(includes, i);
				buf_len		    = convert_slash(CSTR(buf), include->data, include->len);

				if (lang & (1 << LANG_ASM)) {
					p_fprintf(file, " %.*s/*.inc", buf_len, buf);
				}
				if ((lang & (1 << LANG_C)) || (lang & (1 << LANG_CPP))) {
					p_fprintf(file, " %.*s/*.h", buf_len, buf);
				}
				if (lang & (1 << LANG_CPP)) {
					p_fprintf(file, " %.*s/*.hpp", buf_len, buf);
				}
			}
		}

		p_fprintf(file, ")\n\n");
	}

	if ((proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) || charset->mask == CHARSET_UNICODE) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		if (defines->cnt > 0 || charset->mask == CHARSET_UNICODE) {
			p_fprintf(file, "add_definitions(");
		}

		int first = 1;

		if (charset->mask == CHARSET_UNICODE) {
			p_fprintf(file, first ? "-DUNICODE -D_UNICODE" : " -DUNICODE -D_UNICODE");
			first = 0;
		}

		for (uint i = 0; i < defines->cnt; i++) {
			const prop_str_t *define = arr_get(defines, i);
			p_fprintf(file, first ? "-D%.*s" : " -D%.*s", define->len, define->data);
			first = 0;
		}

		if (!first) {
			p_fprintf(file, ")\n\n");
		}
	}

	switch (type) {
	case PROJ_TYPE_LIB:
		p_fprintf(file, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		break;
	case PROJ_TYPE_EXE: {
		int first = 0;
		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

			if ((dproj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) && dproj->props[PROJ_PROP_LIBDIRS].arr.cnt > 0) {
				first = 1;
				break;
			}
		}

		if (first) {
			p_fprintf(file, "link_directories(");
			first = 1;
			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

				if (dproj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
					const arr_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
					for (uint j = 0; j < libdirs->cnt; j++) {
						prop_str_t *libdir = arr_get(libdirs, j);
						if (libdir->len > 0) {
							if (!first) {
								p_fprintf(file, " ");
							}

							buf_len	 = cstr_replaces(libdir->data, libdir->len, CSTR(buf), vars.old, vars.new, __VAR_MAX);
							buf2_len = cstr_replace(buf, buf_len, CSTR(buf2), CSTR("$(PROJ_NAME)"), dproj->name->data, dproj->name->len);

							print_rel_path(file, dproj, buf2, buf2_len);
							first = 0;
						}
					}
					break;
				}
			}

			p_fprintf(file, ")\n");
		}

		p_fprintf(file, "add_executable(%.*s ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);

		if (proj->all_depends.cnt > 0) {
			p_fprintf(file, "target_link_libraries(%.*s", name->len, name->data);

			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_t *dproj = *(proj_t **)arr_get(&proj->all_depends, i);

				if (dproj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
					p_fprintf(file, " %.*s", dproj->name->len, dproj->name->data);
				}
			}
			p_fprintf(file, ")\n");
		}

		break;
	}
	case PROJ_TYPE_EXT:
		//p_fprintf(fp, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		goto done;
		break;
	default:
		ERR("unknown project type: %d", type);
		ret = 1;
	}

	int first = 1;
	p_fprintf(file, "include_directories(");

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

		for (uint i = 0; i < sources->cnt; i++) {
			prop_str_t *source = arr_get(sources, i);

			buf_len = convert_slash(CSTR(buf), source->data, source->len);

			p_fprintf(file, first ? "%.*s" : " %.*s", buf_len, buf);
			first = 0;
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			prop_str_t *include = arr_get(includes, i);

			buf_len = convert_slash(CSTR(buf), include->data, include->len);

			p_fprintf(file, first ? "%.*s" : " %.*s", buf_len, buf);
			first = 0;
		}
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &iproj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				prop_str_t *include = arr_get(includes, i);
				if (!first) {
					p_fprintf(file, " ");
				}
				print_rel_path(file, iproj, include->data, include->len);
				first = 0;
			}
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			if (!first) {
				p_fprintf(file, " ");
			}

			buf_len	 = cstr_replaces(iproj->props[PROJ_PROP_ENCLUDE].value.data, iproj->props[PROJ_PROP_ENCLUDE].value.len, CSTR(buf), vars.old, vars.new,
						 __VAR_MAX);
			buf2_len = cstr_replace(buf, buf_len, CSTR(buf2), CSTR("$(PROJ_NAME)"), iproj->name->data, iproj->name->len);

			print_rel_path(file, iproj, buf2, buf2_len);
			first = 0;
		}
	}

	p_fprintf(file, ")\n");

	if (!proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		if (lang & (1 << LANG_C)) {
			p_fprintf(file, "set_target_properties(%.*s PROPERTIES LINKER_LANGUAGE C)\n", name->len, name->data);
		}
	}

	if (proj->dir.len > 0 || (outdir->flags & PROP_SET) || (intdir->flags & PROP_SET)) {
		p_fprintf(file, "set_target_properties(%.*s\n    PROPERTIES\n", name->len, name->data);

		if (proj->dir.len > 0) {
			buf_len = convert_slash(CSTR(buf), proj->dir.path, proj->dir.len);
			p_fprintf(file, "    FOLDER \"%.*s\"\n", buf_len, buf);
		}

		if (outdir->flags & PROP_SET) {
			buf_len = resolve(&outdir->value, CSTR(buf), proj);
			p_fprintf(file, "    ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			p_fprintf(file, "\"\n    ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			p_fprintf(file, "\"\n    LIBRARY_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			p_fprintf(file, "\"\n    LIBRARY_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			p_fprintf(file, "\"\n    RUNTIME_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			p_fprintf(file, "\"\n    RUNTIME_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			p_fprintf(file, "\"\n)\n");
		}
	}

	if (proj->props[PROJ_PROP_WDIR].flags & PROP_SET) {
		const prop_str_t *wdir = &proj->props[PROJ_PROP_WDIR].value;
		buf_len		       = convert_slash(CSTR(buf), wdir->data, wdir->len);
		if (wdir->len > 0) {
			p_fprintf(file, "set_property(TARGET %.*s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data, buf_len,
				  buf);
		}
	} else {
		buf_len = convert_slash(CSTR(buf), proj->rel_path.path, proj->rel_path.len);
		p_fprintf(file, "set_property(TARGET %.*s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data,
			  proj->rel_path.len, buf);
	}

done:
	file_close(file);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}
