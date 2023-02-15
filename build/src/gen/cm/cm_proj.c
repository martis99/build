#include "cm_proj.h"

#include "gen/var.h"

#include "defines.h"
#include "utils.h"

static const var_pol_t vars = {
	.names = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
		[VAR_SLN_NAME] = "$(SLN_NAME)",
		[VAR_PROJ_DIR] = "$(PROJ_DIR)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "${CMAKE_SOURCE_DIR}",
		[VAR_SLN_NAME] = "${CMAKE_PROJECT_NAME}",
		[VAR_PROJ_DIR] = "$(ProjectDir)",
		[VAR_CONFIG] = "$(Configuration)",
		[VAR_PLATFORM] = "${CMAKE_VS_PLATFORM_NAME}",
	},
};

static inline void print_rel_path(FILE *fp, const proj_t *proj, const char *path, unsigned int path_len)
{
	char path_b[MAX_PATH] = { 0 };
	convert_slash(path_b, sizeof(path_b) - 1, path, path_len);
	char rel_path[MAX_PATH] = { 0 };
	convert_slash(rel_path, sizeof(rel_path) - 1, proj->rel_path.path, proj->rel_path.len);
	unsigned int rel_path_len = proj->rel_path.len;

	if (cstrn_cmp(path_b, path_len, "${CMAKE_SOURCE_DIR}", 19, 19)) {
		fprintf_s(fp, "%.*s", path_len, path_b);
	} else {
		fprintf_s(fp, "${CMAKE_SOURCE_DIR}/%.*s/%.*s", proj->rel_path.len, rel_path, path_len, path_b);
	}
}

int cm_proj_gen(const proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *langs, charset_t charset, const prop_t *outdir, const prop_t *intdir)
{
	const prop_str_t *name = &proj->props[PROJ_PROP_NAME].value;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	langs	= proj->props[PROJ_PROP_LANGS].set ? &proj->props[PROJ_PROP_LANGS] : langs;
	charset = proj->props[PROJ_PROP_CHARSET].set ? proj->props[PROJ_PROP_CHARSET].mask : charset;
	outdir	= proj->props[PROJ_PROP_OUTDIR].set ? &proj->props[PROJ_PROP_OUTDIR] : outdir;
	intdir	= proj->props[PROJ_PROP_INTDIR].set ? &proj->props[PROJ_PROP_INTDIR] : intdir;

	char source[MAX_PATH]  = { 0 };
	char include[MAX_PATH] = { 0 };
	char enclude[MAX_PATH] = { 0 };
	int source_len	       = 0;
	int include_len	       = 0;
	int enclude_len	       = 0;
	int include_diff       = 0;

	if (proj->props[PROJ_PROP_SOURCE].set) {
		source_len = proj->props[PROJ_PROP_SOURCE].value.len;
		convert_slash(source, sizeof(source) - 1, proj->props[PROJ_PROP_SOURCE].value.data, source_len);
	}

	if (proj->props[PROJ_PROP_INCLUDE].set) {
		include_len = proj->props[PROJ_PROP_INCLUDE].value.len;
		convert_slash(include, sizeof(include) - 1, proj->props[PROJ_PROP_INCLUDE].value.data, include_len);

		include_diff = !proj->props[PROJ_PROP_SOURCE].set || !cstr_cmp(source, source_len, include, include_len);
	}

	if (proj->props[PROJ_PROP_ENCLUDE].set) {
		enclude_len = proj->props[PROJ_PROP_ENCLUDE].value.len;
		convert_slash(enclude, sizeof(enclude) - 1, proj->props[PROJ_PROP_ENCLUDE].value.data, enclude_len);
	}

	int ret = 0;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, "CMakeLists.txt", 14)) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	unsigned int lang = langs->mask;

	if (proj->props[PROJ_PROP_SOURCE].set || proj->props[PROJ_PROP_INCLUDE].set || proj->props[PROJ_PROP_ENCLUDE].set) {
		fprintf_s(fp, "file(GLOB_RECURSE %.*s_SOURCE", name->len, name->data);
		if (proj->props[PROJ_PROP_SOURCE].set) {
			if (lang & (1 << LANG_ASM)) {
				fprintf_s(fp, " %.*s/*.asm %.*s/*.inc", source_len, source, source_len, source);
			}
			if (lang & (1 << LANG_C)) {
				fprintf_s(fp, " %.*s/*.c", source_len, source);
			}
			if ((lang & (1 << LANG_C)) || (lang & (1 << LANG_CPP))) {
				fprintf_s(fp, " %.*s/*.h", source_len, source);
			}
			if (lang & (1 << LANG_CPP)) {
				fprintf_s(fp, " %.*s/*.cpp %.*s/*.hpp", source_len, source, source_len, source);
			}
		}

		if (include_diff) {
			if (lang & (1 << LANG_ASM)) {
				fprintf_s(fp, " %.*s/*.inc", include_len, include);
			}
			if ((lang & (1 << LANG_C)) || (lang & (1 << LANG_CPP))) {
				fprintf_s(fp, " %.*s/*.h", include_len, include);
			}
			if (lang & (1 << LANG_CPP)) {
				fprintf_s(fp, " %.*s/*.hpp", include_len, include);
			}
		}

		fprintf_s(fp, ")\n\n");
	}

	if (proj->props[PROJ_PROP_DEFINES].set || charset == CHARSET_UNICODE) {
		const array_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		if (defines->count > 0 || charset == CHARSET_UNICODE) {
			fprintf_s(fp, "add_definitions(");
		}

		int first = 1;

		if (charset == CHARSET_UNICODE) {
			fprintf_s(fp, first ? "-DUNICODE -D_UNICODE" : " -DUNICODE -D_UNICODE");
			first = 0;
		}

		for (int i = 0; i < defines->count; i++) {
			const prop_str_t *define = array_get(defines, i);
			fprintf_s(fp, first ? "-D%.*s" : " -D%.*s", define->len, define->data);
			first = 0;
		}

		if (!first) {
			fprintf_s(fp, ")\n\n");
		}
	}

	switch (type) {
	case PROJ_TYPE_LIB:
		fprintf_s(fp, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		break;
	case PROJ_TYPE_EXE: {
		int first = 0;
		for (int i = 0; i < proj->all_depends.count; i++) {
			prop_str_t **depend = array_get(&proj->all_depends, i);
			proj_t *dproj	    = NULL;
			if (hashmap_get(projects, (*depend)->data, (*depend)->len, &dproj)) {
				ERR("project doesn't exists: '%.*s'", (*depend)->len, (*depend)->data);
				continue;
			}

			if (dproj->props[PROJ_PROP_LIBDIRS].set && dproj->props[PROJ_PROP_LIBDIRS].arr.count > 0) {
				first = 1;
				break;
			}
		}

		if (first) {
			fprintf_s(fp, "link_directories(");
			first = 1;
			for (int i = 0; i < proj->all_depends.count; i++) {
				prop_str_t **depend = array_get(&proj->all_depends, i);
				proj_t *dproj	    = NULL;
				if (hashmap_get(projects, (*depend)->data, (*depend)->len, &dproj)) {
					ERR("project doesn't exists: '%.*s'", (*depend)->len, (*depend)->data);
					continue;
				}

				if (dproj->props[PROJ_PROP_LIBDIRS].set) {
					array_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
					for (int j = 0; j < libdirs->count; j++) {
						prop_str_t *libdir = array_get(libdirs, j);
						if (libdir->len > 0) {
							if (!first) {
								fprintf_s(fp, " ");
							}

							char buff[MAX_PATH] = { 0 };
							unsigned int buf_len;
							char buff2[MAX_PATH] = { 0 };
							unsigned int buf2_len;

							buf_len	 = cstr_replaces(libdir->data, libdir->len, buff, MAX_PATH, vars.names, vars.tos, __VAR_MAX);
							buf2_len = cstr_replace(buff, buf_len, buff2, MAX_PATH, "$(PROJ_NAME)", 12,
										dproj->props[PROJ_PROP_NAME].value.data, dproj->props[PROJ_PROP_NAME].value.len);

							print_rel_path(fp, dproj, buff, buf_len);
							first = 0;
						}
					}
					break;
				}
			}

			fprintf_s(fp, ")\n");
		}

		fprintf_s(fp, "add_executable(%.*s ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);

		fprintf_s(fp, "target_link_libraries(%.*s", name->len, name->data);

		for (int i = 0; i < proj->all_depends.count; i++) {
			prop_str_t **depend = array_get(&proj->all_depends, i);
			proj_t *dproj	    = NULL;
			if (hashmap_get(projects, (*depend)->data, (*depend)->len, &dproj)) {
				ERR("project doesn't exists: '%.*s'", (*depend)->len, (*depend)->data);
				continue;
			}

			if (dproj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
				fprintf_s(fp, " %.*s", (*depend)->len, (*depend)->data);
			}
		}

		fprintf_s(fp, ")\n");

		break;
	}
	case PROJ_TYPE_EXT:
		//fprintf_s(fp, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		goto done;
		break;
	default:
		ERR("unknown project type: %d", type);
		ret = 1;
	}

	int first = 1;
	fprintf_s(fp, "include_directories(");
	if (proj->props[PROJ_PROP_SOURCE].set) {
		fprintf_s(fp, "%.*s", source_len, source);
		first = 0;
	}
	if (include_diff) {
		fprintf_s(fp, first ? "%.*s" : " %.*s", include_len, include);
		first = 0;
	}

	if (proj->props[PROJ_PROP_INCLUDES].set) {
		const array_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;
		for (int i = 0; i < includes->count; i++) {
			prop_str_t *inc_str = array_get(includes, i);
			proj_t *inc_proj    = NULL;
			if (hashmap_get(projects, inc_str->data, inc_str->len, &inc_proj)) {
				ERR_LOGICS("project '%.*s' doesn't exists", inc_str->path, inc_str->line + 1, inc_str->start - inc_str->line_start + 1, inc_str->len,
					   inc_str->data);
				ret++;
				continue;
			}

			if (inc_proj->props[PROJ_PROP_INCLUDE].set) {
				if (!first) {
					fprintf_s(fp, " ");
				}
				print_rel_path(fp, inc_proj, inc_proj->props[PROJ_PROP_INCLUDE].value.data, inc_proj->props[PROJ_PROP_INCLUDE].value.len);
				first = 0;
			}

			if (inc_proj->props[PROJ_PROP_ENCLUDE].set) {
				if (!first) {
					fprintf_s(fp, " ");
				}
				char buff[MAX_PATH] = { 0 };
				unsigned int buf_len;

				char buff2[MAX_PATH] = { 0 };
				unsigned int buf2_len;
				buf_len	 = cstr_replaces(inc_proj->props[PROJ_PROP_ENCLUDE].value.data, inc_proj->props[PROJ_PROP_ENCLUDE].value.len, buff, MAX_PATH,
							 vars.names, vars.tos, __VAR_MAX);
				buf2_len = cstr_replace(buff, buf_len, buff2, MAX_PATH, "$(PROJ_NAME)", 12, inc_proj->props[PROJ_PROP_NAME].value.data,
							inc_proj->props[PROJ_PROP_NAME].value.len);

				print_rel_path(fp, inc_proj, buff, buf_len);
				first = 0;
			}
		}
	}
	fprintf_s(fp, ")\n");

	if (!proj->props[PROJ_PROP_SOURCE].set) {
		if (lang & (1 << LANG_C)) {
			fprintf_s(fp, "set_target_properties(%.*s PROPERTIES LINKER_LANGUAGE C)\n", name->len, name->data);
		}
	}

	if (proj->dir.len > 0 || outdir->set || intdir->set) {
		char buff[MAX_PATH] = { 0 };
		unsigned int buf_len;
		char buff2[MAX_PATH] = { 0 };
		unsigned int buf2_len;

		fprintf_s(fp, "set_target_properties(%.*s\n    PROPERTIES\n", name->len, name->data);

		if (proj->dir.len > 0) {
			convert_slash(buff, sizeof(buff) - 1, proj->dir.path, proj->dir.len);
			fprintf_s(fp, "    FOLDER \"%.*s\"\n", (unsigned int)proj->dir.len, buff);
		}

		if (outdir->set) {
			buf_len	 = cstr_replaces(outdir->value.data, outdir->value.len, buff, MAX_PATH, vars.names, vars.tos, __VAR_MAX);
			buf2_len = cstr_replace(buff, buf_len, buff2, MAX_PATH, "$(PROJ_NAME)", 12, proj->props[PROJ_PROP_NAME].value.data,
						proj->props[PROJ_PROP_NAME].value.len);
			fprintf_s(fp, "    ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(fp, proj, buff2, buf2_len);
			fprintf_s(fp, "\"\n    ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(fp, proj, buff2, buf2_len);
			fprintf_s(fp, "\"\n    LIBRARY_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(fp, proj, buff2, buf2_len);
			fprintf_s(fp, "\"\n    LIBRARY_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(fp, proj, buff2, buf2_len);
			fprintf_s(fp, "\"\n    RUNTIME_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(fp, proj, buff2, buf2_len);
			fprintf_s(fp, "\"\n    RUNTIME_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(fp, proj, buff2, buf2_len);
			fprintf_s(fp, "\"\n)\n");
		}
	}

	if (proj->props[PROJ_PROP_WDIR].set) {
		const prop_str_t *wdir = &proj->props[PROJ_PROP_WDIR].value;
		char wdir_b[MAX_PATH]  = { 0 };
		convert_slash(wdir_b, sizeof(wdir_b) - 1, wdir->data, wdir->len);
		if (wdir->len > 0) {
			fprintf_s(fp, "set_property(TARGET %.*s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data, wdir->len,
				  wdir_b);
		}
	}

done:
	fclose(fp);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}
