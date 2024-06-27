#include "cm_proj.h"

#include "gen/mk/mk_pgen.h"
#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"
/*
static int cm_proj_gen_proj(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, FILE *file, int shared)
{
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
*/
typedef struct mk_pgen_header_data_s {
	str_t dir;
	mk_pgen_header_ext_t exts;
} mk_pgen_header_data_t;

typedef struct mk_pgen_src_data_s {
	str_t dir;
	mk_pgen_src_ext_t exts;
} mk_pgen_src_data_t;

typedef struct mk_pgen_file_data_s {
	str_t path;
	mk_pgen_file_ext_t ext;
} mk_pgen_file_data_t;

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
	str_replace(buf, STR("$(SLN_DIR)"), STR("$(CMAKE_SOURCE_DIR)"));
	str_replace(buf, STR("$(PROJ_DIR)"), strc(proj->rel_dir.path, proj->rel_dir.len));
	str_replace(buf, STR("$(PROJ_NAME)"), strc(proj->name.data, proj->name.len));
	str_replace(buf, STR("$(CONFIG)"), STR("$(Configuration)"));
	str_replace(buf, STR("$(ARCH)"), STR("${CMAKE_VS_PLATFORM_NAME}"));
	convert_slash((char *)buf->data, buf->len);
	return *buf;
}

static void cm_pgen(mk_pgen_t *gen, print_dst_t dst)
{
	static const char *header_ext[] = {
		[MK_EXT_INC] = "inc",
		[MK_EXT_H]   = "h",
		[MK_EXT_HPP] = "hpp",
	};

	// clang-format off
	static struct {
		str_t name;
		str_t flags;
		const char *ext;
		int cov;
	} src_c[] = {
		[MK_EXT_ASM] = { STRS("SRC_ASM"), STRS("ASFLAGS"),  "asm", 0 },
		[MK_EXT_S]   = { STRS("SRC_S"),  STRS("ASFLAGS"),  "S",   0 },
		[MK_EXT_C]   = { STRS("SRC_C"),    STRS("CFLAGS"),   "c",   1 },
		[MK_EXT_CPP] = { STRS("SRC_CPP"),  STRS("CXXFLAGS"), "cpp", 1 },
	};

	static struct {
		str_t postfix;
	} target_c[] = {
		[MK_BUILD_EXE]	  = { STRS("")},
		[MK_BUILD_STATIC] = { STRS("_s")},
		[MK_BUILD_SHARED] = { STRS("_d")},
		[MK_BUILD_ELF]	  = { STRS("")},
		[MK_BUILD_BIN]	  = { STRS("")},
		[MK_BUILD_FAT12]  = { STRS("")},
	};
	// clang-format on

	int src[__MK_SRC_MAX]	   = { 0 };
	int target[__MK_BUILD_MAX] = { 0 };

	int is_src = 0;

	if (gen->headers.cnt > 0 || gen->srcs.cnt > 0) {
		dprintf(dst, "file(GLOB_RECURSE %.*s_SOURCE", gen->name.len, gen->name.data);

		const mk_pgen_header_data_t *header;
		arr_foreach(&gen->headers, header)
		{
			for (mk_pgen_header_ext_t ext = 0; ext < __MK_HEADER_MAX; ext++) {
				if ((header->exts & (1 << ext)) == 0) {
					continue;
				}

				dprintf(dst, " %.*s*.%s", header->dir.len, header->dir.data, header_ext[ext]);
			}
		}

		const mk_pgen_src_data_t *data;
		arr_foreach(&gen->srcs, data)
		{
			for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
				if ((data->exts & (1 << s)) == 0) {
					continue;
				}

				is_src = 1;
				src[s] = 1;
				dprintf(dst, " %.*s*.%s", data->dir.len, data->dir.data, src_c[s].ext);
			}
		}

		dprintf(dst, ")\n\n");
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if ((gen->builds & (1 << b)) == 0) {
			continue;
		}

		switch (b) {
		case MK_BUILD_EXE:
			if (is_src) {
				dprintf(dst, "add_executable(%.*s%.*s ${%.*s_SOURCE})\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data, gen->name.len, gen->name.data);
				target[b] = 1;
			}
			break;
		case MK_BUILD_STATIC:
			if (is_src) {
				dprintf(dst, "add_library(%.*s%.*s STATIC ${%.*s_SOURCE})\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data, gen->name.len, gen->name.data);
				target[b] = 1;
			}
			break;
		case MK_BUILD_SHARED:
			if (is_src) {
				dprintf(dst, "add_library(%.*s%.*s SHARED ${%.*s_SOURCE})\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data, gen->name.len, gen->name.data);
				target[b] = 1;
			}
			break;
		default:
			break;
		}

		for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
			if (target[b] && gen->defines[i].len > 0) {
				dprintf(dst, "add_definitions(%.*s)\n", gen->defines[i].len, gen->defines[i].data);
			}
		}

		if (target[b]) {
			if (src[MK_EXT_CPP]) {
				dprintf(dst, "set_target_properties(%.*s%.*s PROPERTIES LINKER_LANGUAGE CXX)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data);
			} else if (src[MK_EXT_C]) {
				dprintf(dst, "set_target_properties(%.*s%.*s PROPERTIES LINKER_LANGUAGE C)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data);
			} else if (src[MK_EXT_S]) {
				dprintf(dst, "set_target_properties(%.*s%.*s PROPERTIES LINKER_LANGUAGE ASM)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data);
			}
		}
	}
}

static int cm_proj_gen_proj2(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, FILE *file, int shared)
{
	mk_pgen_t lgen = { 0 };
	mk_pgen_init(&lgen);

	mk_pgen_t *gen = &lgen;

	char buf_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);

	int ret = 0;

	const str_t *filename = &proj->name;

	const prop_t *pfilename = &proj->props[PROJ_PROP_FILENAME];
	if (pfilename->flags & PROP_SET) {
		filename = &pfilename->value.val;
	}
	gen->name = str_cpy(*filename);

	switch (proj->props[PROJ_PROP_TYPE].mask) {
	case PROJ_TYPE_EXE:
		gen->builds = F_MK_BUILD_EXE;
		break;
	case PROJ_TYPE_BIN:
		gen->builds = F_MK_BUILD_BIN;
		break;
	case PROJ_TYPE_ELF:
		gen->builds = F_MK_BUILD_ELF;
		break;
	case PROJ_TYPE_FAT12:
		gen->builds = F_MK_BUILD_FAT12;
		break;
	case PROJ_TYPE_LIB:
		gen->builds = F_MK_BUILD_STATIC | F_MK_BUILD_SHARED;
		break;
	case PROJ_TYPE_EXT:
		break;
	}

	const prop_t *outdir = &proj->props[PROJ_PROP_OUTDIR];
	if (outdir->flags & PROP_SET) {
		gen->outdir = str_cpy(resolve(outdir->value.val, &buf, proj));
		switch (proj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_EXE: {
			str_t c = strn(buf.data, buf.len, buf.len + 5);
			str_cat(&c, STR("cov/"));
			gen->covdir = c;
			break;
		}
		default:
			break;
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].arr.cnt > 0) {
		const prop_t *intdir = &proj->props[PROJ_PROP_INTDIR];
		if (intdir->flags & PROP_SET) {
			resolve(intdir->value.val, &buf, proj);
			switch (proj->props[PROJ_PROP_TYPE].mask) {
			case PROJ_TYPE_EXE:
			case PROJ_TYPE_BIN:
			case PROJ_TYPE_ELF:
				gen->intdir[MK_INTDIR_OBJECT] = str_cpy(buf);
				break;
			case PROJ_TYPE_LIB: {
				resolve(intdir->value.val, &buf, proj);
				str_t s = strn(buf.data, buf.len, buf.len + 8);
				str_cat(&s, STR("static/"));
				gen->intdir[MK_INTDIR_STATIC] = s;

				str_t d = strn(buf.data, buf.len, buf.len + 8);
				str_cat(&d, STR("shared/"));
				gen->intdir[MK_INTDIR_SHARED] = d;
				break;
			}
			default:
				break;
			}
		}
	}

	const prop_str_t *config;
	arr_foreach(&sln_props[SLN_PROP_CONFIGS].arr, config)
	{
		mk_pgen_add_config(gen, strs(config->val));
	}

	uint lang = proj->props[PROJ_PROP_LANGS].mask;
	const prop_str_t *include;
	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		int exts = 0;
		exts |= lang & (1 << LANG_NASM) ? F_MK_EXT_INC : 0;
		exts |= lang & (1 << LANG_ASM) ? F_MK_EXT_INC : 0;
		exts |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_MK_EXT_H : 0;
		exts |= lang & (1 << LANG_CPP) ? F_MK_EXT_HPP : 0;

		if (exts) {
			mk_pgen_add_header(gen, strs(include->val), exts);
		}
	}

	const prop_str_t *source;
	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		int headers = 0;
		headers |= lang & (1 << LANG_NASM) ? F_MK_EXT_INC : 0;
		headers |= lang & (1 << LANG_ASM) ? F_MK_EXT_INC : 0;
		headers |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_MK_EXT_H : 0;
		headers |= lang & (1 << LANG_CPP) ? F_MK_EXT_HPP : 0;

		if (headers) {
			mk_pgen_add_header(gen, strs(source->val), headers);
		}

		int srcs = 0;
		srcs |= lang & (1 << LANG_NASM) ? F_MK_EXT_ASM : 0;
		srcs |= lang & (1 << LANG_ASM) ? F_MK_EXT_S : 0;
		srcs |= lang & (1 << LANG_C) ? F_MK_EXT_C : 0;
		srcs |= lang & (1 << LANG_CPP) ? F_MK_EXT_CPP : 0;

		if (srcs) {
			mk_pgen_add_src(gen, strs(source->val), srcs);
		}
	}

	cm_pgen(gen, PRINT_DST_FILE(file));

	mk_pgen_free(&lgen);
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

	cm_proj_gen_proj2(proj, projects, sln_props, file, 0);

	file_close(file);
	if (ret == 0) {
		SUC("generating project: %s success", gen_path.path);
	} else {
		ERR("generating project: %s failed", gen_path.path);
	}

	return ret;
}
//467