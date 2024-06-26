#include "cm_proj.h"

#include "gen/var.h"

#include "common.h"

static size_t resolve(const prop_str_t *prop, char *buf, size_t buf_size, const proj_t *proj)
{
	size_t buf_len;

	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(SLN_DIR)"), CSTR("${CMAKE_SOURCE_DIR}/"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_DIR)"), proj->rel_dir.path, proj->rel_dir.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJ_NAME)"), proj->name.data, proj->name.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(CONFIG)"), CSTR("$(Configuration)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PLATFORM)"), CSTR("${CMAKE_VS_PLATFORM_NAME}"), NULL);
	convert_slash(buf, buf_size);

	return buf_len;
}

static inline void print_rel_path(FILE *fp, const proj_t *proj, const char *path, size_t path_len)
{
	char path_b[P_MAX_PATH] = { 0 };
	mem_cpy(path_b, sizeof(path_b), path, path_len);
	convert_slash(path_b, path_len);
	char rel_path[P_MAX_PATH] = { 0 };
	mem_cpy(rel_path, sizeof(rel_path), proj->rel_dir.path, proj->rel_dir.len);
	convert_slash(rel_path, proj->rel_dir.len);

	if (cstr_eqn(path_b, path_len, CSTR("${CMAKE_SOURCE_DIR}"), 19)) {
		c_fprintf(fp, "%.*s", path_len, path_b);
	} else {
		c_fprintf(fp, "${CMAKE_SOURCE_DIR}/%.*s/%.*s", proj->rel_dir.len, rel_path, path_len, path_b);
	}
}

static int cm_proj_gen_proj(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, FILE *file, int shared)
{
	const str_t *name = &proj->name;
	proj_type_t type  = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *langs   = &proj->props[PROJ_PROP_LANGS];
	const prop_t *charset = &proj->props[PROJ_PROP_CHARSET];
	const prop_t *flags   = &proj->props[PROJ_PROP_FLAGS];
	const prop_t *outdir  = &proj->props[PROJ_PROP_OUTDIR];
	const prop_t *intdir  = &proj->props[PROJ_PROP_INTDIR];

	char enclude[P_MAX_PATH] = { 0 };
	char buf[P_MAX_PATH]	 = { 0 };

	size_t enclude_len;
	size_t buf_len;

	if (proj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
		mem_cpy(enclude, sizeof(enclude), proj->props[PROJ_PROP_ENCLUDE].value.val.data, proj->props[PROJ_PROP_ENCLUDE].value.val.len);
		enclude_len = proj->props[PROJ_PROP_ENCLUDE].value.val.len;
		convert_slash(enclude, enclude_len);
	}

	int ret = 0;

	uint lang = langs->mask;

	if ((proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) || (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) || (proj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET)) {
		c_fprintf(file, "file(GLOB_RECURSE %.*s%s_SOURCE", name->len, name->data, shared ? "_d" : "");
		if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
			const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

			for (uint i = 0; i < sources->cnt; i++) {
				prop_str_t *source = arr_get(sources, i);
				mem_cpy(buf, sizeof(buf), source->val.data, source->val.len);
				buf_len = source->val.len;
				convert_slash(buf, source->val.len);

				if (lang & (1 << LANG_NASM)) {
					c_fprintf(file, " %.*s*.nasm", buf_len, buf, buf_len, buf);
				}
				if (lang & (1 << LANG_ASM)) {
					c_fprintf(file, " %.*s*.asm", buf_len, buf, buf_len, buf);
				}
				if ((lang & (1 << LANG_NASM)) || (lang & (1 << LANG_ASM))) {
					c_fprintf(file, " %.*s*.inc", buf_len, buf);
				}
				if (lang & (1 << LANG_C)) {
					c_fprintf(file, " %.*s*.c", buf_len, buf);
				}
				if ((lang & (1 << LANG_C)) || (lang & (1 << LANG_CPP))) {
					c_fprintf(file, " %.*s*.h", buf_len, buf);
				}
				if (lang & (1 << LANG_CPP)) {
					c_fprintf(file, " %.*s*.cpp %.*s*.hpp", buf_len, buf, buf_len, buf);
				}
			}
		}

		if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				prop_str_t *include = arr_get(includes, i);
				buf_len		    = include->val.len;
				mem_cpy(buf, sizeof(buf), include->val.data, include->val.len);
				convert_slash(buf, include->val.len);

				if ((lang & (1 << LANG_NASM) || (lang & (1 << LANG_ASM)))) {
					c_fprintf(file, " %.*s*.inc", buf_len, buf);
				}
				if ((lang & (1 << LANG_C)) || (lang & (1 << LANG_CPP))) {
					c_fprintf(file, " %.*s*.h", buf_len, buf);
				}
				if (lang & (1 << LANG_CPP)) {
					c_fprintf(file, " %.*s*.hpp", buf_len, buf);
				}
			}
		}

		c_fprintf(file, ")\n\n");
	}

	if ((proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) || charset->mask == CHARSET_UNICODE) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		if (defines->cnt > 0 || charset->mask == CHARSET_UNICODE) {
			c_fprintf(file, "add_definitions(");
		}

		int first = 1;

		if (charset->mask == CHARSET_UNICODE) {
			c_fprintf(file, first ? "-DUNICODE -D_UNICODE" : " -DUNICODE -D_UNICODE");
			first = 0;
		}

		if (shared) {
			str_t upper = strz(name->len + 1);
			str_to_upper(*name, &upper);
			c_fprintf(file, first ? "-D%.*s_BUILD_DLL" : " -D%.*s_BUILD_DLL", (int)upper.len, upper.data);
			first = 0;
			str_free(&upper);
		}

		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_dep_t *dep = arr_get(&proj->all_depends, i);

			if (dep->link_type != LINK_TYPE_SHARED) {
				continue;
			}

			str_t upper = strz(dep->proj->name.len + 1);
			str_to_upper(dep->proj->name, &upper);
			c_fprintf(file, first ? "-D%.*s_DLL" : " -D%.*s_DLL", (int)upper.len, upper.data);
			first = 0;
			str_free(&upper);
		}

		for (uint i = 0; i < defines->cnt; i++) {
			const prop_str_t *define = arr_get(defines, i);
			c_fprintf(file, first ? "-D%.*s" : " -D%.*s", define->val.len, define->val.data);
			first = 0;
		}

		if (!first) {
			c_fprintf(file, ")\n\n");
		}
	}

	switch (type) {
	case PROJ_TYPE_LIB:
		c_fprintf(file, "add_library(%.*s%s %s ${%.*s%s_SOURCE})\n", name->len, name->data, shared ? "_d" : "", shared ? "SHARED" : "STATIC", name->len,
			  name->data, shared ? "_d" : "");

		if (shared && proj->all_depends.cnt > 0) {
			c_fprintf(file, "target_link_libraries(%.*s_d", name->len, name->data);

			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_dep_t *dep = arr_get(&proj->all_depends, i);

				if (dep->proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
					c_fprintf(file, " %.*s%s", dep->proj->name.len, dep->proj->name.data, dep->link_type == LINK_TYPE_SHARED ? "_d" : "");
				}

				if (dep->proj->props[PROJ_PROP_LINK].flags & PROP_SET) {
					const arr_t *links = &dep->proj->props[PROJ_PROP_LINK].arr;
					for (uint j = 0; j < links->cnt; j++) {
						const prop_str_t *link = arr_get(links, j);
						if (link->val.len > 0) {
							c_fprintf(file, " %.*s", link->val.len, link->val.data);
						}
					}
				}
			}
			c_fprintf(file, ")\n");
		}
		break;
	case PROJ_TYPE_EXE: {
		int first = 0;
		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_dep_t *dep = arr_get(&proj->all_depends, i);

			if ((dep->proj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) && dep->proj->props[PROJ_PROP_LIBDIRS].arr.cnt > 0) {
				first = 1;
				break;
			}
		}

		if (first) {
			c_fprintf(file, "link_directories(");
			first = 1;
			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_dep_t *dep = arr_get(&proj->all_depends, i);

				if (dep->proj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
					const arr_t *libdirs = &dep->proj->props[PROJ_PROP_LIBDIRS].arr;
					for (uint j = 0; j < libdirs->cnt; j++) {
						prop_str_t *libdir = arr_get(libdirs, j);
						if (libdir->val.len > 0) {
							if (!first) {
								c_fprintf(file, " ");
							}

							buf_len = resolve(libdir, buf, sizeof(buf), dep->proj);

							print_rel_path(file, dep->proj, buf, buf_len);
							first = 0;
						}
					}
					break;
				}
			}

			c_fprintf(file, ")\n");
		}

		c_fprintf(file, "add_executable(%.*s ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);

		if (proj->all_depends.cnt > 0) {
			c_fprintf(file, "target_link_libraries(%.*s", name->len, name->data);

			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_dep_t *dep = arr_get(&proj->all_depends, i);

				if (dep->proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
					c_fprintf(file, " %.*s%s", dep->proj->name.len, dep->proj->name.data, dep->link_type == LINK_TYPE_SHARED ? "_d" : "");
				}

				if (dep->proj->props[PROJ_PROP_LINK].flags & PROP_SET) {
					const arr_t *links = &dep->proj->props[PROJ_PROP_LINK].arr;
					for (uint j = 0; j < links->cnt; j++) {
						const prop_str_t *link = arr_get(links, j);
						if (link->val.len > 0) {
							c_fprintf(file, " %.*s", link->val.len, link->val.data);
						}
					}
				}
			}
			c_fprintf(file, ")\n");
		}

		break;
	}
	case PROJ_TYPE_EXT:
		//c_fprintf(fp, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		goto done;
		break;
	default:
		ERR("unknown project type: %d", type);
		ret = 1;
	}

	int first = 1;
	c_fprintf(file, "include_directories(");

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

		for (uint i = 0; i < sources->cnt; i++) {
			prop_str_t *source = arr_get(sources, i);

			buf_len = source->val.len;
			mem_cpy(buf, sizeof(buf), source->val.data, source->val.len);
			convert_slash(buf, source->val.len);

			c_fprintf(file, first ? "%.*s" : " %.*s", buf_len - 1, buf);
			first = 0;
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

		for (uint i = 0; i < includes->cnt; i++) {
			prop_str_t *include = arr_get(includes, i);

			buf_len = include->val.len;
			mem_cpy(buf, sizeof(buf), include->val.data, include->val.len);
			convert_slash(buf, include->val.len);

			c_fprintf(file, first ? "%.*s" : " %.*s", buf_len - 1, buf);
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
					c_fprintf(file, " ");
				}
				print_rel_path(file, iproj, include->val.data, include->val.len);
				first = 0;
			}
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			if (!first) {
				c_fprintf(file, " ");
			}

			buf_len = resolve(&iproj->props[PROJ_PROP_ENCLUDE].value, buf, sizeof(buf), iproj);

			print_rel_path(file, iproj, buf, buf_len);
			first = 0;
		}
	}

	c_fprintf(file, ")\n");

	if (!proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		if (lang & (1 << LANG_C)) {
			c_fprintf(file, "set_target_properties(%.*s%s PROPERTIES LINKER_LANGUAGE C)\n", name->len, name->data, shared ? "_d" : "");
		}
	}

	if (proj->rel_dir.len > 0 || (outdir->flags & PROP_SET) || (intdir->flags & PROP_SET)) {
		c_fprintf(file, "set_target_properties(%.*s%s\n    PROPERTIES\n", name->len, name->data, shared ? "_d" : "");

		if (proj->rel_dir.len > 0) {
			buf_len = proj->rel_dir.len;
			mem_cpy(buf, sizeof(buf), proj->rel_dir.path, proj->rel_dir.len);
			convert_slash(buf, proj->rel_dir.len);
			c_fprintf(file, "    FOLDER \"%.*s\"\n", buf_len - 1, buf);
		}

		if (outdir->flags & PROP_SET) {
			buf_len = resolve(&outdir->value, CSTR(buf), proj);
			c_fprintf(file, "    ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    LIBRARY_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    LIBRARY_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    RUNTIME_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    RUNTIME_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n)\n");
		}
	}

	if (proj->props[PROJ_PROP_WDIR].flags & PROP_SET) {
		const prop_str_t *wdir = &proj->props[PROJ_PROP_WDIR].value;

		buf_len = wdir->val.len;
		mem_cpy(buf, sizeof(buf), wdir->val.data, wdir->val.len);
		convert_slash(buf, wdir->val.len);
		if (wdir->val.len > 0) {
			c_fprintf(file, "set_property(TARGET %.*s%s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data,
				  shared ? "_d" : "", buf_len - 1, buf);
		}
	} else {
		buf_len = proj->rel_dir.len;
		mem_cpy(buf, sizeof(buf), proj->rel_dir.path, proj->rel_dir.len);
		convert_slash(buf, proj->rel_dir.len);
		c_fprintf(file, "set_property(TARGET %.*s%s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data,
			  shared ? "_d" : "", proj->rel_dir.len - 1, buf);
	}

#if defined(C_WIN)
	if (type == PROJ_TYPE_EXE) {
		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_dep_t *dep = arr_get(&proj->all_depends, i);

			if (dep->link_type != LINK_TYPE_SHARED) {
				continue;
			}

			path_t target = { 0 };
			path_init(&target, "", 0);
			target.len = resolve(&dep->proj->props[PROJ_PROP_OUTDIR].value, target.path, sizeof(target.path), dep->proj);
			path_child(&target, dep->proj->name.data, dep->proj->name.len);
			path_child_s(&target, CSTR("_d.dll"), 0);

			c_fprintf(file,
				  "add_custom_command(TARGET %.*s POST_BUILD\n"
				  "    COMMAND ${CMAKE_COMMAND} -E copy_if_different\n"
				  "    \"%.*s\"\n"
				  "    $<TARGET_FILE_DIR:%.*s>\n"
				  ")\n",
				  proj->name.len, proj->name.data, target.len, target.path, proj->name.len, proj->name.data);

			if (dep->proj->props[PROJ_PROP_LINK].flags & PROP_SET) {
				const arr_t *links = &dep->proj->props[PROJ_PROP_LINK].arr;
				for (uint j = 0; j < links->cnt; j++) {
					const prop_str_t *link = arr_get(links, j);
					if (link->val.len > 0) {
						c_fprintf(file, " %.*s", link->val.len, link->val.data);
					}
				}
			}
		}
	}
#endif

done:
	return ret;
}

int cm_proj_gen(const proj_t *proj, const dict_t *projects, const prop_t *sln_props)
{
	path_t gen_path = { 0 };
	if (path_init(&gen_path, proj->dir.path, proj->dir.len) == NULL) {
		return 1;
	}

	if (!folder_exists(gen_path.path)) {
		folder_create(gen_path.path);
	}

	if (path_child(&gen_path, CSTR("CMakeLists.txt")) == NULL) {
		return 1;
	}

	FILE *file = file_open(gen_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	int ret = 0;

	MSG("generating project: %s", gen_path.path);

	cm_proj_gen_proj(proj, projects, sln_props, file, 0);
	if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
		c_fprintf(file, "\n");
		cm_proj_gen_proj(proj, projects, sln_props, file, 1);
	}

	file_close(file);
	if (ret == 0) {
		SUC("generating project: %s success", gen_path.path);
	} else {
		ERR("generating project: %s failed", gen_path.path);
	}

	return ret;
}
